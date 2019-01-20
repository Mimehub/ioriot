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

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include "version.h"
#include "utils/utils.h"

/** Max amount of tokens per line in the .capture file */
#define MAX_TOKENS 10
/** Max line length in either .capture or .replay file */
#define MAX_LINE_LEN 1024*8
/** Controls how many tasks can be queued and buffered per worker thread */
#define TASK_BUFFER_PER_THREAD 512
/** Max open files resource user limit */
#define SET_RLIMIT_NOFILE 369216
/** Max processes resource user limit */
#define SET_RLIMIT_NPROC 30768
/** Ignore file hole size */
#define IGNORE_FILE_HOLE_BYTES 1024*1024*10
/** Max mmap file size */
#define MAX_MMAP_SIZE 1024*1024*1024  // 1tb

// The following are for debugging purposes only

//#define NO_IOOP
//#define THREAD_DEBUG //#define LOG_FILTERED

/**
 * @brief Return status codes
 */
typedef enum status_e_ {
    SUCCESS, /**< Great success! */
    UNKNOWN, /**< Unknown return status :-/ */
    ERROR, /**< An error happened :-( */
} status_e;

#endif // DEFAULTS_H
