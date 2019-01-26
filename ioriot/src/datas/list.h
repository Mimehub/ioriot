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

#ifndef LIST_H
#define LIST_H

#include "../defaults.h"

/**
 * @brief Definition of a linked list element
 */
typedef struct list_elem_s_ {
    struct list_elem_s_ *prev; /**< The previous element */
    struct list_elem_s_ *next; /**< The next element */
    char *key; /**< The key of the lemenet */
    long key_l; /**< The same as key, but for long keys */
    void *data; /**< Pointer to the stored data */
} list_elem_s;

/**
 * @brief Definition of a named linked list data structure
 *
 * There are two version of this list data structure. One version is utilising
 * string keys and the other one is utilising long keys.
 */
typedef struct list_s_ {
    list_elem_s *first; /**< The first element, NULL if list empty */
    void (*data_destroy)(void *data); /**< Callback to destroy all data */
} list_s;

list_s* list_new();
list_s* list_new_l();
void list_destroy(list_s* l);
void list_run_cb(list_s* l, void (*cb)(void *data));
void list_run_cb2(list_s* l, void (*cb)(void *data, void *data2), void *data_);
int list_key_insert(list_s* l, char *key, void *data);
int list_key_insert_l(list_s* l, const long key, void *data);
void* list_key_remove(list_s* l, char *key);
void* list_key_remove_l(list_s* l, const long key);
void* list_key_get(list_s* l, char *key);
void* list_key_get_l(list_s* l, const long key);
void list_print(list_s* l);
void list_test();

#endif // LIST_H
