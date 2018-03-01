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

#include "stack.h"


stack_s *stack_new()
{
    stack_s *s = Malloc(stack_s);
    *s = (stack_s) {
        .top = NULL, .size = 0
    };
    return s;
}

void stack_destroy(stack_s *s)
{
    stack_elem_s *current = s->top;

    while (current) {
        stack_elem_s *next = current->next;
        free(current);
        current = next;
    }

    free(s);
}

void stack_push(stack_s *s, void *data)
{
    stack_elem_s *new_top = Malloc(stack_elem_s);

    *new_top = (stack_elem_s) {
        .next = s->top,
         .data = data
    };

    s->top = new_top;
    s->size++;
}

void* stack_pop(stack_s *s)
{
    if (s->top == NULL) {
        return NULL;
    }

    stack_elem_s *old_top = s->top;

    void *data = old_top->data;
    s->top = old_top->next;
    free(old_top);
    s->size--;

    return data;
}

int stack_is_empty(stack_s *s)
{
    return s->top == NULL;
}

stack_s* stack_new_reverse_from(stack_s *s)
{
    stack_s* r = stack_new();

    while (!stack_is_empty(s)) {
        stack_push(r, stack_pop(s));
    }

    stack_destroy(s);

    return r;
}
