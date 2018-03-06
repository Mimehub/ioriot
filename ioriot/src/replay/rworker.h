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

#ifndef RWORKER_H
#define RWORKER_H

#include <pthread.h>

#include "../datas/amap.h"
#include "../datas/rbuffer.h"
#include "../defaults.h"
#include "../options.h"
#include "rstats.h"

/**
 * @brief Represents a worker process.
 *
 * This represents an I/O replay worker process. The user can specify the
 * amount of worker processes via the -p command line switch. This is not
 * to confuse with rprocess_s, which represents an original captured process
 * and we now want to replay the I/O for!
 */
typedef struct {
    int rworker_num; /**< The current worker ID */
    amap_s* fds_map; /**< Holding all file descriptors */
    amap_s* rprocess_map; /**< Holding all processes handled by this worker */
    amap_s* rthread_map; /**< Holding all threads handled by this worker */
    rbuffer_s *task_buffer; /**< Buffering thread tasks to be reused */
    pthread_mutex_t task_buffer_mutex; /**< To sync access to task_buffer */
    rbuffer_s *rthread_buffer; /**< Buffering idle threads to be reused */
    pthread_mutex_t rthread_buffer_mutex; /**< Sync access to rthread_buffer */
    options_s *opts; /**< To synchronise access to rthread_buffer */
    rworker_stats_s *worker_stats; /**< Object holding per worker statistics */
#ifdef RTHREAD_DEBUG
    FILE *rworker_fd; /**< For debugging purposes only */
#endif
} rworker_s;

/**
 * @brief Creates a new worker object
 *
 * @param rworker_num The worker number
 * @param fds_map A map of all virtual file descriptor objects
 * @param num_vsizes The amount of virtual sizes/total file paths of the test
 * @param num_pids The total amount of virtual process IDs used in this test
 * @param opts A pointer to the options object
 * @param worker_stats A pointer to the worker stats object

 * @return The new worker object
 */
rworker_s* rworker_new(const int rworker_num, amap_s *fds_map,
                       const long num_vsizes, const long num_pids,
                       options_s* opts, rworker_stats_s *worker_stats);

/**
 * @brief Destroys a worker object
 *
 * @param w The worker object to be destroyed
 */
void rworker_destroy(rworker_s* w);

/**
 * @brief Makes the worker to process all .replay lines
 *
 * @param w The responsible worker object
 * @param num_lines The total amount of I/O op lines in the .replay file
 * @return SUCCESS if everything went fine
 */
status_e rworker_process_lines(rworker_s* w, const long num_lines);

#endif // RWORKER_H
