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

#ifndef TPOOL_H
#define TPOOL_H

#include "../defaults.h"
#include "../datas/rbuffer.h"

/**
 * @brief Definition of a tpool
 */
typedef struct tpool_s_ {
    pthread_mutex_t mutex; /**< To synchronize pool operations */
    pthread_cond_t not_full; /**< To synchronize pool operations */
    pthread_cond_t not_empty; /**< To synchronize pool operations */
    pthread_t *threads; /**< Holds the POSIX threads */
    _Bool terminate; /**< Determines whether threads should termiate */
    void (*callback)(void *data, void *data2, void *data3); /**< The callback */
    int num_threads; /**< Amount of threads */
    rbuffer_s *datas; /**< The data to run the functions with */
    rbuffer_s *datas2; /**< More data to run the functions with */
    rbuffer_s *datas3; /**< More more data to run the functions with */
} tpool_s;

/**
 * @brief Creates a new thread pool
 *
 * @param num_threads Amount of threads used
 * @param callback The function to call on the provided data
 * @return The thread pool
 */
tpool_s *tpool_new(int num_threads, void (*callback)(void *data, void *data2, void *data3));

/**
 * @brief Destroys a thread pool
 *
 * This function first joins all threads and then destroys the thread pool.
 *
 * @param t The thread pool
 */
void tpool_destroy(tpool_s* t);

/**
 * @brief Join all threads
 *
 * Wait for all threads to finish their work
 *
 * @param t The thread pool
 */
void tpool_join_threads(tpool_s* t);

/**
 * @brief Add work to the thread pool
 *
 * First thread available will grab the work added
 *
 * @param t The thread pool
 * @param data Data passed to the callback function
 * @param data2 More data passed to the callback function
 * @param data3 More more data passed to the callback function
 */
void tpool_add_work3(tpool_s* t, void *data, void *data2, void *data3);

/**
 * @brief Runs thread pool unit tests
 */
void tpool_test();

#endif // TPOOL_H
