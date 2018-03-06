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

#include "ranges.h"

/**
 * @brief Callback for printing a range
 *
 * @param data The btree key, representing the from offset
 * @param data The btree value, representing the to offset
 */
void _range_print_cb(void *data, void *data2)
{
    long from = (long) data;
    long to = (long) data2;

    Put("\tfrom:'%ld' to:'%ld'", from, to);
}

range_s *range_new(const long from, const long to)
{
    range_s *r = Malloc(range_s);

    r->from = from;
    r->to = to;

    return r;
}

void range_destroy(range_s *r)
{
    free(r);
}

ranges_s *ranges_new()
{
    ranges_s *r = Malloc(ranges_s);
    *r = (ranges_s) {
        .btree = btree_new()
    };
    return r;
}

void ranges_destroy(ranges_s *r)
{
    btree_destroy(r->btree);
    free(r);
}

void range_ensure(ranges_s* r, long from, long to)
{
}

void ranges_print(ranges_s* r)
{
    Put("ranges:%p", (void*)r);
    btree_run_cb2(r->btree, _range_print_cb);
}

void ranges_test(void)
{
    ranges_s *r = ranges_new();
    range_ensure(r, 0, 10);
    range_ensure(r, 1032, 2000);
    ranges_print(r);
    ranges_destroy(r);
}
