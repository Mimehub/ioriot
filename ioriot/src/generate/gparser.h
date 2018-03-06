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

#ifndef GPARSER_H
#define GPARSER_H

#include "../datas/rbuffer.h"
#include "../defaults.h"
#include "generate.h"
#include "gtask.h"

/**
 * @brief The parser definition
 *
 * The parser is to extract all information from the .capture file.
 */
typedef struct gparser_s_ {
    bool terminate; /**< The parser thread will terminate if set to true */
    generate_s *generate; /**< The generate object */
    pthread_t pthread; /**< The posix thread */
    rbuffer_s *queue; /**< A queue of task objects */
} gparser_s;

/**
 * @brief Creates a new parser
 *
 * @param g The generate object
 * @return The new parser object
 */
gparser_s* gparser_new(generate_s *g);

/**
 * @brief Starts the parser thread
 *
 * @param p The parser object
 */
void gparser_start(gparser_s *p);

/**
 * @brief Terminates the parser thread
 *
 * @param p The parser object
 */
void gparser_terminate(gparser_s *p);

/**
 * @brief Destroys the parser thread
 *
 * @param p The parser object
 */
void gparser_destroy(gparser_s *p);

/**
 * @brief Extracts information a .capture line
 *
 * Extracts information from a .capture line and stores it into the task
 * object.
 *
 * @param p The parser object
 * @param t The task object
 */
void gparser_extract(gparser_s *p, gtask_s *t);

/**
 * @brief Extracts information from a specific token string
 *
 * @param p The parser object
 * @param t The task object
 * @param tok The token string
 * @return Returns with SUCCESS on success
 */
status_e gparser_extract_tok(gparser_s *p, gtask_s *t, char *tok);

/**
 * @brief Verifies the correctness of a token
 *
 * @param p The parser object
 * @param tok The token to be verified
 * @return true if token verified successfully
 */
bool gparser_token_not_ok(gparser_s *p, char *tok);

/**
 * @brief Checks whether the pidtid string is correct or not
 *
 * @param p The parser object
 * @param pidtid The string to check
 * @param pid The pointer to the resulting pid
 * @param tid The pointer to the resulting tid
 * @return true on success
 */
bool gparser_get_pidtid(gparser_s *p, char *pidtid, long *pid, long *tid);

/**
 * @brief Entry point of the parser POSIX thread
 *
 * @param data A pointer to the parser object
 * return Always NULL
 */
void* gparser_pthread_start(void *data);

#endif // GPARSER_H
