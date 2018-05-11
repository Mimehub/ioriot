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

#ifndef GRAPH_H
#define GRAPH_H

#include "../defaults.h"
#include "hmap.h"

/**
 * @brief Definition of a graph element
 */
typedef struct graph_elem_s_ {
    unsigned int id; /**< The id of the graph element */
    char *path; /** < The path of the graph element */
    struct graph_elem_s_ **prev; /**< The previous elements */
    struct graph_elem_s_ **next; /**< The next elements */
    int num_prev; /**< Amount of prev elems */
    int num_next; /**< Amount of next elems */
    void *data; /**< Pointer to the stored data */
    pthread_mutex_t mutex; /**< To sync access to this graph element */

} graph_elem_s;

/**
 * @brief Definition of a graph data structure
 */
typedef struct graph_s_ {
    graph_elem_s *root; /**< The root element */
    void (*data_destroy)(void *data); /**< Callback to destroy all data */
    hmap_s *paths; /**< The paths of the graph */
    pthread_mutex_t mutex; /**< To sync access to this graph element */
} graph_s;

graph_elem_s *graph_elem_new(void *data, char *key);
void graph_elem_destroy(graph_elem_s *e, void(*data_destroy)(void *data));
void graph_elem_append(graph_elem_s *e, graph_elem_s *e2);
void graph_elem_print(graph_elem_s *e);

graph_s *graph_new(unsigned int init_size, void(*data_destroy)(void *data));
void graph_destroy(graph_s* g);
void graph_insert(graph_s* g, char *path, void *data);
void* graph_get(graph_s* g, char *path);
void graph_print(graph_s* g);
void graph_test();

#endif // GRAPH_H
