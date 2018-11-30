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

#ifndef GWRITER_H
#define GWRITER_H

#include "../datas/rbuffer.h"
#include "../defaults.h"
#include "../journal/owriter.h"
#include "gtask.h"
#include "generate.h"

// Forward declaration (header include hell)
struct gtask_s_;
struct generate_s_;

/**
 * @brief Definition of the writer object
 *
 * The writer utilises the information extracted by the parser to actually
 * write the .replay file.
 */
typedef struct gwriter_s_ {
    bool terminate; /**< The writer thread will terminate if set to true */
    struct generate_s_ *generate; /**< The generate object */
    pthread_t pthread; /**< The posix thread */
    rbuffer_s *queue; /**< A queue of task objects */
    owriter_s *owriter; /**< The journal offset writer */
} gwriter_s;

/**
 * @brief Creates a new writer
 *
 * @param g The generate object
 * @return The new writer object
 */
gwriter_s* gwriter_new(struct generate_s_ *g);

/**
 * @brief Starts the writer thread
 *
 * @param w The writer object
 */
void gwriter_start(gwriter_s *w);

/**
 * @brief Terminates the writer thread
 *
 * @param w The writer object
 */
void gwriter_terminate(gwriter_s *w);

/**
 * @brief Destroys the writer thread
 *
 * @param w The writer object
 */
void gwriter_destroy(gwriter_s *w);

/**
 * @brief Writes a line to the .replay file
 *
 * @param w The writer object
 * @param t The task object
 */
void gwriter_write(gwriter_s *w, struct gtask_s_ *t);

/**
 * @brief Entry function of the writer pthread
 *
 * @param data A pointer to the writer object
 * @return Always returns a NULL pointer if it doesnt crash!
 */
void* gwriter_pthread_start(void *data);

#endif // GWRITER_H
