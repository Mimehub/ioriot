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

#define _Header(g) &g->nodes[0].header
#define _Node(g, id) &g->nodes[id].node
#define _Dep(g, id) &g->nodes[id].dep

mgraph_dep_s *mgraph_dep_new(mgraph_s *g)
{
    unsigned long id = g->next_node_id++;
    mgraph_dep_s *d = &g->nodes[id].dep;

    d->id = id;
    d->next_dep_id = d->num_deps = 0;
    memset(d->deps, 0, sizeof(unsigned long)*MGRAPH_DEP_LEN);

    return d;
}

mgraph_dep_s *mgraph_dep_new2(mgraph_s *g, unsigned long id_add)
{
    mgraph_dep_s *d = mgraph_dep_new(g);
    d->deps[0] = id_add;
    d->num_deps = 1;

    return d;
}

void mgraph_dep_add(mgraph_s *g, unsigned long dep_id, unsigned long id_add)
{
    mgraph_dep_s *d = _Dep(g, dep_id);

    // Find the first dependency node with a free index
    while (d->num_deps == MGRAPH_DEP_LEN) {
        if (d->next_dep_id == 0) {
            // All indices used, but next dep list is empty
            mgraph_dep_s *d2 = mgraph_dep_new(g);
            d->next_dep_id = d2->id;
            d = d2;
            break;
        }

        // All indices used, iterate to next dep list
        d = _Dep(g, d->next_dep_id);
    }

    // There are free indexes
    d->deps[d->num_deps] = id_add;
    d->num_deps++;
}

void mgraph_dep_print(mgraph_s *g, unsigned long dep_id)
{
    mgraph_dep_s *d = _Dep(g, dep_id);
    Out("mgraph_dep:%p id:%ld num_deps:%d ", (void*)d, d->id, d->num_deps);
    Out(" deps:");
    for (int i = 0; i < d->num_deps; ++i) {
        Out("%ld ", d->deps[i]);
    }
    Put("next_dep_id:%ld", d->next_dep_id);
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
        mgraph_dep_s *d = mgraph_dep_new2(g, e2->id);
        e->next_dep_id = d->id;

    } else {
        mgraph_dep_add(g, e->next_dep_id, e2->id);
    }

    if (e2->prev_id == 0) {
        e2->prev_id = e->id;

    } else if (e2->prev_dep_id == 0) {
        mgraph_dep_s *d = mgraph_dep_new2(g, e->id);
        e2->prev_dep_id = d->id;

    } else {
        mgraph_dep_add(g, e2->prev_dep_id, e->id);
    }
}

static void _mgraph_node_print_single(mgraph_node_s *e, int ident)
{
    for (int i = 0; i < ident; ++i)
        printf(" ");
    Put("mgraph_node:%p id:%ld path:%s", (void*)e, e->id, e->path);
}

static void _mgraph_node_print(mgraph_s *g, mgraph_node_s *e, int ident)
{
    _mgraph_node_print_single(e, ident);
    if (e->next_id == 0)
        return;

    // Print direct next node
    _mgraph_node_print(g, _Node(g, e->next_id), ident+1);

   if (e->next_dep_id == 0)
       return;

    // We have more next nodes, but they are stored in a dep object
   mgraph_dep_s *d = _Dep(g, e->next_dep_id);

   // Go through all dependency objects iteratively
   while (d->num_deps > 0) {
       for (int i = 0; i < d->num_deps; ++i)
           _mgraph_node_print(g, _Node(g, d->deps[i]), ident+1);
       if (d->next_dep_id == 0)
           break;
       d = _Dep(g, d->next_dep_id);
   }
}

void mgraph_node_print(mgraph_s *g, mgraph_node_s *e)
{
    _mgraph_node_print(g, e, 0);
}


static mgraph_s *_mgraph_init(unsigned int init_size, void(*data_destroy)(void *data),
        mmap_s *mmap, bool open_new)
{
    mgraph_s *g = Malloc(mgraph_s);

    g->mmap = mmap;
    g->nodes = g->mmap->memory;
    if (init_size == 0)
        g->paths = NULL;
    else
        g->paths = hmap_new(init_size);
    g->data_destroy = data_destroy;
    pthread_mutex_init(&g->mutex, NULL);

    return g;
}

mgraph_s *mgraph_new(char *name, unsigned int init_size, void(*data_destroy)(void *data))
{
    mmap_s *mmap = mmap_new(name, MAX_MMAP_SIZE);
    mgraph_s *g = _mgraph_init(init_size, data_destroy, mmap, true);

    g->next_node_id = 1; // Root node starts at id 1
    g->root = mgraph_node_new(g, NULL, "/");
    hmap_insert(g->paths, "/", g->root);

    return g;
}

mgraph_s *mgraph_open(char *name, unsigned int init_size, void(*data_destroy)(void *data))
{
    mmap_s *mmap = mmap_open(name);
    mgraph_s *g =  _mgraph_init(init_size, data_destroy, mmap, false);

    mgraph_header_s *header = _Header(g);
    if (header->version != GRAPH_MMAP_VERSION) {
        Error("Can not load '%s', incompatible header version '%d', expected '%d'",
              header->version, GRAPH_MMAP_VERSION);
    }
    g->next_node_id = header->next_node_id;
    g->root = _Node(g, 1);

    return g;
}

void mgraph_destroy(mgraph_s *g)
{
    // Before destroying, set some header information which can be
    // retrieved after opening the mgraph from file later on again.
    mgraph_header_s *header = _Header(g);
    header->version = GRAPH_MMAP_VERSION;
    header->next_node_id = g->next_node_id;

    if (g->paths)
        hmap_destroy(g->paths);
    pthread_mutex_destroy(&g->mutex);
    mmap_destroy(g->mmap);
    free(g);

    return;
}

void mgraph_unlink(mgraph_s *g)
{
    mmap_unlink(g->mmap);
    return;
}

static mgraph_node_s* _mgraph_get_parent(mgraph_s *g, char *path)
{
    char *clone = Clone(path);
    char *parent_path = dirname(clone);
    //Debug("_mgraph_get_parent %s (parent_path: %s)", clone, parent_path);
    mgraph_node_s *parent_node = hmap_get(g->paths, parent_path);

    if (parent_node == NULL) {
        parent_node = mgraph_node_new(g, NULL, parent_path);
        hmap_insert(g->paths, parent_path, parent_node);
        mgraph_node_s *grandparent_node = _mgraph_get_parent(g, parent_path);
        mgraph_node_append(g, grandparent_node, parent_node);
    }

    //Debug("_mgraph_get_parent %s (parent_path: %s %s %p)", clone, parent_path, parent_node->path, parent_node);

    free(clone);
    return parent_node;
}

void mgraph_insert(mgraph_s *g, char *path, void *data)
{
    //Debug("mgraph_insert %s", path);
    if (data == NULL) {
        Error("insert data can not be NULL");
    }

    mgraph_node_s *node = mgraph_node_new(g, data, path);
    pthread_mutex_lock(&g->mutex);

    if (g->root == NULL) {
        //Debug("mgraph_insert A %s", path);
        g->root = node;
        hmap_insert(g->paths, path, node);
        goto cleanup;
    }

    mgraph_node_s *node_ = hmap_get(g->paths, path);

    if (node_ != NULL) {
        //Debug("mgraph_insert B %s", path);
        mgraph_node_append(g, node_, node);
        hmap_replace(g->paths, path, node);

    } else {
        //Debug("mgraph_insert C %s", path);
        mgraph_node_s *parent = _mgraph_get_parent(g, path);
        //Debug("mgraph_insert C %s (parent:%s %p)", path, parent->path, (void*)parent);
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
    mgraph_node_print(g, g->root);
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
    Put("FOO");
    _mgraph_node_print_single(node, depth);
}

void mgraph_test(void) 
{
    Put("Creating 'mgraph_test'");
    mgraph_s *g = mgraph_new("mgraph_test", 1024, _mgraph_test_data_destroy);

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
    mgraph_destroy(g);

    Put("Loading 'mgraph_test' from file");
    g = mgraph_open("mgraph_test", 1024, _mgraph_test_data_destroy);
    // These two don't work as we don't serialise hmap
    //assert(23 == (long) mgraph_get(g, "/foo/bar"));
    //assert(42 == (long) mgraph_get(g, "/foo/bar/baz"));
    mgraph_print(g);

    Put("Traversing 'mgraph_test'");

    mgraph_traverser_s *t = mgraph_traverser_new(_mgraph_test_traverse, 5);
    mgraph_traverser_traverse(t, g);
    mgraph_traverser_destroy(t);

    mgraph_unlink(g);
    mgraph_destroy(g);

    exit(0);


}
