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
 * @file rstats.h
 * @author Paul Buetow
 *
 * @brief For collecting replay stats
 */

#ifndef RSTATS_H
#define RSTATS_H

#include "../defaults.h"
#include "../options.h"

#include <pthread.h>

/**
 * @brief Definition of the rstats object
 *
 * Used to store global statistics.
 */
typedef struct rstats_s_ {
    double loadavg_high; /**< Highest load average */
    long ioops; /**< Total amount if io operations */
    double duration; /**< Duration of the test */
    long time_ahead; /**< Time ahead of the original speed */
    struct timeval start_time; /**< Start time of the test */
    struct timeval end_time; /**< End time of the test */
    options_s *opts; /**< The I/O replay options object */
    FILE *stats_fd; /**< The file descriptor for writing the stats */
} rstats_s;

/**
 * @brief Definition of the per worker stats object
 *
 * Used to store per worker process I/O stats
 */
typedef struct rworker_stats_s_ {
    double loadavg_high; /**< Highest amount of io ops per second */
    long ioops; /**< Total amount if io operations */
    long time_ahead; /**< Time ahead of the original speed */
} rworker_stats_s;

/**
 * @brief Creates a new stats object
 *
 * @return The new stats object
 */
rstats_s* rstats_new(options_s *opts);

/**
 * @brief Destroys the stats object
 *
 * @param s The stats object
 */
void rstats_destroy(rstats_s* s);

/**
 * @brief Creates a new per worker stats object
 *
 * The memory is mapped into shared memory so it can be shared across multiple
 * processes.
 *
 * @return The new stats object
 */
rworker_stats_s* rworker_stats_new_mmap();

/**
 * @brief Destroys the per worker stats object
 *
 * @param s The stats object
 */
void rworker_stats_destroy(rworker_stats_s* s);

/**
 * @brief Starts the stats
 *
 * @param s The stats object
 */
void rstats_start(rstats_s* s);

/**
 * @brief Finalises the stats
 *
 * @param s The stats object
 */
void rstats_stop(rstats_s* s);

/**
 * @brief Prints the stats
 *
 * @param s The stats object
 */
void rstats_print(rstats_s* s);

/**
 * @brief Adds per worker stats to the global stats object
 *
 * @param s The global stats object
 * @param w The worker stats object
 */
void rstats_add_from_worker(rstats_s* s, rworker_stats_s* w);

#endif
