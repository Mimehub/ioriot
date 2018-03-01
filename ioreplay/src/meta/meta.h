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

#ifndef META_H
#define META_H

#include "../defaults.h"

/**
 * @brief The meta information definition
 *
 * This is used to write or read meta information to/from the header
 * of the .replay file. This information then is used by ioreplay
 * in other steps. E.g. reading the amount of used file descriptors
 * from the meta header in order to allocate data structures of the
 * correct sizes before running the test!
 */
typedef struct meta_s_ {
    FILE* replay_fd; /**< The FS of the .replay file */
    off_t offset; /**< The meta offset (usually 0) */
    char* read_buf; /**< Pointer to a read buffer */
} meta_s;

/**
 * @brief Creates a new meta bject
 *
 * @return The new meta object
 */
meta_s* meta_new();

/**
 * @brief Destroys a meta object
 *
 * @param m The meta object
 */
void meta_destroy(meta_s *m);

/**
 * @brief Reserves space in the .replay file for the meta header
 *
 * @param m The meta object
 */
void meta_reserve(meta_s *m);

/**
 * @brief Indicates that we start writing the meta header to the .replay file
 *
 * @param m The meta object
 */
void meta_write_start(meta_s *m);

/**
 * @brief Writes a string to the meta header
 *
 * @param m The meta object
 * @param key The key
 * @param val The string value
 */
void meta_write_s(meta_s *m, char *key, char *val);

/**
 * @brief Writes a long to the meta header
 *
 * @param m The meta object
 * @param key The key
 * @param val The long value
 */
void meta_write_l(meta_s *m, char *key, long val);

/**
 * @brief indicates that we start reading from the meta header
 *
 * @param m The meta object
 */
void meta_read_start(meta_s *m);

/**
 * @brief Reads a string from the meta header
 *
 * @param m The meta object
 * @param key The key
 * @param val The string val read
 */
bool meta_read_s(meta_s *m, char *key, char **val);

/**
 * @brief Reads a long from the meta header
 *
 * @param m The meta object
 * @param key The key
 * @param val The long val read
 */
bool meta_read_l(meta_s *m, char *key, long *val);

#endif // META_H

