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

#include <libgen.h>

#include "graph.h"

graph_elem_s *graph_elem_new(void *data, char *path)
{
    static unsigned int id = 0;
    graph_elem_s *e = Malloc(graph_elem_s);

    e->id = id++;
    e->path = Clone(path);
    e->prev = e->next = NULL;
    e->num_prev = e->num_next = 0;
    e->data = data;
    pthread_mutex_init(&e->mutex, NULL);

    return e;
}

void graph_elem_destroy(graph_elem_s *e, void(*data_destroy)(void *data))
{
    if (e->prev != NULL) {
        free(e->prev);
    }

    if (e->next != NULL) {
        for (int i = 0; i < e->num_next; ++i)
            graph_elem_destroy(e->next[i], data_destroy);
        free(e->next);
    }

    data_destroy(e->data);
    pthread_mutex_destroy(&e->mutex);
    free(e->path);
    free(e);

    return;
}

void graph_elem_append(graph_elem_s *e, graph_elem_s *e2)
{
    pthread_mutex_lock(&e->mutex);
    if (e->next == NULL) {
        e->num_next = 1;
        e->next = Malloc(graph_elem_s*);
    } else {
        e->num_next++;
        e->next = Realloc(e->next, e->num_next, graph_elem_s*);
    }
    e->next[e->num_next-1] = e2;
    pthread_mutex_unlock(&e->mutex);

    pthread_mutex_lock(&e2->mutex);
    if (e2->prev == NULL) {
        e2->num_prev = 1;
        e2->prev = Malloc(graph_elem_s*);
    } else {
        e2->num_prev++;
        e2->prev = Realloc(e2->prev, e2->num_prev, graph_elem_s*);
    }
    e2->prev[e2->num_prev-1] = e;
    pthread_mutex_unlock(&e2->mutex);
}

static void _graph_elem_print(graph_elem_s *e, int ident)
{
    pthread_mutex_lock(&e->mutex);
    for (int i = 0; i < ident; ++i)
        printf(" ");
    Put("graph_elem:%p id:%u path:%s", (void*)e, e->id, e->path);
    for (int i = 0; i < e->num_next; ++i)
        _graph_elem_print(e->next[i], ident+1);
    pthread_mutex_unlock(&e->mutex);
}

void graph_elem_print(graph_elem_s *e)
{
    _graph_elem_print(e, 0);
}

graph_s *graph_new(unsigned int init_size, void(*data_destroy)(void *data))
{
    graph_s *g = Malloc(graph_s);

    g->data_destroy = NULL;
    g->paths = hmap_new(init_size);
    g->root = graph_elem_new(NULL, "/");
    g->data_destroy = data_destroy;
    hmap_insert(g->paths, "/", g->root);
    pthread_mutex_init(&g->mutex, NULL);

    return g;
}

void graph_destroy(graph_s *g)
{
    graph_elem_destroy(g->root, g->data_destroy);
    hmap_destroy(g->paths);
    pthread_mutex_destroy(&g->mutex);
    free(g);

    return;
}

static graph_elem_s* _graph_get_parent(graph_s *g, char *path)
{
    char *clone = Clone(path);
    char *parent_path = dirname(clone);
    graph_elem_s *parent_elem = hmap_get(g->paths, parent_path);

    if (parent_elem == NULL) {
        parent_elem = graph_elem_new(NULL, parent_path);
        hmap_insert(g->paths, parent_path, parent_elem);
        graph_elem_s *grandparent_elem = _graph_get_parent(g, parent_path);
        graph_elem_append(grandparent_elem, parent_elem);
    }

    free(clone);
    return parent_elem;
}

void graph_insert(graph_s *g, char *path, void *data)
{
    if (data == NULL) {
        Error("insert data can not be NULL");
    }

    graph_elem_s *elem = graph_elem_new(data, path);
    pthread_mutex_lock(&g->mutex);

    if (g->root == NULL) {
        g->root = elem;
        hmap_insert(g->paths, path, elem);
        goto cleanup;
    }

    graph_elem_s *elem_ = hmap_get(g->paths, path);

    if (elem_ != NULL) {
        graph_elem_append(elem_, elem);
        hmap_replace(g->paths, path, elem);

    } else {
        graph_elem_s *parent = _graph_get_parent(g, path);
        graph_elem_append(parent, elem);
        hmap_insert(g->paths, path, elem);
    }

cleanup:
    pthread_mutex_unlock(&g->mutex);
}

void* graph_get(graph_s *g, char *path)
{
    void *data = NULL;

    pthread_mutex_lock(&g->mutex);
    graph_elem_s *elem = hmap_get(g->paths, path);
    if (elem != NULL)
        data = elem->data;
    pthread_mutex_unlock(&g->mutex);

    return data;
}

void graph_print(graph_s *g)
{
    Put("graph:%p", (void*)g);
    graph_elem_print(g->root);
}

static void _graph_test_data_destroy(void *data)
{
    Put("Destroying %ld", (long)data)
}

void graph_test(void)
{
    graph_s* g = graph_new(1024, _graph_test_data_destroy);

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
    graph_destroy(g);
}
