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

#ifndef ITHREAD_H
#define ITHREAD_H

#include "../defaults.h"
#include "../datas/rbuffer.h"

#include "init.h"
#include "itask.h"

#include <pthread.h>

/**
 * @brief Definition of an init thread
 *
 */
typedef struct ithread_s_ {
    pthread_t pthread; /**< We run the init tasks in concurrent pthreads */
    rbuffer_s *queue; /**< The thread's task queue */
    init_s *init; /**< The responsible init object */
    bool terminate; /**< Indicates that thread can terminate */
} ithread_s;

/**
 * @brief Creates a new thread object
 *
 * @param i The init object
 * @return The new thread object
 */
ithread_s* ithread_new(init_s *i);

/**
 * @brief Terminates the thread
 *
 * This function waits (via join) for the pthread to complete all its
 * current tasks from the queue.
 *
 * @param t The thread object
 */
void ithread_terminate(ithread_s* t);

/**
 * @brief Destroys the thread object
 *
 * @param t The thread object
 */
void ithread_destroy(ithread_s* t);

/**
 * @brief Executes the init task
 *
 * @param t The thread object
 * @param task The task object
 */
void ithread_run_task(ithread_s* t, itask_s *task);

/**
 * @brief Starts the POSIX thread
 *
 * @param t The responsible thread object
 */
void ithread_start(ithread_s *t);

/**
 * @brief Entry point of the POSIX thread
 *
 * @param data Data passed to the pthread
 * @return Always NULL on success
 */

void* ithread_pthread_start(void *data);

#endif // ITHREAD_H
