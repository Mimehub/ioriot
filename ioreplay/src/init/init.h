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

#ifndef INIT_H
#define INIT_H

#include "../defaults.h"
#include "../options.h"
#include "../datas/amap.h"
#include "../datas/rbuffer.h"
#include "../mounts.h"

typedef struct init_s_ {
    amap_s *threads_map;
    rbuffer_s *reuse_queue;
    options_s *opts;
    mounts_s *mounts;
    FILE *replay_fd;
    pthread_mutex_t reuse_queue_mutex;
} init_s;

/**
 * @brief Creates a new init object
 *
 * @param opts The options object
 * @return The new mounts object
 */
init_s* init_new(options_s *opts);

/**
 * @brief Destroys the init object
 *
 * @param i The init object
 */
void init_destroy(init_s *i);

/**
 * @brief Initialises the test environment
 *
 * @param opts The options object
 * @return SUCCESS if initialised without any issues
 */
status_e init_run(options_s *opts);

/**
 * @brief Extracts some useful information from the .replay meta header
 *
 * @param i The init object
 * @param init_offset To store the offset of the init section
 */
void init_extract_header(init_s *i, off_t *init_offset);

#endif // INIT_H
