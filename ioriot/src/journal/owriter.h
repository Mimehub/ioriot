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

#ifndef OWRITER_H
#define OWRITER_H

#define Owriter_write(o, ...) \
    long Offset = ftell(o->fd); \
    fprintf(o->fd, ##__VA_ARGS__);

/**
 * @brief The offset writer object definition
 *
 * This is a offset journal writer. Whenever something is written to a journal
 * the byte offset and size of that journal entry will be returned.
 */
typedef struct owriter_s_ {
    char *journal_path; /**< The journal file path */
    FILE *fd; /**< The journal file descriptor */
} owriter_s;

/**
 * @brief Creates a new owriter object
 *
 * @param journal_path the journal path to write to
 * @return The new owriter object
 */
owriter_s* owriter_new(char *journal_path);

/**
 * @brief Creates a new owriter object from an already given fd
 *
 * @param fd The file descriptor
 * @return The new owriter object
 */
owriter_s* owriter_new_from_fd(FILE *fd);

/**
 * @brief Destroys a owriter object
 *
 * @param o The owriter object to destroy
 */
void owriter_destroy(owriter_s* o);

/**
 * @brief Write a line to the journal
 *
 * @param g The owriter object to write to
 * @param line The line to be written to the journal
 * @return The byte offset where the line starts in the journal
 */
off_t owriter_write(owriter_s* o, const char *line);

#endif
