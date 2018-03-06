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

#ifndef RTASK_H
#define RTASK_H

#include "../defaults.h"

/**
 * @brief The replay task definition
 *
 * The rtask holds all possible variables required to process a particular
 * .replay line and to replay the corresponding I/O operation.
 */
typedef struct rtask_s_ {
    void *worker; /* The responsible worker object */
    void *process; /* The responsible process object */
    unsigned long lineno; /**< The current line number */
    unsigned long vsize; /**< The vsize */
    char *toks[MAX_TOKENS+1]; /**< The tokens parsed from the .replay line */
    char line[MAX_LINE_LEN];  /**< The remaining part of the .replay line */
#ifdef RTASK_DEBUG
    char *clone; /**< Used for debug purposes only */
#endif
} rtask_s;

/**
 * @brief Creates a new thread task object
 *
 * This function creates a new thread task object. Such a task object is used
 * by the worker to hand over I/O tasks to the corresponding threads. The
 * actual I/O work is performed by the threads then.
 *
 * @return The new thread task object
 */
rtask_s* rtask_new();

/**
 * @brief Destroys the replay task object
 *
 * @param t The thread task object to be destroyed
 */
void rtask_destroy(rtask_s* t);

/**
 * @brief Updates a reused/recycle task object
 *
 * @param task The task object to be updated
 * @param worker The responsibe worker object
 * @param process The responsible process object
 * @param line The remaining line of the .replay file
 * @param lineno The current line number of the .replay file
 * @param vsize The vsize/path id
 */
void rtask_update(rtask_s *task, void *worker, void *process, char *line,
                  const long lineno, const long vsize);

#endif // RTASK_H
