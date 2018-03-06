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

#ifndef VFD_H
#define VFD_H

#include "opcodes.h"
#include "defaults.h"

/**
 * @brief The virtual file descriptor definition
 *
 * A virtual file descriptor represents a file descriptor from ioriot's
 * point of view and is being used in various ways to simulate the real I/O
 * protocolled to the .capture/.replay files.
 *
 * Generally speaking I/O replay maps the real FD numbers (the ones logged to
 * the .capture file) to virtual FD numbers (a uniqe FD number for every open
 * to increase concurrency).
 */
typedef struct vfd_s_ {
    int fd; /**< the real fd */
    DIR *dirfd; /**< The real dirfd */
    long mapped_fd; /**< The mapped fd (virtual fd) */
    char *path; /**< The file path belonging to that fd */
    bool free_path; /**< True if path has to be freed or not */
    unsigned long offset; /**< The current virtual file offset in bytes */
    int debug; /**< Used for debugging purposes only */
} vfd_s;

/**
 * @brief Creates a new virtual file descriptor object
 *
 * @param fd The file descriptor
 * @param mapped_fd The mapped file descriptor
 * @param path The path name
 * @return The new fd object
 */
vfd_s* vfd_new(const int fd, const long mapped_fd, char *path);

/**
 * @brief Updates the virtfd object
 *
 * @param vfd The virtfd object
 * @param fd The (real) file descriptor
 * @param mapped_fd The mapped (virtual) file descriptor
 * @param path The path name
 * @return The new fd object
 */
void vfd_update(vfd_s *vfd, const int fd, const long mapped_fd, char *path);

/**
 * @brief Destroys a file descriptor object
 *
 * @param vfd The file descriptor object
 */
void vfd_destroy(vfd_s *vfd);

/**
 * @brief Prints the virtual file descriptor
 * @param vfd The virtual file descriptor
 */
void vfd_print(vfd_s *vfd);

#endif // VFD_H

