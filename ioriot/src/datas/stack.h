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

#ifndef STACK_H
#define STACK_H

#include "../defaults.h"

/**
 * @brief Definition of a stack element
 */
typedef struct stack_elem_s_ {
    struct stack_elem_s_ *next; /**< The next element */
    void *data; /**< Pointer to the stored data in the current element */
} stack_elem_s;

/**
 * @brief Definition of a stack data structure
 */
typedef struct stack_s_ {
    stack_elem_s *top; /**< The top element of the stack, NULL if empty */
    unsigned long size; /**< A count how many elements are in the stack */
} stack_s;

stack_s* stack_new();
stack_s* stack_new_reverse_from(stack_s* s);
void stack_destroy(stack_s* s);
void stack_push(stack_s* s, void *data);
void* stack_pop(stack_s* s);
int stack_is_empty(stack_s* s);

#endif // STACK_H
