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

#include "amap.h"

/**
 * @brief Creates a new array map
 *
 * @param size The array map size
 * @param mmapped true if the memory should be mmapped
 * @return The new amap object
 */
static amap_s *_amap_new(long size, bool mmapped)
{
    amap_s *a = NULL;
    void ***arrays = NULL;

    // Calculate a multiple of 1024, but at least in size of 'size'.
    if (size % 1024 != 0) {
        size = 1024*(1+(long)(size/1024));
    }

    if (size < 1) {
        Error("Size overflow");
    }

    int num_arrays = size / AMAP_MAX_ARRAY_LENGTH;

    if (mmapped) {
        a = Mmapshared(amap_s);
        arrays = Cmapshared(num_arrays, void**);
    } else {
        a = Malloc(amap_s);
        arrays = Calloc(num_arrays, void**);
    }

    for (int i = 0; i < num_arrays; ++i) {
        if (mmapped) {
            //Put("%d", AMAP_MAX_ARRAY_LENGTH);
            arrays[i] = Cmapshared(AMAP_MAX_ARRAY_LENGTH, void*);
        } else {
            arrays[i] = Calloc(AMAP_MAX_ARRAY_LENGTH, void*);
        }
        for (int j = 0; j < AMAP_MAX_ARRAY_LENGTH; ++j) {
            arrays[i][j] = NULL;
        }
    }

    a->arrays = arrays;
    a->num_arrays = num_arrays;
    a->size = size;
    a->data_destroy = NULL;
    a->mmapped = mmapped;

    return a;
}

/**
 * @brief Creates a new array map
 *
 * @param size The array map size
 * @return The new amap object
 */
amap_s* amap_new(const long size)
{
    return _amap_new(size, false);
}

/**
 * @brief Creates a new mmapped array map
 *
 * @param size The array map size
 * @return The new amap object
 */
amap_s* amap_new_mmapped(const long size)
{
    return _amap_new(size, true);
}

/**
 * @brief Destroys a mmap object
 *
 * @a The new amap object
 */
void amap_destroy(amap_s* a)
{
    if (!a) {
        return;
    }

    // Don't bother, the mmapped version of amap will stay alive until
    // process terminations. And after process termination everything
    // will be cleaned up automatically by Linux.
    if (a->mmapped) {
        return;
    }

    for (int i = 0; i < a->num_arrays; ++i) {
        if (a->data_destroy) {
            for (int j = 0; j < AMAP_MAX_ARRAY_LENGTH; ++j)
                if (a->arrays[i][j]) {
                    a->data_destroy(a->arrays[i][j]);
                }
        }
        free(a->arrays[i]);
    }
    free(a->arrays);
    free(a);
}

/**
 * @brief Resets a mmap object
 *
 * This resets all entries to NULL.
 *
 * @a The new amap object
 */
void amap_reset(amap_s* a)
{
    for (int i = 0; i < a->num_arrays; ++i) {
        for (int j = 0; j < AMAP_MAX_ARRAY_LENGTH; ++j) {
            if (a->data_destroy) {
                if (a->arrays[i][j]) {
                    a->data_destroy(a->arrays[i][j]);
                }
            }
            a->arrays[i][j] = NULL;
        }
    }
}

int amap_set(amap_s *a, const long position, void* value)
{
    if (position >= a->size)
        return -1;
    int which_array = position / AMAP_MAX_ARRAY_LENGTH;
    int array_pos = position % AMAP_MAX_ARRAY_LENGTH;
    a->arrays[which_array][array_pos] = value;
    return 0;
}

void* amap_get(amap_s *a, const long position)
{
    if (position >= a->size)
        return NULL;
    int which_array = position / AMAP_MAX_ARRAY_LENGTH;
    int array_pos = position % AMAP_MAX_ARRAY_LENGTH;
    return a->arrays[which_array][array_pos];
}

void* amap_unset(amap_s *a, const long position)
{
    if (position >= a->size)
        return NULL;
    int which_array = position / AMAP_MAX_ARRAY_LENGTH;
    int array_pos = position % AMAP_MAX_ARRAY_LENGTH;
    void *value = a->arrays[which_array][array_pos];
    a->arrays[which_array][array_pos] = NULL;
    return value;
}

void amap_run_cb(amap_s *a, void (*cb)(void *data))
{
    for (int i = 0; i < a->num_arrays; ++i) {
        for (int j = 0; j < AMAP_MAX_ARRAY_LENGTH; ++j) {
            if (a->arrays[i][j])
                cb(a->arrays[i][j]);
        }
    }
}

void amap_print(amap_s* a)
{
    Put("amap_s (%p):", (void*)a);
    Put("\tmmapped: %d", a->mmapped);
    Put("\tmax_array_length: %d", AMAP_MAX_ARRAY_LENGTH);
    Put("\tnum_arrays: %d", a->num_arrays);
    Put("\tsize: %lu", a->size);
    Out("\toccupied slots: ");
    for (int i = 0; i < a->num_arrays; ++i) {
        for (int j = 0; j < AMAP_MAX_ARRAY_LENGTH; ++j) {
            if (a->arrays[i][j] != NULL) {
                Out("%d:%d ", i, j);
            }
        }
    }
    Out("\n");
}

void _amap_test(amap_s *a)
{
    assert(0 == amap_set(a, 0, (void*)10));
    assert(0 == amap_set(a, 1, (void*)11));
    assert(0 == amap_set(a, 2, (void*)12));
    assert(0 == amap_set(a, 3, (void*)a));
    assert(10 == (long) amap_get(a, 0));
    assert(11 == (long) amap_get(a, 1));
    assert(12 == (long) amap_get(a, 2));
    assert(a == amap_get(a, 3));

    assert(0 == amap_set(a, AMAP_MAX_ARRAY_LENGTH-1, (void*) 23));
    assert(23 == (long) amap_get(a, AMAP_MAX_ARRAY_LENGTH-1));

    assert(0 == amap_set(a, AMAP_MAX_ARRAY_LENGTH, (void*) 42));
    assert(42 == (long) amap_get(a, AMAP_MAX_ARRAY_LENGTH));

    assert(0 == amap_set(a, AMAP_MAX_ARRAY_LENGTH*2-1, (void*) (23+42)));
    assert(42+23 == (long) amap_get(a, AMAP_MAX_ARRAY_LENGTH*2-1));
    assert(0 == amap_set(a, AMAP_MAX_ARRAY_LENGTH*2, (void*) 23));


    assert(NULL == amap_get(a, 1024*1024*9-1));
    assert(0 == amap_set(a, 1024*1024*9-1, (void*) 0x1));
    assert(0x1 == (long) amap_get(a, 1024*1024*9-1));
    assert(0x1 == (long) amap_unset(a, 1024*1024*9-1));
    assert(NULL == amap_get(a, 1024*1024*9-1));

    assert(0 == amap_set(a, 1024*1024*9, (void*) 100));
    assert(100 == (long) amap_get(a, 1024*1024*9));

    assert(0 == amap_set(a, 1024*1024*9+1, (void*) 101));
    assert(101 == (long) amap_get(a, 1024*1024*9+1));

    assert(0 == amap_set(a, 1024*1024*10-2, (void*) 102));
    assert(102 == (long) amap_get(a, 1024*1024*10-2));

    assert(0 == amap_set(a, 1024*1024*10-1, a));
    assert(a == amap_get(a, 1024*1024*10-1));
    //amap_print(a);

    assert(a == amap_unset(a, 1024*1024*10-1));
    assert(a != amap_unset(a, 1024*1024*10-1));
    //amap_print(a);
}

void amap_test(void)
{
    // First test the non-mmapped version
    amap_s* a = amap_new(1024*1024*10);
    _amap_test(a);
    amap_destroy(a);

    // Now test the mapped version
    a = amap_new_mmapped(1024*1024*10);
    _amap_test(a);
    amap_destroy(a);

    // Another test with non-alligned size
    a = amap_new(1024*1024*10+1);
    _amap_test(a);
    amap_destroy(a);
}

