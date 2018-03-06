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

#include "rbuffer.h"

rbuffer_s *rbuffer_new(const int size)
{
    rbuffer_s *r = Malloc(rbuffer_s);

    r->size = size;
    r->read_pos = size-1;
    r->write_pos = 0;
    r->ring = Calloc(size, void*);

    Mset(r->ring, 0, size, void*);

    return r;
}

void rbuffer_destroy(rbuffer_s *r)
{
    if (r) {
        free(r->ring);
        free(r);
    }
}

bool rbuffer_insert(rbuffer_s* r, void *data)
{
    if (r->write_pos == r->read_pos)
        // Ring buffer is full
        return false;

    r->ring[r->write_pos] = data;
    r->write_pos = (r->write_pos+1) % r->size;

    return true;
}

bool rbuffer_has_next(rbuffer_s* r)
{
    sig_atomic_t read_pos = (r->read_pos+1) % r->size;

    if (read_pos == r->write_pos)
        // No more items to read, buffer is empty
    {
        return false;
    }

    return true;
}

void* rbuffer_get_next(rbuffer_s* r)
{
    sig_atomic_t read_pos = (r->read_pos+1) % r->size;

    if (read_pos == r->write_pos)
        // No more items to read, buffer is empty
    {
        return NULL;
    }

    void *data = r->ring[read_pos];
    r->ring[read_pos] = NULL;
    r->read_pos = read_pos;

    return data;
}

void rbuffer_print(rbuffer_s* r)
{
    Put("rbuffer_s (%p):", (void*)r);
    Put("\tsize: %d", (int)r->size);
    Put("\tread_pos: %d", r->read_pos);
    Put("\twrite_pos: %d", r->write_pos);
    Out("\toccupied slots: ");
    for (int i = 0; i < r->size; ++i)
        if (r->ring[i]) {
            Out("%d:%p ", i, r->ring[i]);
        }
    Out("\n");
}

void rbuffer_test(void)
{
    rbuffer_s *r = rbuffer_new(5);
    assert(NULL == rbuffer_get_next(r));

    assert(rbuffer_insert(r, (void*)1));
    assert(rbuffer_insert(r, (void*)2));
    assert(rbuffer_insert(r, (void*)3));
    assert(rbuffer_insert(r, (void*)4));
    assert(!rbuffer_insert(r, (void*)5));
    rbuffer_print(r);

    assert(rbuffer_has_next(r));
    assert(1 == (long) rbuffer_get_next(r));
    assert(2 == (long) rbuffer_get_next(r));
    assert(3 == (long) rbuffer_get_next(r));
    assert(4 == (long) rbuffer_get_next(r));
    assert(!rbuffer_has_next(r));
    assert(NULL == rbuffer_get_next(r));

    assert(rbuffer_insert(r, (void*)1));
    assert(1 == (long) rbuffer_get_next(r));
    assert(rbuffer_insert(r, (void*)2));
    assert(2 == (long) rbuffer_get_next(r));
    assert(rbuffer_insert(r, (void*)3));
    assert(3 == (long) rbuffer_get_next(r));
    assert(rbuffer_insert(r, (void*)4));
    assert(4 == (long) rbuffer_get_next(r));
    assert(rbuffer_insert(r, (void*)5));
    assert(5 == (long) rbuffer_get_next(r));
    assert(NULL == rbuffer_get_next(r));
    rbuffer_print(r);

    assert(rbuffer_insert(r, (void*)1));
    rbuffer_print(r);
    assert(rbuffer_insert(r, (void*)2));
    assert(1 == (long) rbuffer_get_next(r));
    rbuffer_print(r);
    assert(rbuffer_insert(r, (void*)3));
    assert(2 == (long) rbuffer_get_next(r));
    rbuffer_print(r);
    assert(rbuffer_insert(r, (void*)4));
    assert(3 == (long) rbuffer_get_next(r));
    rbuffer_print(r);
    assert(rbuffer_insert(r, (void*)5));
    rbuffer_print(r);
    assert(4 == (long) rbuffer_get_next(r));
    rbuffer_print(r);
    assert(5 == (long) rbuffer_get_next(r));
    assert(NULL == rbuffer_get_next(r));

    rbuffer_destroy(r);
}
