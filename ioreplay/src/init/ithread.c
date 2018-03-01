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

#include "ithread.h"

#include "itask.h"
#include "../utils/futils.h"


void* ithread_pthread_start(void *data)
{
    ithread_s *t = data;
    init_s *i = t->init;
    itask_s *task = NULL;

    do {
        while (NULL != (task = rbuffer_get_next(t->queue))) {
            ithread_run_task(t, task);

            // We need to mutex lock the reuse_queue as multiple threads
            // can insert into it
            pthread_mutex_lock(&i->reuse_queue_mutex);
            int ret = rbuffer_insert(i->reuse_queue, task);
            pthread_mutex_unlock(&i->reuse_queue_mutex);
            if (!ret)
                itask_destroy(task);
        }
        usleep(100);
    } while (!t->terminate);

    while (NULL != (task = rbuffer_get_next(t->queue))) {
        ithread_run_task(t, task);
        if (!rbuffer_insert(i->reuse_queue, task))
            itask_destroy(task);

        pthread_mutex_lock(&i->reuse_queue_mutex);
        int ret = rbuffer_insert(i->reuse_queue, task);
        pthread_mutex_unlock(&i->reuse_queue_mutex);
        if (!ret)
            itask_destroy(task);
    }

    return NULL;
}

ithread_s* ithread_new(init_s *i)
{
    ithread_s *t = Malloc(ithread_s);

    t->init = i;
    t->queue = rbuffer_new(1024);
    t->terminate = false;

    return t;
}

void ithread_start(ithread_s *t)
{
    start_pthread(&t->pthread, ithread_pthread_start, (void*)t);
}

void ithread_destroy(ithread_s *t)
{
    rbuffer_destroy(t->queue);
    free(t);
}

void ithread_terminate(ithread_s *t)
{
    t->terminate = true;
    pthread_join(t->pthread, NULL);
}

void ithread_run_task(ithread_s *t, itask_s *task)
{
    if (task->is_dir) {
        task->dirs_created += ensure_dir_exists(task->path);

    } else if (task->is_file) {
        if (!ensure_file_exists(task->path, &task->dirs_created)) {
            task->files_created++;
            if (task->vsize > 0) {
                append_random_to_file(task->path, task->vsize);
                task->sizes_created += task->vsize;
            }
        }
    }
}
