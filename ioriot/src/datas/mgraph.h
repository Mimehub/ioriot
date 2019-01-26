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

#ifndef MGRAPH_H
#define MGRAPH_H

#include "../defaults.h"
#include "../tpool/tpool.h"

#include "mmap.h"
#include "hmap.h"

#define MGRAPH_DEP_LEN 5
#define MGRAPH_NUM_MUTEXES 16384

/**
 * @brief Definition of a graph mmap header
 *
 * This is to hold some meta information in the mmapped
 * graph structure.
 */
typedef struct mgraph_header_s_ {
    int version; /**< The serialisation version */
    unsigned long next_node_id; /**< To serialise the next node id */
} mgraph_header_s;

/**
 * @brief Definition of a graph node dependency list
 *
 * This data structure holds either the previous or the next
 * graph nodes.
 */
typedef struct mgraph_dep_s_ {
    unsigned long id; /**< The id of the dependency */
    unsigned long deps[MGRAPH_DEP_LEN]; /**< The dependency list */
    unsigned long next_dep_id; /**< If list is too short, continue here */
    int num_deps; /**< How many slots of deps are occupied? */
} mgraph_dep_s;

/**
 * @brief Definition of a graph node
 */
typedef struct mgraph_node_s_ {
    unsigned long id; /**< The id of the graph node */
    bool traversed; /**< Determines if node has been traversed already */
    char *path; /** < The path of the graph node */
    unsigned long prev_id; /**< The previous node id */
    unsigned long prev_dep_id; /**< If more than one prev, id of dependency */
    unsigned long next_id; /**< The next node id */
    unsigned long next_dep_id; /**< If more than one next, id of dependency */
    void *data; /**< Pointer to the stored data */
} mgraph_node_s;

/**
 * @brief A union needed for data serialisation
 *
 * Used to weite arrays of these data types to a file backed mmapped
 * array.
 */
typedef union mgraph_node_u_ {
    mgraph_header_s header;
    mgraph_node_s node;
    mgraph_dep_s dep;
} mgraph_node_u;

/**
 * @brief Definition of a graph data structure
 */
typedef struct mgraph_s_ {
    mmap_s *mmap; /**< The memory map object */
    mgraph_node_u *nodes; /**< All the graph nodes */
    mgraph_node_s *root; /**< The root nodeent */
    hmap_s *paths; /**< The paths of the graph */
    pthread_mutex_t mutexes[MGRAPH_NUM_MUTEXES]; /**< To sync access to the graph nodes */
    unsigned long next_node_id; /**< To get the next node id */
} mgraph_s;

/**
 * @brief Definition of a graph traverser
 */
typedef struct mgraph_traverser_s_ {
    mgraph_s *graph; /**< The graph to be traversed */
    tpool_s *pool; /**< The thread pool to traverse the graph */
    void (*callback)(mgraph_node_s *node, unsigned long depth); /**< Callback to run on all nodes */
} mgraph_traverser_s;

mgraph_dep_s *mgraph_dep_new(mgraph_s *g);
mgraph_dep_s *mgraph_dep_new2(mgraph_s *g, unsigned long add_id);
void mgraph_dep_add(mgraph_s *g, unsigned long dep_id, unsigned long id_add);
void mgraph_dep_print(mgraph_s *g, unsigned long dep_id);

mgraph_node_s *mgraph_node_new(mgraph_s *g, void *data, char *key);
void mgraph_node_init(mgraph_node_s *e, void *data, char *key, unsigned long id);
void mgraph_node_append(mgraph_s *g, mgraph_node_s *e, mgraph_node_s *e2);
void mgraph_node_print(mgraph_s *g, mgraph_node_s *e);

mgraph_s *mgraph_new(char *name, unsigned int init_size);
mgraph_s *mgraph_open(char *name, unsigned int init_size);
void mgraph_destroy(mgraph_s* g);
void mgraph_unlink(mgraph_s* g);
void mgraph_insert(mgraph_s* g, char *path, void *data);
void* mgraph_get(mgraph_s* g, char *path);
void mgraph_print(mgraph_s* g);
void mgraph_test();

mgraph_traverser_s *mgraph_traverser_new(mgraph_s *graph,
        void (*callback)(mgraph_node_s *node, unsigned long depth), int max_threads);
void mgraph_traverser_destroy(mgraph_traverser_s* t);
void mgraph_traverser_traverse(mgraph_traverser_s* t);

#endif // MGRAPH_H
