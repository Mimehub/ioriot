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

#ifndef REPLAY_H
#define REPLAY_H

#include "../defaults.h"
#include "../utils/futils.h"
#include "../opcodes.h"
#include "../options.h"
#include "rioop.h"
#include "rprocess.h"

/**
 * @brief Replays the given .replay file
 *
 * @param opts The options object
 * @return SUCCESS if everything went fine
 */
status_e replay_run(options_s *opts);

/**
 * @brief Extract required meta data from .replay's meta header
 *
 * @param opts The options object
 * @param replay_fd The file handle to the .replay file
 * @param num_vsizes The amount of virtual sizes/paths
 * @param num_pids The amount of process IDs
 * @param num_fds The amount of virtual file descriptors
 * @param num_lines The amount of .replay lines with I/O ops
 */
void replay_extract_header(options_s *opts, FILE *replay_fd, long *num_vsizes,
                           long *num_pids, long *num_fds,long *num_lines);

#endif // REPLAY_H
