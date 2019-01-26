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

#include "gwriter.h"

#include "gtask.h"
#include "generate.h"
#include "gioop.h"
#include "../opcodes.h"

static void _graph_destroy_cb(void *data)
{
}

void* gwriter_pthread_start(void *data)
{
    gwriter_s *w = data;
    generate_s *g = w->generate;
    gtask_s *t = NULL;

    do {
        while (NULL != (t = rbuffer_get_next(w->queue))) {
#ifdef LOG_FILTERED
            // Logging filtered lines
            if (SUCCESS != gioop_run(w, t)) {
                fprintf(g->replay_fd, "#FILTERED @%ld %s", t->lineno,
                        t->original_line);
            }
#else
            gioop_run(w, t);
#endif
            rbuffer_insert(g->reuse_queue, t);
        }
        usleep(100);
    } while (!w->terminate);

    while (NULL != (t = rbuffer_get_next(w->queue))) {
#ifdef LOG_FILTERED
        if (SUCCESS != gioop_run(w, t)) {
            fprintf(g->replay_fd, "#FILTERED @%ld %s\n", t->lineno,
                    t->original_line);
        }
#else
        gioop_run(w, t);
#endif
        rbuffer_insert(g->reuse_queue, t);
    }

    return NULL;
}

gwriter_s* gwriter_new(generate_s *g)
{
    gwriter_s *w = Malloc(gwriter_s);

    w->generate = g;
    w->terminate = false;
    w->graph = graph_new(1024*1024, _graph_destroy_cb);
    //w->graph = mgraph_new(g->name, 1024*1024, _graph_destroy_cb);
    w->fd_map = hmap_new(1024);
    w->queue = rbuffer_new(1024);
    w->owriter = owriter_new_from_fd(g->replay_fd);

    return w;
}

void gwriter_start(gwriter_s *w)
{
    start_pthread(&w->pthread, gwriter_pthread_start, (void*)w);
}

void gwriter_destroy(gwriter_s *w)
{
    graph_destroy(w->graph);
    hmap_destroy(w->fd_map);
    rbuffer_destroy(w->queue);
    owriter_destroy(w->owriter);
    free(w);
}

void gwriter_terminate(gwriter_s *w)
{
    w->terminate = true;
    pthread_join(w->pthread, NULL);
}
