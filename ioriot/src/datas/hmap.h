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

#ifndef HMAP_H
#define HMAP_H

#include "../defaults.h"
#include "list.h"

/**
 * @brief A hash map data structure
 *
 * There are two version of this hmap data structure. One version is utilising
 * string keys and the other one is utilising long keys.
 *
 * On hash collision the data structure will make use of a "named" linked list,
 * whereas every member of the linked list has either a string key or a long
 * key associated.
 */
typedef struct hmap_s_ {
    char **keys; /**< List of all keys, NULL if nothing at a address */
    int *keys_l; /**< Same as keys, but for long keys */
    void **data; /**< Pointers to the stored data, NULL if nothing there */
    list_s **l; /**< Pointers to the linked lists, used on hash collision */
    void (*data_destroy)(void *data); /**< Callback to destroy all data */
    unsigned int size; /**< Size of the hmap */
} hmap_s;

hmap_s* hmap_new(unsigned int init_size);
hmap_s* hmap_new_l(unsigned int init_size);
void hmap_destroy(hmap_s* h);
void hmap_run_cb(hmap_s* h, void (*cb)(void *data));
void hmap_run_cb2(hmap_s* h, void (*cb)(void *data, void *data2), void *data_);
int hmap_insert_l(hmap_s* h, const long key, void *data);
int hmap_insert(hmap_s* h, char* key, void *data);
void* hmap_remove_l(hmap_s* h, const long key);
void* hmap_remove(hmap_s* h, char* key);

/**
 * @brief Removes and destroys all elements where key includes a substring
 *
 * @param h The hmap.
 * @param substr The substring.
 *
 * @return Count of elements destroyed/removed.
 **/
unsigned int hmap_keys_destroy(hmap_s* h, char* substr);

void* hmap_replace(hmap_s* h, char* key, void *data);
void* hmap_get_l(hmap_s* h, const long key);
void* hmap_get(hmap_s* h, char* key);
bool hmap_has(hmap_s* h, char* key);
unsigned int hmap_get_addr_l(hmap_s* h, const long key);
unsigned int hmap_get_addr(hmap_s* h, char* key);
void hmap_print(hmap_s* h);
void hmap_test(void);

#endif // HMAP_H
