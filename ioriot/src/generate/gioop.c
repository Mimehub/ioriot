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

#include "gioop.h"

#define _Require_path \
    if (t->path == NULL) { return ERROR; }

#define _Require_path_from_fd \
    if (!t->has_fd) { return ERROR; } \
    char *_Path = hmap_get(w->fd_map, t->fdid); \
    if (!_Path) { return ERROR; }

static void _graph_insert(gwriter_s *w, char *path, long offset)
{
    if (path)
        graph_insert(w->graph, path, &offset);
    //Out("%ld\n", offset);
}

status_e gioop_run(gwriter_s *w, gtask_s *t)
{
    status_e ret = SUCCESS;
    generate_s *g = w->generate;

    // There was already an error in the parser (parser.c) processing this
    // task! Don't process it futher.
    if (t->ret != SUCCESS) {
        Cleanup(t->ret);
    }

    if (Eq(t->op, "open")) {
        Cleanup(gioop_open(w, t, g));

    } else if (Eq(t->op, "openat")) {
        Cleanup(gioop_openat(w, t, g));

    } else if (Eq(t->op, "creat")) {
        Cleanup(gioop_creat(w, t, g));

    } else if (Eq(t->op, "close")) {
        Cleanup(gioop_close(w, t, g));

    } else if (Eq(t->op, "stat")) {
        Cleanup(gioop_stat(w, t, g));

    } else if (Eq(t->op, "statfs")) {
        Cleanup(gioop_statfs(w, t, g));

    } else if (Eq(t->op, "statfs64")) {
        Cleanup(gioop_statfs64(w, t, g));

    } else if (Eq(t->op, "fstat")) {
        Cleanup(gioop_fstat(w, t, g));

    } else if (Eq(t->op, "fstatat")) {
        Cleanup(gioop_fstatat(w, t, g));

    } else if (Eq(t->op, "fstatfs")) {
        Cleanup(gioop_fstatfs(w, t, g));

    } else if (Eq(t->op, "fstatfs64")) {
        Cleanup(gioop_fstatfs64(w, t, g));

    } else if (Eq(t->op, "rename")) {
        Cleanup(gioop_rename(w, t, g));

    } else if (Eq(t->op, "renameat")) {
        Cleanup(gioop_renameat(w, t, g));

    } else if (Eq(t->op, "renameat2")) {
        Cleanup(gioop_renameat2(w, t, g));

    } else if (Eq(t->op, "read")) {
        Cleanup(gioop_read(w, t, g));

    } else if (Eq(t->op, "readv")) {
        Cleanup(gioop_readv(w, t, g));

    } else if (Eq(t->op, "readahead")) {
        Cleanup(gioop_readahead(w, t, g));

    } else if (Eq(t->op, "readdir")) {
        Cleanup(gioop_readdir(w, t, g));

    } else if (Eq(t->op, "readlink")) {
        Cleanup(gioop_readlink(w, t, g));

    } else if (Eq(t->op, "readlinkat")) {
        Cleanup(gioop_readlinkat(w, t, g));

    } else if (Eq(t->op, "write")) {
        Cleanup(gioop_write(w, t, g));

    } else if (Eq(t->op, "writev")) {
        Cleanup(gioop_writev(w, t, g));

    } else if (Eq(t->op, "lseek")) {
        Cleanup(gioop_lseek(w, t, g));

    } else if (Eq(t->op, "llseek")) {
        Cleanup(gioop_llseek(w, t, g));

    } else if (Eq(t->op, "getdents")) {
        Cleanup(gioop_getdents(w, t, g));

    } else if (Eq(t->op, "mkdir")) {
        Cleanup(gioop_mkdir(w, t, g));

    } else if (Eq(t->op, "rmdir")) {
        Cleanup(gioop_rmdir(w, t, g));

    } else if (Eq(t->op, "mkdirat")) {
        Cleanup(gioop_mkdirat(w, t, g));

    } else if (Eq(t->op, "unlink")) {
        Cleanup(gioop_unlink(w, t, g));

    } else if (Eq(t->op, "unlinkat")) {
        Cleanup(gioop_unlinkat(w, t, g));

    } else if (Eq(t->op, "lstat")) {
        Cleanup(gioop_lstat(w, t, g));

    } else if (Eq(t->op, "fsync")) {
        Cleanup(gioop_fsync(w, t, g));

    } else if (Eq(t->op, "fdatasync")) {
        Cleanup(gioop_fdatasync(w, t, g));

    } else if (Eq(t->op, "sync")) {
        Cleanup(gioop_sync(w, t, g));

    } else if (Eq(t->op, "syncfs")) {
        Cleanup(gioop_syncfs(w, t, g));

    } else if (Eq(t->op, "sync_file_range")) {
        Cleanup(gioop_sync_file_range(w, t, g));

    } else if (Eq(t->op, "fcntl")) {
        Cleanup(gioop_fcntl(w, t, g));

    } else if (Eq(t->op, "fcntl")) {
        Cleanup(gioop_fcntl(w, t, g));

    } else if (Eq(t->op, "mmap2")) {
        // Support for mmap added later

    } else if (Eq(t->op, "munmap")) {
        // Support for mmap added later

    } else if (Eq(t->op, "mremap")) {
        // Support for mmap added later

    } else if (Eq(t->op, "msync")) {
        // Support for mmap added later

    } else if (Eq(t->op, "chmod")) {
        Cleanup(gioop_chmod(w, t, g));

    } else if (Eq(t->op, "fchmodat")) {
        Cleanup(gioop_chmod(w, t, g));

    } else if (Eq(t->op, "fchmod")) {
        Cleanup(gioop_fchmod(w, t, g));

    } else if (Eq(t->op, "chown")) {
        Cleanup(gioop_chown(w, t, g));

    } else if (Eq(t->op, "chown16")) {
        Cleanup(gioop_chown(w, t, g));

    } else if (Eq(t->op, "lchown")) {
        Cleanup(gioop_lchown(w, t, g));

    } else if (Eq(t->op, "lchown16")) {
        Cleanup(gioop_lchown(w, t, g));

    } else if (Eq(t->op, "fchown")) {
        Cleanup(gioop_fchown(w, t, g));

    } else if (Eq(t->op, "fchownat")) {
        Cleanup(gioop_chown(w, t, g));

    } else if (Eq(t->op, "exit_group")) {
        Cleanup(gioop_exit_group(w, t, g));

    } else {
        Cleanup(ERROR);
    }

cleanup:

#ifdef LOG_FILTERED
    if (ret != SUCCESS)
        t->filtered_where = __FILE__;
#endif

    t->ret = ret;
    return ret;
}

static status_e _gioop_open(gwriter_s *w, gtask_s *t, generate_s *g, char *str, int code)
{
    if (!t->has_fd || t->path == NULL || t->flags == -1) {
        return ERROR;
    }

    // Notify that process has a specific file descriptor open
    char *path = hmap_replace(w->fd_map, t->fdid, Clone(t->path));
    if (path) {
        // In case that the process already had the same fd open inject a close
        Owriter_write(w->owriter, "%d|%s|%s\n", CLOSE, t->fdid, "injected close");
        _graph_insert(w, t->path, Offset);
        free(path);
    }

    Owriter_write(w->owriter, "%d|%s|%s|%d|%d|%s\n", code, t->fdid, t->path, t->mode, t->flags, str);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_open(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_open(w, t, g, "open", OPEN);
}

status_e gioop_openat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_open(w, t, g, "openat", OPEN_AT);
}

status_e gioop_creat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_open(w, t, g, "creat", CREAT);
}

status_e gioop_close(gwriter_s *w, gtask_s *t, generate_s *g)
{
    if (!t->has_fd) {
        return ERROR;
    }

    char *path = hmap_remove(w->fd_map, t->fdid);

    if (!path) {
        // Not closing, as there was no such fd open
        return ERROR;
    }

    Owriter_write(w->owriter, "%d|%s|%s\n", CLOSE, t->fdid, "close");
    _graph_insert(w, path, Offset);

    return SUCCESS;
}

static status_e _gioop_stat(gwriter_s *w, gtask_s *t, generate_s *g, char *str, int code)
{
    if (t->path == NULL) {
        return ERROR;
    }

    Owriter_write(w->owriter, "%d|%s|%d|%s\n", code, t->path, t->status, str);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_stat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_stat(w, t, g, "stat", STAT);
}

status_e gioop_statfs(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_stat(w, t, g, "statfs", STATFS);
}

status_e gioop_statfs64(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_stat(w, t, g, "statfs64", STATFS64);
}

static status_e _gioop_fstat(gwriter_s *w, gtask_s *t, generate_s *g, char *str, int code)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%d|%s\n", code, t->fdid, t->status, str);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_fstat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_fstat(w, t, g, "fstat", FSTAT);
}

status_e gioop_fstatat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_stat(w, t, g, "fstatat", FSTAT_AT);
}

status_e gioop_fstatfs(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_fstat(w, t, g, "fstatfs", FSTATFS);
}

status_e gioop_fstatfs64(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_fstat(w, t, g, "fstatfs64", FSTATFS64);
}

static status_e _gioop_rename(gwriter_s *w, gtask_s *t, generate_s *g, char *str, int code)
{
    if (t->path == NULL || t->path2 == NULL ) {
        return ERROR;
    }

    Owriter_write(w->owriter, "%d|%s|%s|%s\n", code, t->path, t->path2, str);
    _graph_insert(w, t->path, Offset);
    _graph_insert(w, t->path2, Offset);

    return SUCCESS;
}

status_e gioop_rename(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_rename(w, t, g, "rename", RENAME);
}

status_e gioop_renameat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_rename(w, t, g, "renameat", RENAME_AT);
}
status_e gioop_renameat2(gwriter_s *w, gtask_s *t, generate_s *g)
{
    return _gioop_rename(w, t, g, "renameat2", RENAME_AT2);
}

status_e gioop_read(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%s\n", CLOSE, t->fdid, "close");
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_readv(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%s\n", READ, t->fdid, "readv");
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_readahead(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%ld|%ld|%s\n", READAHEAD, t->fdid, t->offset, t->count, "readahead");
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_readdir(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%d|%s\n", READDIR, t->fdid, t->status, "readdir");
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_readlink(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    Owriter_write(w->owriter, "%d|%s|%d|%s\n",
                  READLINK, t->path, t->status, "readlink");
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_readlinkat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    Owriter_write(w->owriter, "%d|%s|%d|%s\n",
                  READLINK_AT, t->path, t->status, "readlinkat");
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_write(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%ld|write",
                  WRITE, t->fdid, t->bytes);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_writev(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%ld|writev",
                  WRITEV, t->fdid, t->bytes);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_lseek(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%ld|%ld|%ld|lseek",
                  LSEEK, t->fdid, t->offset, t->whence, t->bytes);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_llseek(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%ld|%ld|%ld|llseek",
                  LLSEEK, t->fdid, t->offset, t->whence, t->bytes);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_getdents(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%ld|%ld|getdents",
                  GETDENTS, t->fdid, t->count, t->bytes);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_mkdir(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    Owriter_write(w->owriter, "%d|%s|%d|%d|mkdir",
                  MKDIR, t->path, t->mode, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_rmdir(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    Owriter_write(w->owriter, "%d|%s|%d|rmdir",
                  RMDIR, t->path, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_mkdirat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    Owriter_write(w->owriter, "%d|%s|%d|%d|mkdirat",
                  MKDIR_AT, t->path, t->mode, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_unlink(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    Owriter_write(w->owriter, "%d|%s|%d|unlink",
                  UNLINK, t->path, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_unlinkat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    Owriter_write(w->owriter, "%d|%s|%d|unlinkat",
                  UNLINK_AT, t->path, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_lstat(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    Owriter_write(w->owriter, "%d|%s|%d|lstat",
                  LSTAT, t->path, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_fsync(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%d|fsync",
                  FSYNC, t->fdid, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_fdatasync(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%d|fdatasync",
                  FDATASYNC, t->fdid, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_sync(gwriter_s *w, gtask_s *t, generate_s *g)
{
    Owriter_write(w->owriter, "%d|%d|sync", SYNC,  t->status);
    // TODO: Use last known existent path for global fsync
    _graph_insert(w, "%d|/", Offset);

    return SUCCESS;
}

status_e gioop_syncfs(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%d|syncfs",
                  SYNCFS, t->fdid, t->status);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_sync_file_range(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%ld|%ld|%d|sync_file_range",
                  SYNC_FILE_RANGE, t->fdid, t->offset, t->bytes, t->status);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_fcntl(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    switch (t->F) {
    case F_GETFD:
    case F_GETFL:
    case F_SETFD:
    case F_SETFL:
        break;
    default:
        return ERROR;
        break;
    }

    Owriter_write(w->owriter, "%d|%s|%d|%d|%d|fcntl",
                  FCNTL, t->fdid, t->F, t->G, t->status);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_chmod(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    Owriter_write(w->owriter, "%d|%s|%d|%d|chmod",
                  CHMOD, t->path, t->mode, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_fchmod(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    Owriter_write(w->owriter, "%d|%s|%d|%d|fchmod",
                  FCHMOD, t->fdid, t->mode, t->status);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_chown(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    // Hmm, maybe rename t->offset, because here it is used for the user UID
    Owriter_write(w->owriter, "%d|%s|%ld|%d|%d|chown",
                  CHOWN, t->path, t->offset, t->G, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

status_e gioop_fchown(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path_from_fd;

    // Hmm, maybe rename t->offset, because here it is used for the user UID
    Owriter_write(w->owriter, "%d|%s|%ld|%d|%d|fchown",
                  FCHOWN, t->fdid, t->offset, t->G, t->status);
    _graph_insert(w, _Path, Offset);

    return SUCCESS;
}

status_e gioop_lchown(gwriter_s *w, gtask_s *t, generate_s *g)
{
    _Require_path;

    // Hmm, maybe rename t->offset, because here it is used for the user UID
    Owriter_write(w->owriter, "%d|%s|%ld|%d|%d|chown",
                  LCHOWN, t->path, t->offset, t->G, t->status);
    _graph_insert(w, t->path, Offset);

    return SUCCESS;
}

static void _gioop_exit_group_cb(char *key, void *data, void *data2)
{
    char *path = data;
    gwriter_s *w = data2;

    Put("DEBUG CLOSE: %s", key);
    Owriter_write(w->owriter, "%d|%s|%s (%s)\n", CLOSE, key, "close on exit_group", path);
    _graph_insert(w, path, Offset);
}

status_e gioop_exit_group(gwriter_s *w, gtask_s *t, generate_s *g)
{
    // It means that the process and all its threads terminate.
    // Therefore close all file handles of that process!

    char *pidstr;
    Asprintf(&pidstr, "%ld:", t->pid);

    // Destroy all elements with keys including pidstr, run
    // callback function on data before destroying it.
    hmap_keys_destroy_cb(w->fd_map, pidstr, _gioop_exit_group_cb, w);
    free(pidstr);

    return SUCCESS;
}
