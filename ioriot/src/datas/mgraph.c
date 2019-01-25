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

#define _GNU_SOURCE
#include <stdio.h>
#include <libgen.h>

#include "mgraph.h"

mgraph_node_dep_s *mgraph_node_dep_new(mgraph_s *g, unsigned long id_add)
{
    unsigned long id = g->next_node_id++;
    mgraph_node_dep_s *d = &g->nodes[id].dep;

    d->id = id;
    d->deps[0] = id_add;
    d->num_deps = 1;

    for (int i = 1; i < MGRAPH_DEP_LEN; ++i)
        d->deps[i] = 0;

    return d;
}

void mgraph_node_dep_add(mgraph_s *g, unsigned long dep_id, unsigned long id_add)
{
    mgraph_node_dep_s *d = &g->nodes[dep_id].dep;

    // Find the first dependency node with a free index
    while (d->num_deps == MGRAPH_DEP_LEN) {
        // Get last element of deps, which points to the next mgraph_node_dep_s
        dep_id = d->deps[MGRAPH_DEP_LEN-1];
        d = &g->nodes[dep_id].dep;
    }

    if (d->num_deps == MGRAPH_DEP_LEN-1) {
        // All indexes but last one are used, it means we have to create a new dep object
        mgraph_node_dep_s *d2 = mgraph_node_dep_new(g, id_add);
        d->deps[d->num_deps] = d2->id;

    } else {
        // There are free indexes
        d->deps[d->num_deps] = id_add;
    }

    d->num_deps++;
}

void mgraph_node_init(mgraph_node_s *e, void *data, char *path, unsigned long id)
{
    e->id = id;
    e->traversed = false;
    e->path = Clone(path);
    e->prev_id = e->next_id = 0;
    e->prev_dep_id = e->next_dep_id = 0;
    e->data = data;
    pthread_mutex_init(&e->mutex, NULL);
}

mgraph_node_s *mgraph_node_new(mgraph_s *g, void *data, char *key)
{
    unsigned long id = g->next_node_id++;
    mgraph_node_s *e = &g->nodes[id].node;
    mgraph_node_init(e, data, key, id);
    return e;
}

bool mgraph_node_is_traversed(mgraph_node_s *e)
{
    pthread_mutex_lock(&e->mutex);
    bool traversed = e->traversed;
    pthread_mutex_unlock(&e->mutex);
    return traversed;
}

void mgraph_node_append(mgraph_s *g, mgraph_node_s *e, mgraph_node_s *e2)
{
    if (e->next_id == 0) {
        e->next_id = e2->id;

    } else if (e->next_dep_id == 0) {
        mgraph_node_dep_s *d = mgraph_node_dep_new(g, e2->id);
        e->next_dep_id = d->id;

    } else {
        mgraph_node_dep_add(g, e->next_dep_id, e2->id);
    }

    if (e2->prev_id == 0) {
        e2->prev_id = e->id;

    } else if (e2->prev_dep_id == 0) {
        mgraph_node_dep_s *d = mgraph_node_dep_new(g, e->id);
        e2->prev_dep_id = d->id;

    } else {
        mgraph_node_dep_add(g, e2->prev_dep_id, e->id);
    }
}

static void _mgraph_node_print_single(mgraph_node_s *e, unsigned long ident)
{
    for (unsigned long i = 0; i < ident; ++i)
        printf(" ");
    Put("mgraph_node:%p id:%ld path:%s", (void*)e, e->id, e->path);
}

/*
static void _mgraph_node_print(mgraph_s *g, mgraph_node_s *e, unsigned long ident)
{
    pthread_mutex_lock(&e->mutex);
    _mgraph_node_print_single(e, ident);
    if (e->next_id != 0)
        _mgraph_node_print_single(&g->nodes[e->next_id].node, ident+1);
    if (e->next_dep_id != 0) {
        mgraph_node_dep_s *d = &g->nodes[e->next_dep_id].dep;
    }
    pthread_mutex_unlock(&e->mutex);
}

void mgraph_node_print(mgraph_s *g, mgraph_node_s *e)
{
    _mgraph_node_print(g, e, 0);
}
*/

mgraph_s *mgraph_new(char *name, unsigned int init_size, void(*data_destroy)(void *data))
{
    mgraph_s *g = Malloc(mgraph_s);
    g->next_node_id = 1; // Root node starts at id 1

    // We want to mmap the graph
    g->mmap = mmap_new(name, MAX_MMAP_SIZE);
    g->nodes = g->mmap->memory;
    g->root = mgraph_node_new(g, NULL, "/");

    mgraph_header_s *header = &g->nodes[0].header;
    header->version = GRAPH_MMAP_VERSION;

    g->data_destroy = NULL;
    g->paths = hmap_new(init_size);
    g->data_destroy = data_destroy;
    hmap_insert(g->paths, "/", g->root);
    pthread_mutex_init(&g->mutex, NULL);

    return g;
}

void mgraph_destroy(mgraph_s *g)
{
    hmap_destroy(g->paths);
    pthread_mutex_destroy(&g->mutex);
    mmap_destroy(g->mmap);
    free(g);

    return;
}

static mgraph_node_s* _mgraph_get_parent(mgraph_s *g, char *path)
{
    char *clone = Clone(path);
    char *parent_path = dirname(clone);
    mgraph_node_s *parent_node = hmap_get(g->paths, parent_path);

    if (parent_node == NULL) {
        parent_node = mgraph_node_new(g, NULL, parent_path);
        hmap_insert(g->paths, parent_path, parent_node);
        mgraph_node_s *grandparent_node = _mgraph_get_parent(g, parent_path);
        mgraph_node_append(g, grandparent_node, parent_node);
    }

    free(clone);
    return parent_node;
}

void mgraph_insert(mgraph_s *g, char *path, void *data)
{
    if (data == NULL) {
        Error("insert data can not be NULL");
    }

    mgraph_node_s *node = mgraph_node_new(g, data, path);
    pthread_mutex_lock(&g->mutex);

    if (g->root == NULL) {
        g->root = node;
        hmap_insert(g->paths, path, node);
        goto cleanup;
    }

    mgraph_node_s *node_ = hmap_get(g->paths, path);

    if (node_ != NULL) {
        mgraph_node_append(g, node_, node);
        hmap_replace(g->paths, path, node);

    } else {
        mgraph_node_s *parent = _mgraph_get_parent(g, path);
        mgraph_node_append(g, parent, node);
        hmap_insert(g->paths, path, node);
    }

cleanup:
    pthread_mutex_unlock(&g->mutex);
}

void* mgraph_get(mgraph_s *g, char *path)
{
    void *data = NULL;

    //pthread_mutex_lock(&g->mutex);
    mgraph_node_s *node = hmap_get(g->paths, path);
    if (node != NULL)
        data = node->data;
    //pthread_mutex_unlock(&g->mutex);

    return data;
}

void mgraph_print(mgraph_s *g)
{
    Put("graph:%p", (void*)g);
    //mgraph_node_print(g, g->root);
}

static void _mgraph_traverser_traverse(void *data, void *data2, void *data3)
{
    mgraph_traverser_s *t = data;
    mgraph_node_s *e = data2;
    unsigned long depth = (long) data3;
    pthread_mutex_lock(&e->mutex);

    // Check if current node is traversed already
    if (e->traversed)
        goto cleanup;

    /*
    for (int i = 0; i < e->num_prev; ++i) {
        mgraph_node_s* prev = e->prev[i];
        pthread_mutex_lock(&prev->mutex);
        _Bool prev_traversed = prev->traversed;
        pthread_mutex_unlock(&prev->mutex);
        if (!prev_traversed)
            goto cleanup;
    }

    // Deal with current node
    t->callback(e, depth);
    e->traversed = true;
    pthread_mutex_unlock(&e->mutex);

    // Tell thread pool to traverse following nodes
    for (int i = 0; i < e->num_next; ++i)
        tpool_add_work3(t->pool, t, e->next[i], (void*)(depth+1));

    return;
        */

cleanup:
    pthread_mutex_unlock(&e->mutex);
}

mgraph_traverser_s *mgraph_traverser_new(void (*callback)(mgraph_node_s *node, unsigned long depth), int max_threads)
{
    mgraph_traverser_s *t = Malloc(mgraph_traverser_s);
    t->pool = tpool_new(max_threads, _mgraph_traverser_traverse);
    t->callback = callback;

    return t;
}

void mgraph_traverser_destroy(mgraph_traverser_s* t)
{
    tpool_destroy(t->pool);
    free(t);
}

void mgraph_traverser_traverse(mgraph_traverser_s* t, mgraph_s *g)
{
    pthread_mutex_lock(&g->mutex);
    if (g->root != NULL)
        tpool_add_work3(t->pool, t, g->root, (void*)0);
    pthread_mutex_unlock(&g->mutex);
}

static void _mgraph_test_data_destroy(void *data)
{
    Put("Destroying %ld", (long)data)
}

static void _mgraph_test_traverse(mgraph_node_s *node, unsigned long depth)
{
    _mgraph_node_print_single(node, depth);
}

void _mgraph_test(mgraph_s *g)
{
    mgraph_insert(g, "/foo/bar", (void*)23);
    assert(23 == (long) mgraph_get(g, "/foo/bar"));
    mgraph_insert(g, "/foo/bar", (void*)42);
    mgraph_insert(g, "/foo/bar/baz", (void*)42);
    assert(42 == (long) mgraph_get(g, "/foo/bar/baz"));

    long value = 100;
    mgraph_insert(g, "/bla", (void*)value++);
    mgraph_insert(g, "/", (void*)value++);
    mgraph_insert(g, "/blu/foo/blu", (void*)value++);

    mgraph_print(g);

    mgraph_traverser_s *t = mgraph_traverser_new(_mgraph_test_traverse, 5);
    mgraph_traverser_traverse(t, g);
    mgraph_traverser_destroy(t);
}

void mgraph_test(void) 
{
   mgraph_s *g = mgraph_new("mgraph_test", 1024, _mgraph_test_data_destroy);
    _mgraph_test(g);
   mgraph_destroy(g);
   exit(0);
}
