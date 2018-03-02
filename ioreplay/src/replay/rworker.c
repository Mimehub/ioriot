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

#include "rworker.h"

#include "../datas/stack.h"
#include "rprocess.h"
#include "rthread.h"

#define _Compute_current_time(now) \
        (now.tv_sec - start_time.tv_sec) * 1000 \
        + (now.tv_usec - start_time.tv_usec) / 1000


/**
 * @brief A callback helper function for destroying all virtual process objects
 *
 * @param data The process object.
 */
static void _rprocess_destroy_cb(void *data)
{
    rprocess_destroy(data);
}

rworker_s* rworker_new(const int rworker_num, amap_s *fds_map,
                       const long num_vsizes, const long num_pids,
                       options_s *opts, rworker_stats_s *worker_stats)
{
    rworker_s *w = Malloc(rworker_s);

#ifdef THREAD_DEBUG
    char *rworker_log = Calloc(1024, char);
    snprintf(rworker_log, 1023, "/tmp/ioreplay/_worker%d.debuglog",
             rworker_num);

    w->rworker_fd = Fopen(rworker_log, "a");
    free(rworker_log);
    fprintf(w->rworker_fd, "DEBUG: Started worker\n");
#endif

    w->rworker_num = rworker_num;
    w->opts = opts;
    w->fds_map = fds_map;

    w->rprocess_map = amap_new(num_pids);
    w->rthread_map = amap_new(num_vsizes);
    w->task_buffer = rbuffer_new(opts->num_threads_per_worker
                                 *TASK_BUFFER_PER_THREAD);
    w->rthread_buffer = rbuffer_new(opts->num_threads_per_worker);
    w->worker_stats = worker_stats;

    // Attach a cleanup callback function to the worker map.
    w->rprocess_map->data_destroy = _rprocess_destroy_cb;

    pthread_mutex_init(&w->rthread_buffer_mutex, NULL);
    pthread_mutex_init(&w->task_buffer_mutex, NULL);

    // TODO: Check in the program whether the ulimit is high enough
    // or not! (ulimit -n)

    return w;
}

/**
 * @brief Destroys the object
 *
 * Destroys the worker object (frees all memory allocated by the worker)
 *
 * @param w The worker object
 */
void rworker_destroy(rworker_s *w)
{
    if (!w)
        return;

    if (w->rprocess_map)
        amap_destroy(w->rprocess_map);
    if (w->rthread_map)
        amap_destroy(w->rthread_map);

    if (w->task_buffer) {
        rtask_s *task = NULL;
        while (NULL != (task = rbuffer_get_next(w->task_buffer)))
            rtask_destroy(task);
        rbuffer_destroy(w->task_buffer);
    }

    if (w->rthread_buffer)
        rbuffer_destroy(w->rthread_buffer);

    pthread_mutex_destroy(&w->task_buffer_mutex);
    pthread_mutex_destroy(&w->rthread_buffer_mutex);

#ifdef THREAD_DEBUG
    if (w->rworker_fd)
        fclose(w->rworker_fd);
#endif

    free(w);
}

status_e rworker_process_lines(rworker_s* w, const long num_lines)
{
    Out("worker(%d): Starting to process replay lines\n", w->rworker_num);

    options_s *opts = w->opts;
    FILE *replay_fd = Fopen(opts->replay_file, "r");

    // Drop root privileges, otherwise we may overwrite other system
    // files by accident in case of a bug or user error!
    set_limits_drop_root(opts->user);

    // Variables required for the time based caluclations
    struct timeval now, start_time;
    long current_time = 0, stats_time = 0;
    gettimeofday(&start_time, NULL);

    // Helper variables required for reading lines
    char *line = NULL;
    char *next = NULL, *next2 = NULL;
    size_t len = 0, read = 0;

    // Helpers required for threading
    rthread_s *t = NULL;
    stack_s *all_threads = stack_new();
    rworker_stats_s *s = w->worker_stats;

    // More helper variables
    //unsigned long lineno = 0, stats_ioop = 0, vsize_id = 0;
    unsigned long lineno = 0, vsize_id = 0;
    long pid = -1, time = -1;

    // Process the .replay file line by line.
    while ((read = getline(&line, &len, replay_fd)) != -1) {
        lineno++;

        if (read >= MAX_LINE_LEN) {
            Error("line:%lu Exceeded max line len", lineno);
        }

        // If the line begins with #: Ignore that line, it contains
        // debug or meta information or comments.

        if (line[0] == '#') {
            if (line[1] == 'I') {
                // We stop replaying I/O once we reach the line '#INIT'
                // which incitates the begin of the INIT section.
                break;
            }
            continue;
        }

#ifdef THREAD_DEBUG
        char *clone = Clone(line);
#endif

        next = strchr(line, '|');
        Error_if(!next, "lineno:%ld Could not parse time from input file",
                 lineno);
        next[0] = '\0';
        next++;
        time = atol(line);

        next2 = strchr(next, '|');
        Error_if(!next2, "Could not parse vsize_id from input file");
        next2[0] = '\0';
        next2++;
        vsize_id = atol(next);

        // This worker is not responsible for this line, skip it!
        if ((vsize_id % opts->num_workers) != w->rworker_num) {
#ifdef THREAD_DEBUG
            free(clone);
#endif
            continue;
        }

        next = strchr(next2, '|');
        Error_if(!next, "Could not parse PID from input file");
        next[0] = '\0';
        next++;
        pid = atol(next2);

        gettimeofday(&now, NULL);
        current_time = _Compute_current_time(now);

        // Check whether the user specified a replay speed factor. If so, we
        // may need to throttle down a bit.

        if (opts->speed_factor) {
            s->time_ahead = time / opts->speed_factor - current_time;
            if (s->time_ahead > 0)
                usleep(s->time_ahead*1000);

        } else {
            s->time_ahead = time - current_time;
        }

        // Get the responsible process object. The process object holds data
        // structures usually found in a Linux process, e.g. a table of open
        // file descriptors.

        rprocess_s *p = amap_get(w->rprocess_map, pid);
        if (p == NULL) {
            p = rprocess_new(pid, w->fds_map);
            amap_set(w->rprocess_map, pid, p);
        }
        p->lineno = lineno;

        if (opts->num_threads_per_worker == 1) {
            // Single threaded mode?
            if (!t)
                t = rthread_new(vsize_id, w);
            else
                rthread_update(t, vsize_id);

        }  else {
            t = amap_get(w->rthread_map, vsize_id);
        }

        if (t == NULL) {

            // First try to recycle an old (likely unused) thread
            if (NULL != (t = rbuffer_get_next(w->rthread_buffer))) {
                rthread_update(t, vsize_id);

#ifdef THREAD_DEBUG
                fprintf(w->rworker_fd, "DEBUG: Reused an idling thread\n");
                fflush(w->rworker_fd);
#endif

            } else if (opts->num_threads_per_worker <= all_threads->size) {
                // Reached max threads, waiting until one becomes available

#ifdef THREAD_DEBUG
                fprintf(w->rworker_fd, "DEBUG: Reached max threads\n");
                fflush(w->rworker_fd);
#endif
                while (NULL == (t = rbuffer_get_next(w->rthread_buffer)))
                    usleep(1000);

#ifdef THREAD_DEBUG
                fprintf(w->rworker_fd, "DEBUG: Reused an idling thread\n");
                fflush(w->rworker_fd);
#endif

                rthread_update(t, vsize_id);

            } else {
                t = rthread_new(vsize_id, w);

                // We hold a pointer to all created threads in a stack. This
                // stack is later used to terminate/join all therads.
                stack_push(all_threads, t);

#ifdef THREAD_DEBUG
                fprintf(w->rworker_fd, "DEBUG: Created a new thread\n");
                fflush(w->rworker_fd);
#endif
            }

            amap_set(w->rthread_map, vsize_id, t);
        }

        // Create a new task for the thread. The task contains all required
        // information to run an I/O operation. However, first try to
        // reuse/recycle a task object! If there is no such, create a new one.

        rtask_s *task = rbuffer_get_next(w->task_buffer);
        if (!task)
            task = rtask_new();
        rtask_update(task, w, p, next, lineno, vsize_id);
        s->ioops++;


#ifdef THREAD_DEBUG
        task->clone = clone;
        fprintf(w->rworker_fd, "DEBUG: Inserting new task\n");
        fflush(w->rworker_fd);
#endif

        // Insert that task to a ring buffer to pass it to the pthread without
        // much synchronisation overhead!

        while (!rthread_insert_task(t, task))
            // The ring buffer is full. This may happen if the pthread didn't
            // manage to process tasks fast enough. re-try after a short period!
            usleep(1000);

#ifdef THREAD_DEBUG
        fprintf(w->rworker_fd, "DEBUG: Task inserted\n");
        fflush(w->rworker_fd);
#endif

        // The worker prints out stats every 3s
        if (current_time - stats_time > 3000) {
            // IDEA: Maybe refactor this block to be implemented in rstats.c

            double loadavg = get_loadavg();

            // Determines whether we replay the I/O faster or slower than
            // original speed!
            char *a_b = s->time_ahead >= 0 ? "ahead" : "behind";

            Put("worker(%d): threads:%ld %s:%lds progress:%0.2f%% "
                "loadavg:%0.2f",
                w->rworker_num, all_threads->size, a_b, Abs(s->time_ahead/1000),
                Perc(lineno,num_lines), loadavg);

            stats_time = current_time;
            //stats_ioop = lineno;

            if (s->loadavg_high < loadavg)
                s->loadavg_high = loadavg;
        }
    }

    Put("worker(%d): Waiting for all threads to finish business...",
        w->rworker_num);

    // This will wait (join) all threads one after another until all threads
    // have finished their work and have terminated.

    while (!stack_is_empty(all_threads)) {
        rthread_s *t = stack_pop(all_threads);
        rthread_terminate(t);
        rthread_destroy(t);
    }
    stack_destroy(all_threads);

    // Collect some stats last time
    double loadavg = get_loadavg();
    if (s->loadavg_high < loadavg)
        s->loadavg_high = loadavg;

    gettimeofday(&now, NULL);
    current_time = _Compute_current_time(now);
    if (opts->speed_factor) {
        s->time_ahead = time / opts->speed_factor - current_time;
    } else {
        s->time_ahead = time - current_time;
    }


    Put("worker(%d): All threads terminated!", w->rworker_num);
    fclose(replay_fd);

    return SUCCESS;
}
