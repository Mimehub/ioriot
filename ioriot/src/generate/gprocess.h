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

#ifndef GPROCESS_H
#define GPROCESS_H

#include "../datas/hmap.h"
#include "../defaults.h"
#include "gtask.h"
#include "generate.h"

// Forward declarations (header include hell)
struct gtask_s_;
struct generate_s_;

/**
 * @brief Virtual process object used for generating .replay file
 *
 * An object of this represents a Linux process in generate context.
 * In Linux every process owns * its own file descriptor table which is
 * simulated here. Usually, a Linux process re-uses a FD number once not used
 * anymore (e.g. after a close). However, as we want to increase concurrency
 * while replaying the I/O we want * to ensure to always use unique file
 * descriptor IDs for every open. Thats why we use max_mapped_fd to always
 * map a real FD number to a uniq virtual FD number.
 */
typedef struct gprocess_s_ {
    long pid; /**< The real PID */
    long mapped_pid; /**< The mapped PID */
    hmap_s *vfd_map; /**< All virtual file descriptors of that process */
    hmap_s *fd_map; /**< All mappings from real fd to virtual fd */
    long max_mapped_fd; /**< The max mapped fd number */
} gprocess_s;

/**
 * @brief Creates a new gprocess object
 *
 * @param pid The process ID
 * @param mapped_pid the mapped PID
 * @return The new gprocess object
 */
gprocess_s* gprocess_new(const long pid, const long mapped_pid);

/**
 * @brief Destroys a gprocess object
 *
 * @param gp The gprocess object
 */
void gprocess_destroy(gprocess_s *gp);

/**
 * @brief Creates a new virtual FD from a given real FD number
 *
 * In ioriot we map the real file descriptor (the fd number protocolled in
 * the.capture file) to a virtual file descriptor (the fd numner written to the
 * .replay file). The purpose is to increase concurrency of the I/O during
 * replay. Normally, a process would reuse the same file descriptor number
 * once closed earlier. However, when replaying we can't reuse the number if
 * we want to replay the I/O on multiple paths in parallel. Therefore, it is
 * ensured that the virtual file descriptor number in the .replay file is
 * always * unique for every open!
 *
 * @param gp The process object
 * @param t The task object (the vfd pointer will be stored to * t->vfd)
 * @param g The generate object
 */
void gprocess_create_vfd_by_realfd(gprocess_s *gp, struct gtask_s_ *t,
                                   struct generate_s_ *g);

/**
 * @brief Retrieves a virtual FD from a given real FD number
 *
 * @param gp The process object
 * @param t The task object (the vfd pointer will be stored to * t->vfd)
 * @return SUCCESS if everything went smothly!
 */
status_e gprocess_vfd_by_realfd(gprocess_s *gp, struct gtask_s_ *t);

#endif // GPROCESS_H
