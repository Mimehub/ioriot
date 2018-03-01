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

#include "btree.h"

btree_s* btree_new()
{
    btree_s *b = Malloc(btree_s);
    *b = (btree_s) {
        .root = NULL, .size = 0
    };
    return b;
}

void btree_destroy(btree_s* b)
{
    if (b->root)
        btreelem_destroy_r(b->root);
    free(b);
}

void btree_destroy2(btree_s* b)
{
    if (b->root)
        btreelem_destroy_r2(b->root);
    free(b);
}

int btree_insert(btree_s* b, int key, void *data)
{
    int ret = 1;

    if (b->root == NULL) {
        b->root = btreelem_new(key, data);
        ret = 0;
    } else {
        ret = btreelem_insert_r(b->root, key, data);
    }

    if (ret == 0) {
        b->size++;
    }

    return ret;
}

void* btree_get(btree_s* b, int key)
{
    if (b->root == NULL)
        return NULL;

    return btreelem_get_r(b->root, key);
}

void btree_print(btree_s* b)
{
    btreelem_print_r(b->root, 0);
}

btreelem_s* btreelem_new(int key, void *data)
{
    btreelem_s *e = Malloc(btreelem_s);
    *e = (btreelem_s) {
        .key = key, .data = data, .left = NULL, .right = NULL
    };
    return e;
}

void btreelem_destroy_r(btreelem_s* e)
{
    if (e->left) {
        btreelem_destroy_r(e->left);
    }
    if (e->right) {
        btreelem_destroy_r(e->right);
    }

    free(e);
}

void btreelem_destroy_r2(btreelem_s* e)
{
    if (e->left)
        btreelem_destroy_r(e->left);
    if (e->right)
        btreelem_destroy_r(e->right);
    if (e->data)
        btree_destroy(e->data);

    free(e);
}

int btreelem_insert_r(btreelem_s* e, int key, void *data)
{
    int ret = 0;

    if (e->key == key) {
        ret = 1;
    }

    else if (e->key > key) {
        if (e->left == NULL) {
            e->left = btreelem_new(key, data);
        } else {
            ret = btreelem_insert_r(e->left, key, data);
        }
    }

    else {
        if (e->right == NULL) {
            e->right = btreelem_new(key, data);
        } else {
            ret = btreelem_insert_r(e->right, key, data);
        }
    }

    return ret;
}

void* btreelem_get_r(btreelem_s* e, int key)
{
    void *data = NULL;

    if (e->key == key) {
        data = e->data;
    }

    else if (e->key > key) {
        if (e->left) {
            data = btreelem_get_r(e->left, key);
        }
    }

    else {
        if (e->right) {
            data = btreelem_get_r(e->right, key);
        }
    }

    return data;
}

void btreelem_print_r(btreelem_s* e, int depth)
{
    for (int i = 0; i < depth; ++i) {
        Out(" ");
    }
    Put("%d\n", e->key);

    if (e->left) {
        btreelem_print_r(e->left, depth);
    }

    if (e->right) {
        btreelem_print_r(e->right, depth+1);
    }
}

