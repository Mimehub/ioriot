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

#ifndef VSIZE_H
#define VSIZE_H

#include "../utils/utils.h"
#include "../datas/hmap.h"
#include "../vfd.h"

/**
 * @brief Definition of a virtual size object
 *
 * The virtual size is used to determine the expected type and size of a file.
 * This piece of information will be added to the INIT section of the the
 * .replay file. That file then will be created during test initialisation.
 * before running the test. It is very likely the case that the test requires
 * a file of a certain size already to be present, so it can be read from disk.
 */
typedef struct vsize_s_ {
    char *path; /**< The path to the file/directory */
    off_t offset; /**< The current file offset */
    unsigned long id; /**< The vsize id */
    void *generate; /**< A pointer to the generate object */
    long vsize; /**< The virtual size */
    long vsize_deficit; /**< Size to use for file creating during init mode */
    bool renamed; /**< True if file/dir has been renamed */
    bool required; /**< True if init mode will create this file/dir */
    bool is_dir; /**< True if this file/dir is a directory */
    bool is_file; /**< True if this file/dir is a regular file */
    bool unsure; /**< True if the file type is not fully clear */
    long updates; /**< Amount of times this vsize has been updated */
    bool inserted; /**< For debugging purposes only */
} vsize_s;

/**
 * @brief Creates a new vsize object
 *
 * @param file_path The corresponding file path
 * @param id The vsize vsize aka ID
 * @param generate The generate object
 * @return The new vsize object
 */
vsize_s* vsize_new(char *file_path, const unsigned long id, void *generate);

/**
 * @brief Destroys a vsize object
 *
 * @param v The vsize object
 */
void vsize_destroy(vsize_s *v);

/**
 * @brief Ensures that the parent directory exists
 *
 * This function ensures that the parent directory exists as a vsize object!
 *
 * @param v The vsize object
 * @param path The given path
 */
void init_parent_dir(vsize_s *v, const char *path);

/**
 * @brief Adjusts the vsize
 *
 * Compares the virtual file size of the file in the vsize
 * object to the the offset in the virtual file descriptor.
 * In case the offset is higher we have a size deficit and
 * we need to mark it. That way ioreplay can ensure that
 * during init mode it will create a file with the correct
 * size prior of running the test!
 *
 * @param v The virtual size object
 * @param vfd The virtual file descriptor object
 */
void vsize_adjust(vsize_s *v, vfd_s* vfd);

/**
 * @brief Adjust vsize on open
 *
 * @param v The virtual size object
 * @param vfd The virtual file descriptor object
 * @param path The file open path
 * @param flags The file open flags
 */
void vsize_open(vsize_s *v, void *vfd, const char *path, const int flags);

/**
 * @brief Adjust vsize on close
 *
 * @param v The virtual size object
 * @param vfd The virtual file descriptor object
 */
void vsize_close(vsize_s *v, void *vfd);

/**
 * @brief Adjust vsize on stat
 *
 * @param v The virtual size object
 * @param path The stat path
 */
void vsize_stat(vsize_s *v, const char *path);

/**
 * @brief Adjust vsize on rename
 *
 * @param v The virtual size object
 * @param v2 The virtual size object of path2
 * @param path The first file path
 * @param path2 The second file path
 */
void vsize_rename(vsize_s *v, vsize_s *v2,
                  const char *path, const char *path2);

/**
 * @brief Adjust vsize on read
 *
 * @param v The virtual size object
 * @param vfd The virtual vile descriptor object
 * @param path The file path
 * @param bytes The amount of bytes read
 */
void vsize_read(vsize_s *v, void *vfd, const char *path, const int bytes);

/**
 * @brief Adjust vsize on seek
 *
 * @param v The virtual size object
 * @param vfd The virtual vile descriptor object
 * @param new_offset The new file offset after seek
 */
void vsize_seek(vsize_s *v, void *vfd, const long new_offset);

/**
 * @brief Adjust vsize on write
 *
 * @param v The virtual size object
 * @param vfd The virtual vile descriptor object
 * @param path The file path
 * @param bytes The amount of bytes written
 */
void vsize_write(vsize_s *v, void *vfd, const char *path, const int bytes);

/**
 * @brief Adjust vsize on mkdir
 *
 * @param v The virtual size object
 * @param path The directory path
 */
void vsize_mkdir(vsize_s *v, const char *path);

/**
 * @brief Adjust vsize on rmdir
 *
 * @param v The virtual size object
 * @param path The directory path
 */
void vsize_rmdir(vsize_s *v, const char *path);

/**
 * @brief Adjust vsize on unlink
 *
 * @param v The virtual size object
 * @param path The file path
 */
void vsize_unlink(vsize_s *v, const char *path);

#endif // VSIZE_H

