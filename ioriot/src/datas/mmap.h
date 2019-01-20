// Copyright 2019 Mimecast Ltd.
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

#ifndef MMAP_H
#define MMAP_H

#include "../defaults.h"

/**
 * @brief Definition of a mmap
 */
typedef struct mmap_s_ {
  char *file; /**< The file name */
  int fd; /**< The fd of the mapped file */
  int size; /**< The size of the file */
  void *memory; /**< The mapped memory */
} mmap_s;

/**
 * @brief Creates a file and mmaps it for rw access
 * @param name The name (file name will be name.mmap)
 * @param size The size of the file in bytes
 * @return The map object
 */
mmap_s* mmap_new(char *name, int size);

/**
 * @brief Opens an already mmapped file
 * @param name The name (file name will be name.mmap)
 * @return The map object
 */
mmap_s* mmap_open(char *name);

/**
 * @brief Destroys a mapped object
 * @param m The map object
 */
void mmap_destroy(mmap_s *m);

/**
 * @brief Delete file from file system
 * @param m The map object
 */
void mmap_unlink(mmap_s *m);

/**
 * @brief Unit tests.
 */
void mmap_test();

#endif // MMAP_H
