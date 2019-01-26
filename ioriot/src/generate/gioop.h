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

#ifndef GIOOP_H
#define GIOOP_H

#include "../defaults.h"
#include "gwriter.h"
#include "gtask.h"
#include "generate.h"


// Helper macro regarding writing the .replay file!

#define Gioop_write(op, ...) \
    fprintf(g->replay_fd, "%ld|%ld|%ld|0|0|%d|", \
            t->mapped_time, \
            (t->vsize ? t->vsize->id : 0),\
            t->gprocess->mapped_pid, \
            op); \
    fprintf(g->replay_fd, __VA_ARGS__); \
    fprintf(g->replay_fd, "@%ld", t->lineno); \
    fprintf(g->replay_fd, "|\n")

/**
 * @brief Function used when closing all virtual FDs of a virtual process
 *
 * This function is run on all virtual file handles whenever a virtual generate
 * process object (gprocess_s) gets destroyed. This is on an exit_group
 * syscall (a thread group, a process with all its threads, terminates). Upon
 * process termination Linux also closes all its file descriptors! This is what
 * we simulate here!
 *
 * @param data The pointer to the virtual file descriptor object
 * @param data2 The pointer to the corresponding generate task object.
 */
void gioop_close_all_vfd_cb(void *data, void *data2);

/**
 * @brief Run a generate I/O operation on a given task
 *
 * @param w The writer object
 * @param t The task object
 * @return SUCCESS if everything went fine
 */
status_e gioop_run(gwriter_s *w, gtask_s *t);

status_e gioop_open(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_openat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_creat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_close(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_stat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_statfs(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_statfs64(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_fstat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_fstatat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_fstatfs(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_fstatfs64(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_rename(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_renameat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_renameat2(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_read(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_readv(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_readahead(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_readdir(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_readlink(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_readlinkat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_write(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_writev(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_lseek(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_llseek(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_getdents(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_mkdir(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_rmdir(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_mkdirat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_unlink(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_unlinkat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_lstat(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_fsync(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_fdatasync(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_sync(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_syncfs(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_sync_file_range(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_fcntl(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_chmod(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_fchmod(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_chown(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_fchown(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_lchown(gwriter_s *w, gtask_s *t, generate_s *g);
status_e gioop_exit_group(gwriter_s *w, gtask_s *t, generate_s *g);

#endif // GIOOP_H
