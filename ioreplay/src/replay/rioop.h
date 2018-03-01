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

#ifndef RIOOP_H
#define RIOOP_H

#include "../defaults.h"
#include "../utils/futils.h"
#include "../opcodes.h"
#include "rprocess.h"
#include "rthread.h"

/**
 * @brief Replays the responsible I/O operation of a given task
 *
 * @param p The virtual replay process object
 * @param t The thread object
 * @param task The replay task object
 */
void rioop_run(rprocess_s *p, rthread_s *t, rtask_s *task);

void rioop_close(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_fcntl(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_fdatasync(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_fstat(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_fsync(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_getdents(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_mkdir(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_open(rprocess_s *p, rthread_s *t, rtask_s *task, int flags_);
void rioop_read(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_rename(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_stat(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_lseek(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_unlink(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_rmdir(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_write(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_chmod(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_fchmod(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_chown(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_fchown(rprocess_s *p, rthread_s *t, rtask_s *task);
void rioop_lchown(rprocess_s *p, rthread_s *t, rtask_s *task);

#endif // RIOOP_H
