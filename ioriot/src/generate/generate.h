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

#ifndef GENERATE_H
#define GENERATE_H

#include "gwriter.h"
#include "../datas/rbuffer.h"
#include "../datas/hmap.h"
#include "../datas/graph.h"
#include "../defaults.h"
#include "../mounts.h"
#include "../options.h"

// Forward declarations (header include hell)
struct gtask_s_;

/**
 * @brief The generate object definition
 *
 * This is the general data structure required to generate a .replay file from
 * the .capture file.
 */
typedef struct generate_s_ {
    long lineno; /**< The current line number */
    long num_lines_filtered; /**< The amount of lines filtered out */
    long start_time; /**< The start time from the .capture file */
    char *name; /**< The name of the test specified by the user */
    FILE *replay_fd; /**< The fd of the .replay file */
    mounts_s *mps; /**< The mounts object */
    options_s *opts; /**< A pointer to the options object */
    hmap_s *fd_map; /**< The file descriptor map */
    rbuffer_s *reuse_queue; /**< A task buffer, for reusing these */
    graph_s *graph; /**< The in-memory execution graph */
    struct gwriter_s_ *writer; /**< A pointer to the writer object */
} generate_s;

/**
 * @brief Creates a new generate object
 *
 * @param opts The options object
 * @return The new generate object
 */
generate_s* generate_new(options_s *opts);

/**
 * @brief Destroys a generate object
 *
 * @param g The generate object to destroy
 */
void generate_destroy(generate_s* g);

/**
 * @brief Generates a .replay file from a .capture file
 *
 * @param opts The options object
 * @return SUCCESS on success
 */
status_e generate_run(options_s *opts);

#endif
