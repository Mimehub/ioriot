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

#include "options.h"

options_s *options_new()
{
    options_s *o = Malloc(options_s);

    o->capture_file = NULL;
    o->replay_file = NULL;
    o->stats_file = NULL;
    o->wd_base = "/usr/local/ioriot";
    o->num_workers = 4;
    o->num_threads_per_worker = 128;
    o->user = "nobody";
    o->name = "test0";
    o->init = false;
    o->replay = false;
    o->speed_factor = 0;
    o->drop_caches = false;
    o->purge = false;
    o->trash = false;
    o->pid = -1;
    o->module = "ioriot.ko";

    return o;
}

void options_destroy(options_s *o)
{
    if (o->capture_file)
        free(o->capture_file);
    if (o->replay_file)
        free(o->replay_file);
    if (o->stats_file)
        free(o->stats_file);

    free(o);
}
