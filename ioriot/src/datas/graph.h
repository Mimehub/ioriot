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
#include "../tpool/tpool.h"

#include "mmap.h"
#include "hmap.h"

#define GRAPH_DEP_LEN 10

/**
 * @brief Definition of a graph mmap header 
 *
 * This is to hold some meta information in the mmapped
 * graph structure.
 */
typedef struct graph_header_s_ {
    int version; /**< The serialisation version */
} graph_header_s;

/**
 * @brief Definition of a graph node dependency list
 *
 * This data structure holds either the previous or the next
 * graph nodes.
 */
typedef struct graph_node_dep_s_ {
    unsigned long id; /**< The id of the dependency */
    unsigned long deps[GRAPH_DEP_LEN]; /**< The dependency list */
    unsigned long next_id; /**< If list is too short, continue here */
} graph_node_dep_s;

/**
 * @brief Definition of a graph node
 */
typedef struct graph_node_s_ {
    unsigned long id; /**< The id of the graph node */
    bool traversed; /**< Determines if node has been traversed already */
    char *path; /** < The path of the graph node */
    struct graph_node_s_ **prev; /**< The previous nodes */
    struct graph_node_s_ **next; /**< The next nodes */
    int num_prev; /**< Count of prev nodes */
    int num_next; /**< Count of next nodes */
    unsigned long prev_id; /**< The previous node id */
    unsigned long prev_dep_id; /**< If more than one prev, id of dependency */
    unsigned long next_id; /**< The next node id */
    unsigned long next_dep_id; /**< If more than one next, id of dependency */
    void *data; /**< Pointer to the stored data */
    pthread_mutex_t mutex; /**< To sync access to this graph node */
} graph_node_s;

/**
 * @brief A union needed for data serialisation
 *
 * Used to weite arrays of these data types to a file backed mmapped
 * array.
 */
typedef union graph_node_u_ {
    graph_header_s header;
    graph_node_s node;
    graph_node_dep_s dep;
} graph_node_u;

/**
 * @brief Definition of a graph data structure
 */
typedef struct graph_s_ {
    mmap_s *mmap; /**< The memory map object */
    graph_node_u *nodes; /**< All the graph nodes */
    graph_node_s *root; /**< The root nodeent */
    void (*data_destroy)(void *data); /**< Callback to destroy all data */
    hmap_s *paths; /**< The paths of the graph */
    pthread_mutex_t mutex; /**< To sync access to this graph nodeent */
    unsigned long next_node_id; /**< To get the next node id */
} graph_s;

/**
 * @brief Definition of a graph traverser
 */
typedef struct graph_traverser_s_ {
    tpool_s *pool; /**< The thread pool to traverse the graph */
    void (*callback)(graph_node_s *node, unsigned long depth); /**< Callback to run on all nodes */
} graph_traverser_s;


graph_node_s *graph_node_new(void *data, char *key, unsigned long id);
graph_node_s *graph_node_new_mmap(graph_s* g, void *data, char *key);
void graph_node_init(graph_node_s *e, void *data, char *key, unsigned long id);
void graph_node_destroy(graph_node_s *e, void(*data_destroy)(void *data));
void graph_node_append(graph_node_s *e, graph_node_s *e2);
void graph_node_print(graph_node_s *e);

graph_s *graph_new(char *name, unsigned int init_size, void(*data_destroy)(void *data));
void graph_destroy(graph_s* g);
void graph_insert(graph_s* g, char *path, void *data);
void* graph_get(graph_s* g, char *path);
void graph_print(graph_s* g);
void graph_test();

graph_traverser_s *graph_traverser_new(void (*callback)(graph_node_s *node, unsigned long depth), int max_threads);
void graph_traverser_destroy(graph_traverser_s* t);
void graph_traverser_traverse(graph_traverser_s* t, graph_s *g);


#endif // GRAPH_H
