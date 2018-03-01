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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include "defaults.h"

/**
 * @brief The options definition, used to store user input
 */
typedef struct options_s_ {
    char *capture_file; /**< The name of the .capture file */
    char *replay_file; /**< The name of the .replay file */
    char *stats_file; /**< The name of the .stats file */
    bool write_stats_file; /**< Write a stats file at the end of the test */
    char *user; /**< The user name to run the test as */
    char *name; /**< The name of the test (found in .ioreplay/name sub-dirs) */
    char *wd_base; /**< The working directory base */
    int num_workers; /**< The amount of worker processes */
    int num_threads_per_worker; /**< Max threads per worker processes */
    bool init; /**< If set ioreplay will initialise the environment */
    bool replay; /**< If set ioreplay will run/replay the test */
    bool purge; /**< If set ioreplay will purge the environment */
    bool trash; /**< If set ioreplay will trash the environment */
    bool drop_caches; /**< True if  ioreplay should drop all Linux caches */
    double speed_factor; /**< Specifies how fast the test is replayed */
    int pid; /**< Specifies a process id to capture */
    char *module; /**< Specifies the kernel module for capturing */
} options_s;

/**
 * @brief Creates a new options object
 *
 * The options object contains all options specified by the user as a command
 * line option. It is filled with default values during creation.
 *
 * @return The options object
 */
options_s *options_new();

/**
 * @brief Destroys the options object
 *
 * @param o The options object
 */
void options_destroy(options_s *o);

#endif // OPTIONS_H
