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

#ifndef ITASK_H
#define ITASK_H

#include "../defaults.h"

/**
 * @brief The initialise task definition
 */
typedef struct itask_s_ {
    bool is_dir;
    bool is_file;
    long offset;
    long bytes;
    char *path;
    long dirs_created;
    long files_created;
    long sizes_created;
} itask_s;

/**
 * @brief Creates a new task object
 *
 * @return The new task object
 */
itask_s* itask_new();

/**
 * @brief Resets the task stats
 *
 * @param task The itask object
 */
void itask_reset_stats(itask_s *task);

/**
 * @brief Extract stats from a task object
 *
 * @param task The itask object
 * @param dirs_created Adds count of dirs created to that variable
 * @param files_created Adds count of files created to that variable
 * @param files_total_size Adds size of files created to that variable
 */
void itask_extract_stats(itask_s *task, long* dirs_created, long *files_created,
                         long *files_total_size);

/**
 * @brief Destroys a given task object
 *
 * @param task The task object
 */
void itask_destroy(itask_s *task);

/**
 * @brief Prints a task to stdout
 *
 * @param task The task object
 */
void itask_print(itask_s *task);

#endif // ITASK_H
