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

#ifndef OPCODES_H
#define OPCODES_H

typedef enum {
    // stat() syscalls
    FSTAT = 0,
    FSTAT_AT,
    FSTATFS,
    FSTATFS64,
    LSTAT,
    STAT,
    STATFS,
    STATFS64,

    // read() syscalls
    READ = 10,
    READV,
    READAHEAD,
    READDIR,
    READLINK,
    READLINK_AT,

    // write() syscalls
    WRITE = 20,
    WRITEV,

    // open() and other syscalls which may create files
    OPEN = 30,
    OPEN_AT,
    CREAT,
    MKDIR,
    MKDIR_AT,
    NAME_TO_HANDLE_AT,
    OPEN_BY_HANDLE_AT,

    // rename() syscalls
    RENAME = 40,
    RENAME_AT,
    RENAME_AT2,

    // close() and unlink() syscalls
    CLOSE = 50,
    UNLINK,
    UNLINK_AT,
    RMDIR,

    // sync() syscalls
    FSYNC = 60,
    FDATASYNC,
    SYNC,
    SYNCFS,
    SYNC_FILE_RANGE,

    // other syscalls
    FCNTL = 70,
    GETDENTS,
    LSEEK,

    // mmap syscalls
    MMAP2 = 80,
    MUNMAP,
    REMAP,
    MSYNC,

    // chmod() syscalls
    CHMOD = 100,
    FCHMOD,
    FCHMODAT,

    // chown() syscalls
    CHOWN = 110,
    CHOWN16,
    LCHOWN,
    LCOWN16,
    FCHOWN,
    FCHOWN16,
    FCHOWNAT,

    // Meta operations (I/O replay internal use only)
    // A single thread terminates
    META_EXIT = 900,
    // All threads of a process termiate (process termination)
    META_EXIT_GROUP,
    // Meta operation for lamport synchronisation (currently unused)
    META_TIMELINE

} opcode_e;

#endif // OPCODES_H
