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

#include "list.h"

list_s *list_new()
{
    list_s *l = Malloc(list_s);
    *l = (list_s) {
        .first = NULL, .data_destroy = NULL
    };
    return l;
}

list_s *list_new_l()
{
    return list_new();
}

void list_destroy(list_s *l)
{
    list_elem_s *current = l->first;

    while (current) {
        if (current->key)
            free(current->key);
        if (current->data && l->data_destroy)
            l->data_destroy(current->data);
        list_elem_s *next = current->next;
        free(current);
        current = next;
    }

    free(l);
}

int list_key_insert(list_s *l, char *key, void *data)
{
    list_elem_s *current = l->first;

    while (current) {
        // Already in the list
        if (strcmp(current->key, key) == 0)
            return 0;
        current = current->next;
    }

    list_elem_s *e = Malloc(list_elem_s);

    e->prev = NULL;
    e->next = l->first;
    e->key = Clone(key);
    e->key_l = -1;
    e->data = data;

    if (l->first) {
        l->first->prev = e;
        l->first = e;

    } else {
        l->first = e;
    }

    return 1;
}

int list_key_insert_l(list_s *l, const long key, void *data)
{
    list_elem_s *current = l->first;

    while (current) {
        if (current->key_l == key)
            return 0;
        current = current->next;
    }

    list_elem_s *e = Malloc(list_elem_s);

    e->prev = NULL;
    e->next = l->first;
    e->key = NULL;
    e->key_l = key;
    e->data = data;

    if (l->first) {
        l->first->prev = e;
        l->first = e;

    } else {
        l->first = e;
    }

    return 1;
}

void _list_elem_remove(list_s *l, list_elem_s *e)
{
    if (l->first == e) {
        list_elem_s *first = e->next;
        if (first)
            first->prev = NULL;
        l->first = first;

    } else {
        list_elem_s *prev = e->prev;
        list_elem_s *next = e->next;

        prev->next = next;
        if (next)
            next->prev = prev;
    }

    if (e->key)
        free(e->key);
    free(e);
}

void* list_key_remove(list_s *l, char *key)
{
    list_elem_s *current = l->first;

    while (current) {
        if (strcmp(current->key, key) == 0) {
            void *data = current->data;
            _list_elem_remove(l, current);
            return data;
        }
        current = current->next;
    }

    return NULL;
}

void* list_key_remove_l(list_s *l, const long key)
{
    list_elem_s *current = l->first;

    while (current) {
        if (current->key_l == key) {
            void *data = current->data;
            _list_elem_remove(l, current);
            return data;
        }
        current = current->next;
    }

    return NULL;
}

void* list_key_get(list_s *l, char *key)
{
    list_elem_s *current = l->first;

    while (current) {
        if (strcmp(current->key, key) == 0)
            return current->data;
        current = current->next;
    }

    return NULL;
}

void* list_key_get_l(list_s *l, const long key)
{
    list_elem_s *current = l->first;

    while (current) {
        if (current->key_l == key)
            return current->data;
        current = current->next;
    }

    return NULL;
}

void list_run_cb(list_s* l, void (*cb)(void *data))
{
    list_elem_s *current = l->first;

    while (current) {
        if (current->data)
            cb(current->data);
        current = current->next;
    }
}

void list_run_cb2(list_s* l, void (*cb)(void *data, void *data2), void *data_)
{
    list_elem_s *current = l->first;

    while (current) {
        if (current->data)
            cb(current->data, data_);
        current = current->next;
    }
}

void list_print(list_s *l)
{
    list_elem_s *current = l->first;

    while (current) {
        if (current->key != NULL) {
            Put("list:%p key:'%s' data:%p", (void*)l,
                current->key, current->data);
        } else {
            Put("list:%p key:%ld data:%p", (void*)l,
                current->key_l, current->data);
        }
        current = current->next;
    }
}

void list_test(void)
{
    list_s *l = list_new();
    void* somedata = (void*)l;

    assert(1 == list_key_insert(l, "foo", (void*)1));
    assert(1 == list_key_insert(l, "bar", (void*)2));
    assert(1 == list_key_insert(l, "baz", (void*)3));
    assert(2 == (long)list_key_remove(l, "bar"));
    assert(1 == (long)list_key_remove(l, "foo"));
    assert(3 == (long)list_key_remove(l, "baz"));

    assert(1 == list_key_insert(l, "I/O replay", somedata));
    assert(1 == list_key_insert(l, "for", somedata));
    assert(1 == list_key_insert(l, "benchmarking your server", somedata));
    assert(0 == list_key_insert(l, "for", somedata));

    assert(NULL != list_key_get(l, "benchmarking your server"));
    assert(NULL == list_key_get(l, "Mimecast"));

    assert(NULL != list_key_remove(l, "benchmarking your server"));
    assert(NULL == list_key_remove(l, "benchmarking your server"));
    assert(1 == list_key_insert(l, "benchmarking your server", somedata));

    assert(1 == list_key_insert(l, "MiMecast", (void*)42));
    assert(42 == (long)list_key_get(l, "MiMecast"));

    l = list_new_l();

    assert(1 == list_key_insert_l(l, 1, (void*)1));
    assert(1 == list_key_insert_l(l, 2, (void*)2));
    assert(1 == list_key_insert_l(l, 3, (void*)3));
    assert(1 == (long)list_key_get_l(l, 1));
    assert(1 == (long)list_key_remove_l(l, 1));
    assert(1 != (long)list_key_remove_l(l, 1));
    assert(3 == (long)list_key_remove_l(l, 3));

    assert(1 == list_key_insert_l(l, 1234, somedata));
    assert(1 == list_key_insert_l(l, 13, somedata));
    assert(1 == list_key_insert_l(l, 666, somedata));
    assert(0 == list_key_insert_l(l, 13, somedata));

    assert(NULL != list_key_get_l(l, 666));
    assert(NULL == list_key_get_l(l, 777));

    assert(NULL != list_key_remove_l(l, 666));
    assert(NULL == list_key_remove_l(l, 666));
    assert(1 == list_key_insert_l(l, 666, somedata));

    assert(1 == list_key_insert_l(l, 42, (void*)42));
    assert(42 == (long)list_key_get_l(l, 42));

    //list_print(l);
}
