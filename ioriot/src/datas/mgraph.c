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
#include <sys/types.h>

#include "mgraph.h"

#define _Header(g) &g->nodes[0].header
#define _Node(g,id) &g->nodes[id].node
#define _Dep(g,id) &g->nodes[id].dep

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
    e->path = path;
    e->prev_id = e->next_id = 0;
    e->prev_dep_id = e->next_dep_id = 0;
    e->data = data;
}

mgraph_node_s *mgraph_node_new(mgraph_s *g, void *data, char *key)
{
    unsigned long id = g->next_node_id++;
    mgraph_node_s *e = &g->nodes[id].node;
    mgraph_node_init(e, data, key, id);
    return e;
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

static void _mgraph_node_print_single(mgraph_node_s *e, int indent)
{
    for (int i = 0; i < indent; ++i)
        printf(" ");

    Put("mgraph_node:%p id:%ld path:%s pthread:%p",
        (void*)e, e->id, e->path, (void*)pthread_self());
}

static void _mgraph_node_print_indent(mgraph_s *g, mgraph_node_s *e, int indent)
{
    _mgraph_node_print_single(e, indent);
    if (e->next_id == 0)
        return;

    // Print direct next node
    _mgraph_node_print_indent(g, _Node(g, e->next_id), indent+1);

    if (e->next_dep_id == 0)
        return;

    // We have more next nodes, but they are stored in a dep object
   mgraph_dep_s *d = _Dep(g, e->next_dep_id);

   // Go through all dependency objects iteratively
   while (d->num_deps > 0) {
       for (int i = 0; i < d->num_deps; ++i)
           _mgraph_node_print_indent(g, _Node(g, d->deps[i]), indent+1);
       if (d->next_dep_id == 0)
           break;
       d = _Dep(g, d->next_dep_id);
   }
}

void mgraph_node_print(mgraph_s *g, mgraph_node_s *e)
{
    _mgraph_node_print_indent(g, e, 0);
}

static mgraph_s *_mgraph_init(unsigned int init_size, mmap_s *mmap, bool open_new)
{
    mgraph_s *g = Malloc(mgraph_s);

    g->mmap = mmap;
    g->nodes = g->mmap->memory;
    if (init_size == 0)
        g->paths = NULL;
    else
        g->paths = hmap_new(init_size);
    for (int i = 0; i < MGRAPH_NUM_MUTEXES; ++i)
        pthread_mutex_init(&g->mutexes[i], NULL);

    return g;
}

mgraph_s *mgraph_new(char *name, unsigned int init_size)
{
    mmap_s *mmap = mmap_new(name, MAX_MMAP_SIZE);
    mgraph_s *g = _mgraph_init(init_size, mmap, true);
    g->next_node_id = 1; // Root node starts at id 1
    g->root = mgraph_node_new(g, NULL, "/");
    hmap_insert(g->paths, "/", g->root);

    return g;
}

mgraph_s *mgraph_open(char *name, unsigned int init_size)
{
    mmap_s *mmap = mmap_open(name);
    mgraph_s *g =  _mgraph_init(init_size, mmap, false);

    mgraph_header_s *header = _Header(g);
    if (header->version != GRAPH_MMAP_VERSION) {
        Error("Can not load '%s', incompatible header version '%d', expected '%d'",
              name, header->version, GRAPH_MMAP_VERSION);
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

    for (int i = 0; i < MGRAPH_NUM_MUTEXES; ++i)
        pthread_mutex_destroy(&g->mutexes[i]);

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

    if (g->root == NULL) {
        g->root = node;
        hmap_insert(g->paths, path, node);
        return;
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
}

void* mgraph_get(mgraph_s *g, char *path)
{
    void *data = NULL;

    mgraph_node_s *node = hmap_get(g->paths, path);
    if (node != NULL)
        data = node->data;

    return data;
}

void mgraph_print(mgraph_s *g)
{
    Put("graph:%p", (void*)g);
    mgraph_node_print(g, g->root);
}

static bool _mgraph_traverser_node_traversed(mgraph_s *g, unsigned long id, int mutex_id)
{
    mgraph_node_s *e = _Node(g, id);
    int mutex_id2 = e->id % MGRAPH_NUM_MUTEXES;
    if (mutex_id == mutex_id2)
        // Same mutex, already locked!
        return e->traversed;

    pthread_mutex_t *mutex = &g->mutexes[mutex_id2];
    pthread_mutex_lock(mutex);
    bool traversed = e->traversed;
    pthread_mutex_unlock(mutex);
    return traversed;
}

static void _mgraph_traverser_traverse(void *data, void *data2, void *data3)
{
    mgraph_traverser_s *t = data;
    mgraph_node_s *e = data2;
    unsigned long depth = (unsigned long)data3;
    mgraph_s *g = t->graph;

    int mutex_id = e->id % MGRAPH_NUM_MUTEXES;
    pthread_mutex_t *mutex = &g->mutexes[mutex_id];
    pthread_mutex_lock(mutex);

    // Check if current node is traversed already
    if (e->traversed)
        goto cleanup;

    // Check if previous node is traversed already
    if (e->prev_id) {
        if (!_mgraph_traverser_node_traversed(g, e->prev_id, mutex_id))
            goto cleanup;

        // In case there are multiple previous nodes go
        // through all dependency lists and check if they
        // are traversed already or not.
        if (e->prev_dep_id) {
            mgraph_dep_s *dep = _Dep(g, e->prev_dep_id);

            while (dep->num_deps) {
                for (int i = 0; i < dep->num_deps; ++i)
                    if (!_mgraph_traverser_node_traversed(g, dep->deps[i], mutex_id))
                        goto cleanup;
                if (dep->next_dep_id == 0)
                    break;
                dep = _Dep(g, dep->next_dep_id);
            }
        }
    }

    // Deal with current node
    t->callback(e, depth);
    e->traversed = true;
    pthread_mutex_unlock(mutex);

    // Tell thread pool to traverse next node
    if (e->next_id) {
        tpool_add_work3(t->pool, t, _Node(g, e->next_id), (void*)(depth+1));

        // In case there are multiple next nodes go
        // through all dependency lists and add them
        // as work to the thread pool.
        if (e->next_dep_id) {
            mgraph_dep_s *dep = _Dep(g, e->next_dep_id);
            while (dep->num_deps) {
                for (int i = 0; i < dep->num_deps; ++i)
                    tpool_add_work3(t->pool, t,
                                    _Node(g, dep->deps[i]), (void*)(depth+1));
                if (dep->next_dep_id == 0)
                    break;
                dep = _Dep(g, dep->next_dep_id);
            }
        }
    }

    return;

cleanup:
    pthread_mutex_unlock(mutex);
}

mgraph_traverser_s *mgraph_traverser_new(mgraph_s *graph,
        void (*callback)(mgraph_node_s *node, unsigned long depth), int max_threads)
{
    mgraph_traverser_s *t = Malloc(mgraph_traverser_s);
    t->graph = graph;
    t->pool = tpool_new(max_threads, _mgraph_traverser_traverse);
    t->callback = callback;
    return t;
}

void mgraph_traverser_destroy(mgraph_traverser_s* t)
{
    tpool_destroy(t->pool);
    free(t);
}

void mgraph_traverser_traverse(mgraph_traverser_s* t)
{
    if (t->graph->root != NULL)
        tpool_add_work3(t->pool, t, t->graph->root, (void*)0);
}

pthread_mutex_t _test_mutex;

static void _mgraph_test_traverse_callback(mgraph_node_s *e, unsigned long depth)
{
    pthread_mutex_lock(&_test_mutex);
    _mgraph_node_print_single(e, depth);
    pthread_mutex_unlock(&_test_mutex);
}

void mgraph_test(void)
{
    pthread_mutex_init(&_test_mutex, NULL);

    Put("MGraph sizes: header=%ld dep:%ld node:%ld union:%ld",
        sizeof(mgraph_header_s), sizeof(mgraph_dep_s),
        sizeof(mgraph_node_s), sizeof(mgraph_node_u));

    Put("Creating 'mgraph_test'");
    mgraph_s *g = mgraph_new("mgraph_test", 1024);

    mgraph_insert(g, "/foo/bar", (void*)23);
    assert(23 == (long) mgraph_get(g, "/foo/bar"));
    mgraph_insert(g, "/foo/bar", (void*)23);
    mgraph_insert(g, "/foo/bar/baz", (void*)42);
    assert(42 == (long) mgraph_get(g, "/foo/bar/baz"));

    long value = 100;
    mgraph_insert(g, "/bla", (void*)value++);
    mgraph_insert(g, "/blu/foo/blu", (void*)value++);
    mgraph_insert(g, "/helloblu", (void*)value++);

    mgraph_print(g);
    mgraph_destroy(g);

    Put("Loading 'mgraph_test' from file");
    g = mgraph_open("mgraph_test", 1024);
    mgraph_print(g);

    Put("Traversing 'mgraph_test' using multiple threads");
    mgraph_traverser_s *t = mgraph_traverser_new(g, _mgraph_test_traverse_callback, 5);
    mgraph_traverser_traverse(t);
    mgraph_traverser_destroy(t);

    mgraph_unlink(g);
    mgraph_destroy(g);
    pthread_mutex_destroy(&_test_mutex);
}
