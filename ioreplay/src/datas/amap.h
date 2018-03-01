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

#ifndef AMAP_H
#define AMAP_H

#include "../defaults.h"

#define AMAP_MAX_ARRAY_LENGTH 1024*8

/**
 * @brief Implements an array map data structure
 *
 * This array map can hold a HUGE amount of entries by allocating multiple
 * smaller arrays. There are two version of the amap data structure available:
 * a memory mapped (mmap) and a normal version. The memory mapped version can
 * be used for IPC between various processes.
 */
typedef struct amap_s_ {
    void*** arrays; /**< The pointers to the amap arrays */
    int num_arrays; /**< The amount of arrays used in the amap */
    long size; /**< The total size/capacity of the amap */
    bool mmapped; /**< True if amap is memory mapped */
    void (*data_destroy)(void *data); /**< Callback to destroy all elements */
} amap_s;

amap_s* amap_new(const long size);
amap_s* amap_new_mmapped(const long size);
int amap_set(amap_s *a, const long position, void* value);
void* amap_get(amap_s *a, const long position);
void* amap_unset(amap_s *a, const long position);
void amap_print(amap_s *a);
void amap_destroy(amap_s *a);
void amap_reset(amap_s *a);
void amap_run_cb(amap_s *a, void (*cb)(void *data));
void amap_test(void);

#endif // AMAP_H
