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

#ifndef RBUFFER_H
#define RBUFFER_H

#include "signal.h"

#include "../defaults.h"

/**
 * @brief An atomic ring buffer data type definition
 *
 * This data structure can be used for the common producer/consumer problem.
 * As long as there is only max one producer thread and max one consumer thread
 * it can be used without any mutex locking. All the operations are atomic.
 */
typedef struct rbuffer_s_ {
    /**
     * The positions are atomic, means the ring buffer can be accessed from
     * multiple threads concurrently (one producer and one consumer thread).
     * This is the current read position.
     */
    sig_atomic_t read_pos;
    /**
     * This is the current write position.
     */
    sig_atomic_t write_pos;
    /**
     * Holds the pointers to the actual ring data stored in the ring buffer
     */
    void **ring;
    /**
     * Determines how many elements the ring buffer can hold. The capacity
     * will be size-1 though, as we need one empty slot.
     */
    int size;
} rbuffer_s;

/**
 * @brief Creates a new ring buffer
 *
 * @param size The size of the ring buffer
 * @return The new ring buffer object
 */
rbuffer_s* rbuffer_new(const int size);

/**
 * @brief Destroys a ring buffer
 *
 * @param r The ring buffer object
 */
void rbuffer_destroy(rbuffer_s* r);

/**
 * @brief Inserts data pointer to the ring buffer
 *
 * @param r The ring buffer object
 * @param data The data pointer
 * @return Returns false if element count not be inserted, true otherwise
 */
bool rbuffer_insert(rbuffer_s* r, void *data);

/**
 * @brief Determines whether there is any data in the ring buffer
 *
 * @param r The ring buffer object
 * @return True if there is any data, false otherwise
 */
bool rbuffer_has_next(rbuffer_s* r);

/**
 * @brief Returns and removes the next element from the ring buffer
 *
 * @param r The ring buffer object
 * @return The data pointer
 */
void* rbuffer_get_next(rbuffer_s* r);

/**
 * @brief Prints a ring buffer
 *
 * @param r The ring buffer object
 */
void rbuffer_print(rbuffer_s* r);

/**
 * @brief Unit tests the ring buffer
 */
void rbuffer_test(void);

#endif // RBUFFER_H
