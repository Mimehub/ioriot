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

#ifndef GTASK_H
#define GTASK_H

#include "vsize.h"

#include "gprocess.h"
#include "../vfd.h"
#include "../datas/amap.h"
#include "../datas/hmap.h"
#include "../datas/rbuffer.h"
#include "../defaults.h"
#include "../mounts.h"
#include "../options.h"

/**
 * @brief The generate task definition
 *
 * The gtask holds all possible variables required to process a particular
 * .capture line and to generate the corresponding .replay line
 */
typedef struct gtask_s_ {
    bool has_fd; /**<  True if task has a file descriptor number */
    char *line; /**< A pointer to the remaining part of the .capture line */
    char *op; /**< Operation/syscall name */
    char *path2; /**< A second path name (e.g. for rename) */
    char *path2_r; /**< Work around to track mallocs, so it can be freed */
    char *path; /**< Path name */
    char *path_r; /**< Work around to track mallocs, so it can be freed */
    char *pidtid; /**< String representing pid:tid */
    int F; /**< Arguments for fcntl syscall */
    int G; /**< Arguments for fcntl syscall */
    int fd; /**< File descriptor number */
    int flags; /**< File open flags */
    int mode; /**< File open mode */
    int ret; /**< ioriot process status, SUCCESS if everything is alright */
    int status; /**< Operation/syscall return status */
    long address2; /**< Another address (used by mmap related syscalls) */
    long address; /**< An address (used by mmap related syscalls) */
    long bytes; /**< Amount of bytes */
    long count; /**< A count */
    long lineno; /**<  The current line number */
    long mapped_fd; /**< The mapped file descriptor number */
    long mapped_time; /**< The mapped time */
    long offset; /**< A offset */
    long pid; /**< The process ID */
    long tid; /**< The thread ID */
    long whence; /**< Whence */
    vfd_s *vfd; /**< A pointer to the virtual file descriptor */
    struct gprocess_s_ *gprocess; /**< A pointer to the process object */
    void *generate; /**< A pointer to the generate object */
    vsize_s *vsize2; /**< Pointer to a second virtual size object */
    vsize_s *vsize; /**< Pointer to the virtual size object */
#ifdef LOG_FILTERED
    char *original_line; /**< Only used for debugging purposes */
    char *filtered_where; /**< Only used for debugging purposes  */
#endif
} gtask_s;

/**
 * @brief Creates a new task object
 *
 * @param generate A pointer to the generate object
 * @return The new task object
 */
gtask_s* gtask_new(void *generate);

/**
 * @brief Initialises a taks object
 *
 * This function is used in particular when we recycle/reuse an old
 * gtask object.
 *
 * @param t The gtask object
 * @param line The corresponding line from the .capture file
 * @param lineno The line number
 */
void gtask_init(gtask_s *t, char *line, const unsigned long lineno);

/**
 * @brief Destroys a given task object
 *
 * @param t The task object
 */
void gtask_destroy(gtask_s *t);

#endif // GTASK_H
