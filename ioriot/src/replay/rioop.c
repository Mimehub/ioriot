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

#include "rioop.h"

#include "../vfd.h"
#include "rworker.h"

// Printing error messages
#define _Error(...) \
  fprintf(stderr, "%s:%d ERROR: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\nlineno:%ld path:%s\n", task->lineno, vfd->path); \
  fflush(stdout); \
  fflush(stderr); \
  exit(ERROR);

#define _Errno(...) \
  fprintf(stderr, "%s:%d ERROR: %s (%d). ", __FILE__, __LINE__, \
      strerror(errno), errno); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\nlineno:%ld path:%s\n", task->lineno, vfd->path); \
  fflush(stdout); \
  fflush(stderr); \
  exit(ERROR);

#define _Init_arg(num) int arg = atoi(task->toks[num])
#define _Init_cmd(num) int cmd = atoi(task->toks[num])
#define _Init_fd(num) long fd = atol(task->toks[num])
#define _Init_flags(num) int flags = atoi(task->toks[num])
//#define _Init_mode(num) int mode = atoi(task->toks[num])
#define _Init_offset(num) long offset = atol(task->toks[num])
#define _Init_op(num) int op = atoi(task->toks[num])
#define _Init_path2(num) char *path2 = task->toks[num]
#define _Init_path(num) char *path = task->toks[num]
#define _Init_rc(num) int rc = atoi(task->toks[num])
#define _Init_whence(num) long whence = atol(task->toks[num])

#define _Init_bytes(num) \
    int bytes = atoi(task->toks[num]); \
    if (bytes <= 0) return

#define _Init_virtfd \
    vfd_s *vfd = amap_get(p->fds_map, fd); \
    if (vfd == NULL) return

void rioop_run(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_op(2);

    switch (op) {
    // stat() syscalls
    case FSTAT:
        rioop_fstat(p, t, task);
        break;
    case FSTATFS:
    case FSTATFS64:
        //Error("op(%d) not implemented", op);
        break;
    case FSTAT_AT:
    case LSTAT:
    case STAT:
        rioop_stat(p, t, task);
        break;
    case STATFS:
    case STATFS64:
        //Error("op(%d) not implemented", op);
        break;

    // read() syscalls
    case READ:
    case READV:
        rioop_read(p, t, task);
        break;
    case READAHEAD:
        //Error("op(%d) not implemented", op);
        break;
    case READLINK:
    case READLINK_AT:
        //Error("op(%d) not implemented", op);
        break;

    // write() syscalls
    case WRITE:
    case WRITEV:
        rioop_write(p, t, task);
        break;

    // open() and other syscalls which may creat
    case OPEN:
    case OPEN_AT:
        rioop_open(p, t, task, -1);
        break;
    case CREAT:
        // A call to crat() is equivalent to calling open() with flags..
        rioop_open(p, t, task, O_CREAT|O_WRONLY|O_TRUNC);
        break;
    case MKDIR:
    case MKDIR_AT:
        rioop_mkdir(p, t, task);
        break;

    // rename() syscalls
    case RENAME:
    case RENAME_AT:
    case RENAME_AT2:
        rioop_rename(p, t, task);
        break;

    // close() and unlink() syscalls
    case CLOSE:
        rioop_close(p, t, task);
        break;
    case UNLINK:
    case UNLINK_AT:
        rioop_unlink(p, t, task);
        break;
    case RMDIR:
        rioop_rmdir(p, t, task);
        break;

    // sync() syscalls
    case FSYNC:
        rioop_fsync(p, t, task);
        break;
    case FDATASYNC:
        rioop_fdatasync(p, t, task);
        break;
    case SYNC:
    case SYNCFS:
    case SYNC_FILE_RANGE:
        //Error("op(%d) not implemented", op);
        break;

    // Other syscalls
    case FCNTL:
        rioop_fcntl(p, t, task);
        break;
    case GETDENTS:
        rioop_getdents(p, t, task);
        break;
    case LSEEK:
    case LLSEEK:
        rioop_lseek(p, t, task);
        break;

    // chmod() syscalls
    case CHMOD:
        rioop_chmod(p, t, task);
        break;
    case FCHMOD:
        rioop_fchmod(p, t, task);
        break;

    // chown() syscalls
    case CHOWN:
        rioop_chown(p, t, task);
        break;
    case FCHOWN:
    case FCHOWNAT:
        rioop_fchown(p, t, task);
        break;
    case LCHOWN:
        rioop_lchown(p, t, task);
        break;

    // Meta operations (I/O replay internal use only).
    case META_EXIT_GROUP:
        break;
    case META_TIMELINE:
        break;

    default:
        Error("op(%d) not implemented", op);
        break;
    }
}

void rioop_stat(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_path(3);
    struct stat buf;
    stat(path, &buf);
}

void rioop_fstat(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_virtfd;
    struct stat buf;
    fstat(vfd->fd, &buf);
}

void rioop_rename(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_path(3);
    _Init_path2(4);
    rename(path, path2);
}

void rioop_read(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_bytes(4);
    _Init_virtfd;

    char *buf = Calloc(bytes+1, char);
    read(vfd->fd, buf, bytes);
    free(buf);
}

void rioop_write(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_bytes(4);
    _Init_virtfd;

    char *buf = Calloc(bytes+1, char);
    sprintf(buf, "%ld", task->lineno);
    Fill_with_stuff(buf, bytes);
    if (vfd->fd == 0) {
        Debug("%d %d %ld", vfd->fd, vfd->debug, task->lineno);
        _Error("ERROR");
    }
    write(vfd->fd, buf, bytes);
    free(buf);
}

void rioop_open(rprocess_s *p, rthread_s *t, rtask_s *task, int flags_)
{
    _Init_fd(3);
    _Init_path(4);
    _Init_flags(6);

    // Special case as this is creat() now
    if (flags_ != -1)
        flags = flags_;

    bool directory = Has(flags, O_DIRECTORY);

    if (fd > 0) {
        if (directory) {
            // We can not open a directory via open() otherwise!
            flags &= (O_RDONLY & ~(O_RDWR|O_WRONLY|O_CREAT));
        } else {
            // We don't want to open the file in read only mode.
            // SystemTap could have skipped syscalls to fcntl or open
            flags &= ~O_RDONLY;
        }
        //    flags |= O_DIRECT|O_SYNC;
        flags &= ~O_EXCL;
    }

    int ret = open(path, flags, S_IRWXU|S_IRWXG|S_IRWXO);

    if (fd < 0 && ret > 0) {
        close(ret);
#ifdef THREAD_DEBUG
        fprintf(t->rthread_fd, "TRACE OPEN|open+close|%s|\n", path);
        fflush(t->rthread_fd);
#endif
    }

    if (fd > 0 && ret > 0) {
        vfd_s *vfd = vfd_new(ret, fd, path);
        amap_set(p->fds_map, fd, vfd);

#ifdef THREAD_DEBUG
        fprintf(t->rthread_fd, "TRACE OPEN|open|%s|\n", path);
        fflush(t->rthread_fd);
#endif
    }
}

void rioop_close(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_virtfd;

    amap_unset(p->fds_map, fd);
    if (vfd->dirfd) {
        closedir(vfd->dirfd);
#ifdef THREAD_DEBUG
        fprintf(t->rthread_fd, "TRACE OPEN|closedir|%s|\n", vfd->path);
        fflush(t->rthread_fd);
#endif
    } else {
        close(vfd->fd);
#ifdef THREAD_DEBUG
        fprintf(t->rthread_fd, "TRACE OPEN|close|%s|\n", vfd->path);
        fflush(t->rthread_fd);
#endif
    }
    vfd_destroy(vfd);
}

void rioop_getdents(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_virtfd;

    // getdents expects a dirfd
    DIR *dirfd = fdopendir(vfd->fd);
    if (dirfd) {
        vfd->dirfd = dirfd;
        readdir(dirfd);
#ifdef THREAD_DEBUG
        fprintf(t->rthread_fd, "TRACE OPEN|fdopendir|%s|\n", vfd->path);
        fflush(t->rthread_fd);
#endif
    }
}

void rioop_mkdir(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_path(3);
    mkdir(path, S_IRWXU|S_IRWXG|S_IRWXO);
}

void rioop_unlink(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_path(3);
    unlink(path);
}

void rioop_rmdir(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_path(3);
    rmdir(path);
}

void rioop_lseek(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_bytes(6);
    _Init_virtfd;
    lseek(vfd->fd, bytes, SEEK_SET);
}

void rioop_fsync(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_virtfd;
    fsync(vfd->fd);
}

void rioop_fdatasync(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_virtfd;
    fdatasync(vfd->fd);
}

void rioop_fcntl(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_cmd(4);
    _Init_arg(5);
    _Init_virtfd;

    switch (cmd) {
    case F_GETFD:
    case F_GETFL:
        fcntl(vfd->fd, cmd);
        break;
    case F_SETFD:
    case F_SETFL:
        fcntl(vfd->fd, cmd, arg);
        break;
    default:
        break;
    }
}

void rioop_chmod(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_path(3);
    chmod(path, S_IRWXU|S_IRWXG|S_IRWXO);
}

void rioop_fchmod(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_virtfd;
    fchmod(vfd->fd, S_IRWXU|S_IRWXG|S_IRWXO);
}

void rioop_chown(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_path(3);
    rworker_s *w = t->worker;
    options_s *opts = w->opts;
    struct passwd *pwd = getpwnam(opts->user);
    chown(path, pwd->pw_uid, -1);
}

void rioop_fchown(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_fd(3);
    _Init_virtfd;
    rworker_s *w = t->worker;
    options_s *opts = w->opts;
    struct passwd *pwd = getpwnam(opts->user);
    fchown(vfd->fd, pwd->pw_uid, -1);
}

void rioop_lchown(rprocess_s *p, rthread_s *t, rtask_s *task)
{
    _Init_path(3);
    rworker_s *w = t->worker;
    options_s *opts = w->opts;
    struct passwd *pwd = getpwnam(opts->user);
    lchown(path, pwd->pw_uid, -1);
}

