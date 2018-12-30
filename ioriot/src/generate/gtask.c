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

#include "gtask.h"

gtask_s* gtask_new(void *generate)
{
    gtask_s *t = Malloc(gtask_s);

    t->generate = generate;
    t->line = NULL;
    t->path_r = NULL;
    t->path2_r = NULL;
    t->fdid = NULL;
#ifdef LOG_FILTERED
    t->original_line = NULL;
#endif

    return t;
}

void gtask_init(gtask_s *t, char *line, const unsigned long lineno)
{
    if (t->line)
        free(t->line);
    t->line = Clone(line);

    if (t->path_r)
        free(t->path_r);
    if (t->path2_r)
        free(t->path2_r);
    if (t->fdid)
        free(t->fdid);

#ifdef LOG_FILTERED
    if (t->original_line)
        free(t->original_line);
    t->original_line = Clone(line);
    t->filtered_where = NULL;
#endif

    t->bytes = -1;
    t->address = 0;
    t->address2 = 0;
    t->count = -1;
    t->F = -1;
    t->fd = -1;
    t->fdid = NULL;
    t->flags = -1;
    t->G = -1;
    t->has_fd = false;
    t->lineno = lineno;
    t->mapped_fd = -1;
    t->mapped_time = -1;
    t->mode = -1;
    t->offset = -1;
    t->op = NULL;
    t->path = NULL;
    t->path_r = NULL;
    t->pid = -1;
    t->pidtid = NULL;
    t->ret = 0;
    t->status = -1;
    t->tid = -1;
    t->vfd = NULL;
    t->whence = -1;
}

void gtask_destroy(gtask_s *t)
{
    if (t->line)
        free(t->line);
    if (t->path_r)
        free(t->path_r);
    if (t->path2_r)
        free(t->path2_r);
    free(t);
}

