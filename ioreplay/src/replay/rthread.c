// Copyright 2018 Mimecast Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rthread.h"

#include <sys/types.h>

#include "rworker.h"
#include "rprocess.h"

#include "rioop.h"

#ifdef THREAD_DEBUG
/**
 * @brief For debugging purposes only
 *
 * @param t The responsible thread object
 */
static void _rthread_init_log(rthread_s *t)
{
    rworker_s *w = t->worker;
    char *rthread_log = Calloc(1024, char);
    snprintf(rthread_log, 1023, "/tmp/ioreplay/worker%d.thread%ld.debuglog",
             w->rworker_num, (long)pthread_self());

    ensure_dir_exists("/tmp/ioreplay");
    t->rthread_fd = Fopen(rthread_log, "a");

    free(rthread_log);
    fprintf(t->rthread_fd, "%ld: DEBUG: Created thread log\n", t->tid);
}
#endif

void rthread_process_task(rthread_s* t, rtask_s *task,
                          pid_t pthread_id)
{
    char *next = task->line;
    rworker_s *w = (rworker_s*) task->worker;

    // Tokenize the remaining elements of the line.
    int ntoks = 0;
    char *saveptr;
    char *tok = strtok_r(next, "|", &saveptr);

    while (tok) {
        if (ntoks > MAX_TOKENS) {
            Error("worker(%d) pthread(%d): lineno:%lu, missing newline?",
                  w->rworker_num, pthread_id, task->lineno);
        }
        task->toks[ntoks++] = tok;
        tok = strtok_r(NULL, "|", &saveptr);
    }
    // NULL marker (no more token from here)
    task->toks[ntoks] = NULL;

#ifdef THREAD_DEBUG
    fprintf(t->rthread_fd, "%ld(%ld): %s",
            t->tid, (long)pthread_self(), task->clone);
    fflush(t->rthread_fd);
    free(task->clone);
    task->clone = NULL;
#endif
#ifndef NO_RIOOP
    // Perform the corresponding I/O operation!
    rioop_run(task->process, t, task);
#endif

    // Make the task object recyclable/reusable
    pthread_mutex_lock(&w->task_buffer_mutex);
    if (!rbuffer_insert(w->task_buffer, task))
        // We can't recycle the task object if the buffer is full!
        rtask_destroy(task);
    pthread_mutex_unlock(&w->task_buffer_mutex);
}

void *rthread_pthread_start(void *data)
{
    rthread_s* t = (rthread_s*) data;
    rworker_s *w = t->worker;
    rtask_s *task = NULL;
    pid_t pthread_id = pthread_self();

#ifdef THREAD_DEBUG
    _rthread_init_log(t);
#endif

    do {
        while (!rbuffer_has_next(t->tasks) && !t->terminate)
            usleep(100);

        while ((task = rbuffer_get_next(t->tasks)) != NULL)
            rthread_process_task(t, task, pthread_id);

#ifdef THREAD_DEBUG
        fprintf(t->rthread_fd, "%ld: DEBUG: Idling\n", t->tid);
        fflush(t->rthread_fd);
#endif

        // Tell rworker_s that thread is not doing any work!
        int inserted = false;
        while (!inserted && !t->terminate) {
            if (rbuffer_has_next(t->tasks))
                break;

            usleep(1000);

            if (rbuffer_has_next(t->tasks))
                break;

            // Make the rthread reusable, he is without any tasks
            // for some time.
            pthread_mutex_lock(&w->rthread_buffer_mutex);
            inserted = rbuffer_insert(w->rthread_buffer, t);
            pthread_mutex_unlock(&w->rthread_buffer_mutex);
        }

#ifdef THREAD_DEBUG
        if (inserted) {
            fprintf(t->rthread_fd, "%ld: DEBUG: Added to thread buffer\n",
                    t->tid);
        } else {
            fprintf(t->rthread_fd, "%ld: DEBUG: Idling thread recovered\n",
                    t->tid);
        }
        fflush(t->rthread_fd);
#endif

    } while (!t->terminate);

#ifdef THREAD_DEBUG
    fprintf(t->rthread_fd, "%ld: DEBUG: Terminating\n", t->tid);
    fflush(t->rthread_fd);
#endif

    // Process the very last tasks
    while (NULL != (task = rbuffer_get_next(t->tasks)))
        rthread_process_task(t, task, pthread_id);

#ifdef THREAD_DEBUG
    fprintf(t->rthread_fd, "%ld: DEBUG: Done terminating\n", t->tid);
    fflush(t->rthread_fd);
#endif

    return NULL;
}

rthread_s* rthread_new(const long tid, void *worker)
{
    rthread_s *t = Malloc(rthread_s);
    rworker_s *w = worker;

    t->single_threaded = w->opts->num_threads_per_worker == 1;
    t->tasks = rbuffer_new(TASK_BUFFER_PER_THREAD);
    t->terminate = false;
    t->worker = worker;
    rthread_update(t, tid);

    if (t->single_threaded) {
#ifdef THREAD_DEBUG
        _rthread_init_log(t);
#endif
        return t;
    }

    start_pthread(&t->pthread, rthread_pthread_start, (void*)t);
    return t;
}

long rthread_update(rthread_s *t, const long tid)
{
    long prev_tid = t->tid;
    t->tid = tid;

    return prev_tid;
}

void rthread_destroy(rthread_s *t)
{
    if (rbuffer_has_next(t->tasks)) {
        Error("Didn't expect to have any tasks left!");
    }
    rbuffer_destroy(t->tasks);

#ifdef THREAD_DEBUG
    if (t->rthread_fd)
        fclose(t->rthread_fd);
#endif

    free(t);
}

bool rthread_insert_task(rthread_s* t, rtask_s* task)
{
    if (t->single_threaded) {
        rthread_process_task(t, task, pthread_self());
        return true;
    }
    return rbuffer_insert(t->tasks, task);
}

void rthread_terminate(rthread_s* t)
{
    t->terminate = true;
    pthread_join(t->pthread, NULL);
}
