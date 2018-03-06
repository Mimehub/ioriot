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

#include "hmap.h"

#define _Using_string_keys h->keys != NULL

unsigned int hmap_get_addr(hmap_s *h, char *key)
{
    unsigned long hash = 5381;
    int len = strlen(key);

    for (int i = 0; i < len; ++i) {
        hash = ((hash << 5) + hash) + key[i];    /* hash * 33 + c */
    }

    return (unsigned int) (hash % h->size);
}

unsigned int hmap_get_addr_l(hmap_s *h, const long key)
{
    return (unsigned int) (key % h->size);
}

hmap_s *_hmap_new(unsigned int init_size)
{
    hmap_s *h = Malloc(hmap_s);

    h->size = init_size;
    h->data = Calloc(init_size, void*);
    h->l = Calloc(init_size, list_s*);
    h->data_destroy = NULL;
    h->keys = NULL;
    h->keys_l = NULL;

    Mset(h->data, 0, init_size, void*);
    Mset(h->l, 0, init_size, list_s*);

    return h;
}

hmap_s *hmap_new(unsigned int init_size)
{
    hmap_s *h = _hmap_new(init_size);
    h->keys = Calloc(init_size, char*);
    Mset(h->keys, 0, init_size, char*);

    return h;
}

hmap_s *hmap_new_l(unsigned int init_size)
{
    hmap_s *h = _hmap_new(init_size);
    h->keys_l = Calloc(init_size, int);
    Mset(h->keys_l, -1, init_size, int);

    return h;
}

void hmap_destroy(hmap_s *h)
{
    for (int i = 0; i < h->size; ++i) {
        if (h->l[i]) {
            list_s *l = h->l[i];
            if (h->data_destroy)
                l->data_destroy = h->data_destroy;
            list_destroy(h->l[i]);
        }
        if (h->data[i] && h->data_destroy) {
            h->data_destroy(h->data[i]);
        }
    }

    free(h->data);
    if (h->keys)
        free(h->keys);
    if (h->keys_l)
        free(h->keys_l);
    free(h->l);
    free(h);

    return;
}

int hmap_insert(hmap_s *h, char *key, void *data)
{
    if (data == NULL) {
        Error("insert data can not be NULL");
    }

    int addr = hmap_get_addr(h, key);

    if (h->data[addr]) {

        if (strcmp(key, h->keys[addr]) == 0) {
            // Key already exists
            return 0;
        }

        // There is already data, collision, create a linked list
        list_s *l = h->l[addr] = list_new();
        list_key_insert(l, h->keys[addr], h->data[addr]);
        list_key_insert(l, key, data);

        // Not needed anymore, as the elements are in the linked list now.
        free(h->keys[addr]);
        h->data[addr] = h->keys[addr] = NULL;

        return 1;

    } else if (h->l[addr]) {
        // There was a collision at this address before. Insert
        // the element to the linked list. Returns 0 if key is already
        // in the list (no additional insert made) or 1 otherwise.
        return list_key_insert(h->l[addr], key, data);
    }

    // New entry on a collision free address
    h->data[addr] = data;
    h->keys[addr] = Clone(key);

    return 1;
}

int hmap_insert_l(hmap_s *h, const long key, void *data)
{
    if (data == NULL) {
        Error("insert data can not be NULL");
    }

    int addr = hmap_get_addr_l(h, key);

    if (h->data[addr]) {

        if (key == h->keys_l[addr]) {
            // Key already exists
            return 0;
        }

        // There is already data, collision, create a linked list
        list_s *l = h->l[addr] = list_new_l();
        list_key_insert_l(l, h->keys_l[addr], h->data[addr]);
        list_key_insert_l(l, key, data);

        // Not needed anymore, as the elements are in the linked list now.
        h->data[addr] = NULL;
        h->keys_l[addr] = -1;

        return 1;

    } else if (h->l[addr]) {
        // There was a collision at this address before. Insert
        // the element to the linked list. Returns 0 if key is already
        // in the list (no additional insert made) or 1 otherwise.
        return list_key_insert_l(h->l[addr], key, data);
    }

    // New entry on a collision free address
    h->data[addr] = data;
    h->keys_l[addr] = key;

    return 1;
}

void* hmap_remove(hmap_s *h, char *key)
{
    int addr = hmap_get_addr(h, key);

    if (h->data[addr] != NULL) {
        void *data = h->data[addr];
        free(h->keys[addr]);
        h->data[addr] = h->keys[addr] = NULL;
        return data;

    } else if (h->l[addr] != NULL) {
        // There was a collision at this address before. Remove
        // the element to the linked list. Returns the object if key is
        // already in the list (no additional insert made) or NULL
        // otherwise.
        return list_key_remove(h->l[addr], key);
    }

    // Key is not present
    return NULL;
}

void* hmap_remove_l(hmap_s *h, const long key)
{
    int addr = hmap_get_addr_l(h, key);

    if (h->data[addr] != NULL) {
        void *data = h->data[addr];
        h->data[addr] = NULL;
        h->keys_l[addr] = -1;
        return data;

    } else if (h->l[addr] != NULL) {
        // There was a collision at this address before. Remove
        // the element to the linked list. Returns the object if key is
        // already in the list (no additional insert made) or NULL
        // otherwise.
        return list_key_remove_l(h->l[addr], key);
    }

    // Key is not present
    return NULL;
}

void* hmap_get(hmap_s *h, char *key)
{
    int addr = hmap_get_addr(h, key);
    if (h->data[addr] && strcmp(h->keys[addr], key) == 0) {
        return h->data[addr];

    } else if (h->l[addr]) {
        return list_key_get(h->l[addr], key);
    }

    return NULL;
}

void* hmap_get_l(hmap_s *h, const long key)
{
    int addr = hmap_get_addr_l(h, key);
    if (h->data[addr] && h->keys_l[addr] == key) {
        return h->data[addr];

    } else if (h->l[addr]) {
        return list_key_get_l(h->l[addr], key);
    }

    return NULL;
}

void hmap_run_cb(hmap_s* h, void (*cb)(void *data))
{
    for (int i = 0; i < h->size; ++i) {
        if (h->l[i]) {
            list_s *l = h->l[i];
            list_run_cb(l, cb);
        }
        if (h->data[i]) {
            cb(h->data[i]);
        }
    }
}

void hmap_run_cb2(hmap_s* h, void (*cb)(void *data, void *data2), void *data_)
{
    for (int i = 0; i < h->size; ++i) {
        if (h->l[i]) {
            list_s *l = h->l[i];
            list_run_cb2(l, cb, data_);
        }
        if (h->data[i]) {
            cb(h->data[i], data_);
        }
    }
}

void hmap_print(hmap_s *h)
{
    for (int i = 0; i < h->size; ++i) {
        if (h->data[i]) {
            if (_Using_string_keys) {
                Put("hmap:%p addr:%d key:'%s'", (void*)h, i, h->keys[i]);
            } else {
                Put("hmap:%p addr:%d key:%d", (void*)h, i, h->keys_l[i]);
            }
        } else if (h->l[i]) {
            Put("hmap:%p addr:%d LIST", (void*)h, i);
            list_print(h->l[i]);
        }
    }
}

static void _hmap_test(hmap_s *h)
{
    void* somedata = (void*)h;

    assert(1 == hmap_insert(h, "someval", (void*)23));
    assert(1 == hmap_insert(h, "another value", (void*)123));

    assert(1 == hmap_insert(h, "mimecast", somedata));
    assert(0 == hmap_insert(h, "mimecast", somedata));
    assert(1 == hmap_insert(h, "is", somedata));
    assert(1 == hmap_insert(h, "hiring", somedata));

    assert(NULL != hmap_get(h, "mimecast"));
    assert(NULL == hmap_get(h, "Mimecast"));

    assert(NULL != hmap_remove(h, "mimecast"));
    assert(NULL == hmap_remove(h, "mimecast"));

    assert(1 == hmap_insert(h, "mimecast", somedata));
    assert(NULL != hmap_get(h, "mimecast"));

    assert(23 == (long)hmap_get(h, "someval"));
    assert(23 == (long)hmap_get(h, "someval"));

    assert(123 == (long)hmap_remove(h, "another value"));
    assert(0 == (long)hmap_remove(h, "another value"));
    assert(NULL == hmap_get(h, "another value"));

    //hmap_print(h);
}

static void _hmap_test_l(hmap_s *h)
{
    void* somedata = (void*)h;

    assert(1 == hmap_insert_l(h, 1, (void*)23));
    assert(1 == hmap_insert_l(h, 5, (void*)123));

    assert(1 == hmap_insert_l(h, 3, somedata));
    assert(0 == hmap_insert_l(h, 3, somedata));
    assert(1 == hmap_insert_l(h, 4, somedata));
    assert(1 == hmap_insert_l(h, 6, somedata));

    assert(NULL != hmap_get_l(h, 3));
    assert(NULL == hmap_get_l(h, 7));

    assert(NULL != hmap_remove_l(h, 3));
    assert(NULL == hmap_remove_l(h, 3));

    assert(1 == hmap_insert_l(h, 3, somedata));
    assert(NULL != hmap_get_l(h, 3));

    assert(23 == (long)hmap_get_l(h, 1));
    assert(23 == (long)hmap_get_l(h, 1));

    assert(123 == (long)hmap_remove_l(h, 5));
    assert(0 == (long)hmap_remove_l(h, 5));
    assert(NULL == hmap_get_l(h, 5));
}

void hmap_test(void)
{
    hmap_s* h = hmap_new(1024);
    _hmap_test(h);
    hmap_destroy(h);

    h = hmap_new(2);
    _hmap_test(h);
    hmap_destroy(h);

    h = hmap_new_l(1024);
    _hmap_test_l(h);
    hmap_print(h);
    hmap_destroy(h);
}
