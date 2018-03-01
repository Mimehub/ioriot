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

/**
 * @file rthread.h
 * @author Paul Buetow
 *
 * @brief The replay thread definitiion
 */

#ifndef RTHREAD_H
#define RTHREAD_H

#include "../defaults.h"
#include "../datas/rbuffer.h"
#include "../datas/amap.h"
#include "../vfd.h"
#include "rtask.h"

#include <pthread.h>

/**
 * @brief Definition of a worker thread
 *
 * Every worker utilises a set of worker threads in order to parallelise the
 * replaying of the I/O! Every thread comes with its own task queue. It is
 * filled by the repsonsible worker.
 *
 * The user can specify the max amount of threads per worker per -t command
 * line switch.
 */
typedef struct rthread_s_ {
    void *worker; /**< The responsible worker object */
    long tid; /**< The virtual thread id */
    rbuffer_s* tasks; /**< Holds all outstanding tasks */
    bool terminate; /**< True if thread shall terminate */
    bool single_threaded; /**< Worker is single threaded or not */
    pthread_t pthread; /**< We run the tasks in concurrent pthreads */
#ifdef RTHREAD_DEBUG
    FILE *rthread_fd; /**< Used for debugging purposes only */
#endif
} rthread_s;

/**
 * @brief Creates a new thread object
 *
 * @param tid The thread ID
 * @param worker The worker object managing this thread
 * @return The new thread object
 */
rthread_s* rthread_new(const long tid, void *worker);

/**
 * @brief Updates a thread object after recycling it
 *
 * @param t The thread object
 * @param tid The new thread ID
 */
long rthread_update(rthread_s *t, const long tid);

/**
 * @brief Terminates the thread
 *
 * This function waits (via join) for the pthread to complete all its
 * current tasks from the queue.
 *
 * @param t The thread object
 */
void rthread_terminate(rthread_s* t);

/**
 * @brief Destroys the thread object
 *
 * @param t The thread object
 */
void rthread_destroy(rthread_s* t);

/**
 * @brief Inserts a task into the threads work queue
 *
 * Inserts a task into the threads work queue. We use an atomic ring buffer
 * data structure for the work queue. The ring buffer does not require any
 * mutex locks.
 *
 * @param t The thread object
 * @param task The task to be inserted
 * @return Returns true on success, returns false if the task queue is full
 */
bool rthread_insert_task(rthread_s* t, rtask_s* task);

/**
 * @brief Used by the pthread to process a task
 *
 * In this function the pthread will attempt to process a task. It extracts all
 * required information from the task object and invokes the corresponding I/O
 * syscalls.
 *
 * @param t The responsible thread object
 * @param task The task object
 * @param pthread_id The current pthread id
 */
void rthread_process_task(rthread_s* t, rtask_s *task, pid_t pthread_id);

/**
 * @brief The entry function for the pthreads
 *
 * @param data The data structure passed to the pthread
 * @return The exit code of the pthread.
 */
void *rthread_pthread_start(void *data);

#endif // RTHREAD_H
