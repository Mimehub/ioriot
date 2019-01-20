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

#include "utests.h"

#include "datas/amap.h"
#include "datas/btree.h"
#include "datas/graph.h"
#include "datas/hmap.h"
#include "datas/list.h"
#include "datas/mmap.h"
#include "datas/rbuffer.h"
#include "tpool/tpool.h"
#include "utils/utils.h"

void utests_run()
{
    fprintf(stderr, "Running mmap_test()\n");
    mmap_test();

    fprintf(stderr, "Running graph_test()\n");
    graph_test();

    fprintf(stderr, "Running tpool_test()\n");
    tpool_test();

    fprintf(stderr, "Running hmap_test()\n");
    hmap_test();

    fprintf(stderr, "Running btree_test()\n");
    btree_test();

    fprintf(stderr, "Running hmap_test()\n");
    hmap_test();

    fprintf(stderr, "Running list_test()\n");
    list_test();

    fprintf(stderr, "Running rbuffer_test()\n");
    rbuffer_test();

    fprintf(stderr, "Running utils_test()\n");
    utils_test();

    //fprintf(stderr, "Running amap_test()\n");
    //amap_test();

    fprintf(stderr, "Great success, run all unit tests without any errors!\n");
}
