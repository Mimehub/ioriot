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

#ifndef RPROCESS_H
#define RPROCESS_H

#include "../datas/hmap.h"
#include "../datas/amap.h"
#include "../defaults.h"
#include "rthread.h"

/**
 * @brief The virtual replay process object definition
 *
 * This defines a virtual process in replay context.
 */
typedef struct rprocess_s_ {
    int terminate; /**< Indicates whether the worker is terminating or not */
    int rworker_num; /**< The worker number of the responsible worker */
    int pid; /**< The virtual process ID */
    unsigned long lineno; /**< Holding the current .replay line number */
    bool initm; /**< Indicates whether ioreplay is in init mode or not */
    amap_s *fds_map; /**< Holding all file descriptors */
} rprocess_s;

rprocess_s* rprocess_new(const int pid, amap_s *fds_map);
void rprocess_destroy(rprocess_s* p);

#endif // RPROCESS_H
