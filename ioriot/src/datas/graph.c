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

#include "graph.h"

static void _graph_traverser_traverse(void *data, void *data2, void *data3)
{
    graph_traverser_s *t = data;
    graph_node_s *e = data2;
    unsigned long depth = (long) data3;
    pthread_mutex_lock(&e->mutex);

    // Check if current node is traversed already
    if (e->traversed)
        goto cleanup;

    for (int i = 0; i < e->num_prev; ++i) {
        graph_node_s* prev = e->prev[i];
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

cleanup:
    pthread_mutex_unlock(&e->mutex);
}

void graph_node_init(graph_node_s *e, void *data, char *path, unsigned long id)
{
    e->id = id;
    e->traversed = false;
    e->path = Clone(path);
    e->prev = e->next = NULL;
    e->num_prev = e->num_next = 0;
    e->prev_id = e->next_id = 0;
    e->prev_dep_id = e->next_dep_id = 0;
    e->data = data;
    pthread_mutex_init(&e->mutex, NULL);
}

graph_node_s *graph_node_new(void *data, char *path, unsigned long id)
{
    graph_node_s *e = Malloc(graph_node_s);
    graph_node_init(e, data, path, id);
    return e;
}

graph_node_s *graph_node_new_mmap(graph_s *g, void *data, char *path)
{
    unsigned long id = g->next_node_id++;
    graph_node_s *e = &g->nodes[id];
    graph_node_init(e, data, path, id);

    return e;
}

bool graph_node_is_traversed(graph_node_s *e)
{
    pthread_mutex_lock(&e->mutex);
    bool traversed = e->traversed;
    pthread_mutex_unlock(&e->mutex);
    return traversed;
}

void graph_node_destroy(graph_node_s *e, void(*data_destroy)(void *data))
{
    if (e->prev != NULL) {
        free(e->prev);
    }

    if (e->next != NULL) {
        for (int i = 0; i < e->num_next; ++i)
            graph_node_destroy(e->next[i], data_destroy);
        free(e->next);
    }

    data_destroy(e->data);
    pthread_mutex_destroy(&e->mutex);
    free(e->path);
    free(e);

    return;
}

void graph_node_append(graph_node_s *e, graph_node_s *e2)
{
    pthread_mutex_lock(&e->mutex);
    if (e->next == NULL) {
        e->num_next = 1;
        e->next = Malloc(graph_node_s*);
    } else {
        e->num_next++;
        e->next = Realloc(e->next, e->num_next, graph_node_s*);
    }
    e->next[e->num_next-1] = e2;
    pthread_mutex_unlock(&e->mutex);

    pthread_mutex_lock(&e2->mutex);
    if (e2->prev == NULL) {
        e2->num_prev = 1;
        e2->prev = Malloc(graph_node_s*);
    } else {
        e2->num_prev++;
        e2->prev = Realloc(e2->prev, e2->num_prev, graph_node_s*);
    }
    e2->prev[e2->num_prev-1] = e;
    pthread_mutex_unlock(&e2->mutex);
}

static void _graph_node_print_single(graph_node_s *e, unsigned long ident)
{
    for (unsigned long i = 0; i < ident; ++i)
        printf(" ");
    Put("graph_node:%p id:%ld path:%s", (void*)e, e->id, e->path);
}

static void _graph_node_print(graph_node_s *e, unsigned long ident)
{
    pthread_mutex_lock(&e->mutex);
    _graph_node_print_single(e, ident);
    for (int i = 0; i < e->num_next; ++i)
        _graph_node_print(e->next[i], ident+1);
    pthread_mutex_unlock(&e->mutex);
}

void graph_node_print(graph_node_s *e)
{
    _graph_node_print(e, 0);
}

graph_s *graph_new(char *name, unsigned int init_size, void(*data_destroy)(void *data))
{
    graph_s *g = Malloc(graph_s);
    g->next_node_id = 1; // Root node starts at id 1

    if (name) {
        // We want to mmap the graph
        g->map = mmap_new(name, MAX_MMAP_SIZE);
        g->nodes = g->map->memory;
        g->root = graph_node_new_mmap(g, NULL, "/");

    } else {
        // We don't want to mmap the graph
        g->map = NULL;
        g->root = graph_node_new(NULL, "/", g->next_node_id++);
    }

    g->data_destroy = NULL;
    g->paths = hmap_new(init_size);
    g->data_destroy = data_destroy;
    hmap_insert(g->paths, "/", g->root);
    pthread_mutex_init(&g->mutex, NULL);

    return g;
}

void graph_destroy(graph_s *g)
{
    hmap_destroy(g->paths);
    pthread_mutex_destroy(&g->mutex);

    if (g->map)
        mmap_destroy(g->map);
    else
        graph_node_destroy(g->root, g->data_destroy);

    free(g);

    return;
}

static graph_node_s* _graph_get_parent(graph_s *g, char *path)
{
    char *clone = Clone(path);
    char *parent_path = dirname(clone);
    graph_node_s *parent_node = hmap_get(g->paths, parent_path);

    if (parent_node == NULL) {
        if (g->map)
            parent_node = graph_node_new_mmap(g, NULL, parent_path);
        else
            parent_node = graph_node_new(NULL, parent_path, g->next_node_id++);

        hmap_insert(g->paths, parent_path, parent_node);
        graph_node_s *grandparent_node = _graph_get_parent(g, parent_path);
        graph_node_append(grandparent_node, parent_node);
    }

    free(clone);
    return parent_node;
}

void graph_insert(graph_s *g, char *path, void *data)
{
    if (data == NULL) {
        Error("insert data can not be NULL");
    }

    graph_node_s *node;
    if (g->map)
        node = graph_node_new_mmap(g, data, path);
    else
        node = graph_node_new(data, path, g->next_node_id++);
    pthread_mutex_lock(&g->mutex);

    if (g->root == NULL) {
        g->root = node;
        hmap_insert(g->paths, path, node);
        goto cleanup;
    }

    graph_node_s *node_ = hmap_get(g->paths, path);

    if (node_ != NULL) {
        graph_node_append(node_, node);
        hmap_replace(g->paths, path, node);

    } else {
        graph_node_s *parent = _graph_get_parent(g, path);
        graph_node_append(parent, node);
        hmap_insert(g->paths, path, node);
    }

cleanup:
    pthread_mutex_unlock(&g->mutex);
}

void* graph_get(graph_s *g, char *path)
{
    void *data = NULL;

    pthread_mutex_lock(&g->mutex);
    graph_node_s *node = hmap_get(g->paths, path);
    if (node != NULL)
        data = node->data;
    pthread_mutex_unlock(&g->mutex);

    return data;
}

void graph_print(graph_s *g)
{
    Put("graph:%p", (void*)g);
    graph_node_print(g->root);
}

graph_traverser_s *graph_traverser_new(void (*callback)(graph_node_s *node, unsigned long depth), int max_threads)
{
    graph_traverser_s *t = Malloc(graph_traverser_s);
    //tpool_add_work3(t->pool, _graph_traverser_traverse, t, e->next[i], (void*)(depth+1));
    t->pool = tpool_new(max_threads, _graph_traverser_traverse);
    t->callback = callback;
    return t;
}

void graph_traverser_destroy(graph_traverser_s* t)
{
    tpool_destroy(t->pool);
    free(t);
}

void graph_traverser_traverse(graph_traverser_s* t, graph_s *g)
{
    pthread_mutex_lock(&g->mutex);
    if (g->root != NULL)
        tpool_add_work3(t->pool, t, g->root, (void*)0);
    pthread_mutex_unlock(&g->mutex);
}

static void _graph_test_data_destroy(void *data)
{
    Put("Destroying %ld", (long)data)
}

static void _graph_test_traverse(graph_node_s *node, unsigned long depth)
{
    _graph_node_print_single(node, depth);
}

void _graph_test(graph_s *g)
{
    graph_insert(g, "/foo/bar", (void*)23);
    assert(23 == (long) graph_get(g, "/foo/bar"));
    graph_insert(g, "/foo/bar", (void*)42);
    graph_insert(g, "/foo/bar/baz", (void*)42);
    assert(42 == (long) graph_get(g, "/foo/bar/baz"));

    long value = 100;
    graph_insert(g, "/bla", (void*)value++);
    graph_insert(g, "/", (void*)value++);
    graph_insert(g, "/blu/foo/blu", (void*)value++);

    graph_print(g);

    graph_traverser_s *t = graph_traverser_new(_graph_test_traverse, 5);
    graph_traverser_traverse(t, g);
    graph_traverser_destroy(t);
}

void graph_test(void) 
{
    // Test without mmapping
    graph_s *g = graph_new(NULL, 1024, _graph_test_data_destroy);
    _graph_test(g);
    graph_destroy(g);

    // Test with mmapping
   g = graph_new("graph_test", 1024, _graph_test_data_destroy);
    _graph_test(g);
   graph_destroy(g);
   exit(0);
}
