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

#ifndef BTREE_H
#define BTREE_H

#include "../defaults.h"

/**
 * @brief This defines an element of the binary tree data structure
 */
typedef struct btreelem_ {
    struct btreelem_ *left; /**< The next element to the left */
    struct btreelem_ *right; /**< The next element to the right */
    long key; /**< The key of the element */
    void *data; /**< A pointer to the data stored in this element */
} btreelem_s;

/**
 * @brief This defines a binary tree data structure.
 */
typedef struct btree_s_ {
    btreelem_s *root; /**< The root element */
    int size; /**< The current size of the binary tree */
} btree_s;

btree_s* btree_new();
void btree_destroy(btree_s *b);
void btree_destroy2(btree_s *b);
int btree_insert(btree_s *b, long key, void *data);
void* btree_get(btree_s *b, long key);
long btree_get_l(btree_s *b, long key);
bool btree_has_range_l(btree_s *b, const long start, const long end);
void btree_ensure_range_l(btree_s *b, const long start, const long end, const long threshold);
void btree_print(btree_s *b);
void btree_run_cb2(btree_s* b, void (*cb)(long key, void *data, void *data2), void *data2);
void btree_test(void);

btreelem_s* btreelem_new(long key, void *data);
void btreelem_destroy_r(btreelem_s *e);
void btreelem_destroy_r2(btreelem_s *e);
int btreelem_insert_r(btreelem_s *e, long key, void *data);
void* btreelem_get_r(btreelem_s *e, long key);
bool btreelem_has_range_lr(btreelem_s *e, const long start, const long end);
int btreelem_ensure_range_lr(btreelem_s *e, const long start, const long end, const long threshold);
void btreelem_print_r(btreelem_s *e, int depth);
void btreelem_run_cb2_r(btreelem_s* e, void (*cb)(long key, void *data, void *data2), void *data2);

#endif // BTREE_H
