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

#include "tpool.h"

tpool_s *tpool_new(int max_threads)
{
    tpool_s *t = Malloc(tpool_s);
    t->max_threads = max_threads;

    return t;
}

void tpool_join_threads(tpool_s *t)
{
}

void tpool_destroy(tpool_s *t)
{
    tpool_join_threads(t);
    free(t);
}

void tpool_add_work3(tpool_s* t,
                     void (*callback)(void *data, void *data2, void *data3),
                     void *data, void *data2, void *data3)
{
    // TODO: Implement multi threading, as for now this is a dummy tpool
    callback(data, data2, data3);
}

void tpool_test(void)
{
    tpool_s* t = tpool_new(1024);
    tpool_destroy(t);
}
