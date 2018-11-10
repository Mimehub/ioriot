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

/**
 * @brief Definition of a tpool
 */
typedef struct tpool_s_ {
    int max_threads; /**< Max amount of threads */
} tpool_s;

/**
 * @brief Creates a new thread pool
 *
 * @param max_threads The max amount of threads to use
 * @return The thread pool
 */
tpool_s *tpool_new(int max_threads);

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
 * @param callback The function to call on the provided data
 * @param data Data passed to the callback function
 * @param data2 More data passed to the callback function
 * @param data3 More more data passed to the callback function
 */
void tpool_add_work3(tpool_s* t, void (*callback)(void *data, void *data2, void *data3), void *data, void *data2, void *data3);

/**
 * @brief Runs thread pool unit tests
 */
void tpool_test();

#endif // TPOOL_H
