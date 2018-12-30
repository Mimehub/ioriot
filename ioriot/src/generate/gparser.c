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

#define _GNU_SOURCE
//#include <stdio.h>

#include "gparser.h"

#include "gtask.h"
#include "gwriter.h"

void* gparser_pthread_start(void *data)
{
    gparser_s *p = data;
    generate_s *g = p->generate;
    gwriter_s *w = g->writer;
    gtask_s *t = NULL;

    do {
        while (NULL != (t = rbuffer_get_next(p->queue))) {
            // First extract
            gparser_extract(p, t);
            // Second, pass the task to the writer thread
            rbuffer_insert(w->queue, t);
        }
        usleep(100);
    } while (!p->terminate);

    while (NULL != (t = rbuffer_get_next(p->queue))) {
        gparser_extract(p, t);
        rbuffer_insert(w->queue, t);
    }

    return NULL;
}

gparser_s* gparser_new(generate_s *g)
{
    gparser_s *p = Malloc(gparser_s);

    p->generate = g;
    p->terminate = false;
    p->queue = rbuffer_new(1024);

    return p;
}

void gparser_start(gparser_s *p)
{
    start_pthread(&p->pthread, gparser_pthread_start, (void*)p);
}

void gparser_destroy(gparser_s *p)
{
    rbuffer_destroy(p->queue);
    free(p);
}

void gparser_terminate(gparser_s *p)
{
    p->terminate = true;
    pthread_join(p->pthread, NULL);
}

void gparser_extract(gparser_s *p, gtask_s *t)
{
    status_e ret = SUCCESS;
    generate_s *g = p->generate;

    char *saveptr;
    char* tok = strtok2_r(t->line, ";:,", &saveptr);
    int ntoks = 0;

    while (tok) {
        if (++ntoks > MAX_TOKENS) {
            ret = ERROR;
            break;
        }
        ret = gparser_extract_tok(p, t, tok);
        if (ret != SUCCESS)
            break;

        tok = strtok2_r(NULL, ";:,", &saveptr);
    }

    if (ret == SUCCESS) {
        // Check for the existance of mandatory values!
        if (t->pid < 0 || t->tid < 0) {
            Cleanup(ERROR);

        } else if (t->op == NULL) {
            Cleanup(ERROR);

        } else if (t->mapped_time == -1) {
            Cleanup(ERROR);
        }

        // We are inserting ".ioriot/NAME" to the paths. This enables us to
        // run multiple tests simoultaneously.

        if (t->path) {
            if (!mounts_transform_path(g->mps, g->name,
                                       t->path, &t->path_r)) {
                Cleanup(ERROR);
            }
            if (t->path_r)
                t->path = t->path_r;
        }

        if (t->path2) {
            if (!mounts_transform_path(g->mps, g->name,
                                       t->path2, &t->path2_r)) {
                Cleanup(ERROR);
            }
            if (t->path2_r)
                t->path2 = t->path2_r;
        }
    }

    if (t->has_fd) {
        // TODO: All asprintf functions should check result type (whole I/O Rout source base)
        if (-1 == asprintf(&t->fdid, "%ld:%d", t->pid, t->fd)) {
            Error("Could not allocate memory");
        }
    }

cleanup:

    t->ret = ret;

#ifdef LOG_FILTERED
    t->filtered_where = __FILE__;
#endif
}

status_e gparser_extract_tok(gparser_s *p, gtask_s *t, char *tok)
{
    status_e ret = SUCCESS;

    if (gparser_token_not_ok(p, tok)) {
        Cleanup(ERROR);
    }

    generate_s *g = t->generate;

    char key = tok[0];
    char *value = tok;
    value += 2;

    switch (key) {
    case 'a':
        // Address
        t->address = strtol(value, NULL, 10);
        break;

    case 'A':
        // Address 2
        t->address2 = strtol(value, NULL, 10);
        break;

    case 'b':
        // Bytes
        if (t->bytes != -1) {
            Cleanup(ERROR);
        }
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->bytes = strtol(value, NULL, 10);
        break;

    case 'c':
        // Count
        if (t->count != -1) {
            Cleanup(ERROR);
        }
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->count = strtol(value, NULL, 10);
        break;

    case 'd':
        // Descriptor
        if (t->fd != -1) {
            Cleanup(ERROR);
        }
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->fd = atoi(value);
        if (t->fd > 0)
            t->has_fd = true;
        break;

    case 'f':
        // Flags
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->flags = atoi(value);
        break;

    case 'i':
        // PID:TID
        t->pidtid = value;
        // Extract PID and TID from "PID:TID"
        if (!gparser_get_pidtid(p, t->pidtid, &t->pid, &t->tid)) {
            Cleanup(ERROR);
        }
        break;

    case 'm':
        // Mode
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->mode = atoi(value);
        break;

    case 'o':
        // Operation
        t->op = value;
        break;

    case 'O':
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->offset = strtol(value, NULL, 10);
        break;

    case 'W':
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->whence = strtol(value, NULL, 10);
        break;

    case 'p':
        // File path
        t->path = value;
        chreplace(t->path, '|', '_');
        strunquote(t->path);
        break;

    case 'P':
        // File path 2
        t->path2 = value;
        chreplace(t->path2, '|', '_');
        strunquote(t->path2);
        break;

    case 's':
        // Cleanup status
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->status = atoi(value);
        break;

    case 't':
        // Time
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->mapped_time = strtol(value, NULL, 10);
        // Start replay time from 0
        if (g->start_time == -1) {
            g->start_time = t->mapped_time;
        }
        t->mapped_time -= g->start_time;
        break;

    case 'F':
        // FCNTL function
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->F = atoi(value);
        break;

    case 'G':
        // FCNTL argument
        if (is_number(value) == 0) {
            Cleanup(ERROR);
        }
        t->G = atoi(value);
        break;

    case 'T':
        break;

    case 'D':
        break;

    default:
        // Unknown key
    {
        Cleanup(ERROR);
    }
    }

cleanup:
    if (t->path_r) {
        free(t->path_r);
        t->path_r = NULL;
    }
    if (t->path2_r) {
        free(t->path2_r);
        t->path2_r = NULL;
    }

    return ret;
}

bool gparser_token_not_ok(gparser_s *p, char *tok)
{
    if (strlen(tok) < 3) {
        return true;

    } else if (tok[1] != '=') {
        return true;
    }

    return false;
}

bool gparser_get_pidtid(gparser_s *p, char *pidtid, long *pid, long *tid)
{
    char *pos = strchr(pidtid, ':');

    if (pos) {
        char *tmp = pos;
        tmp++;

        if (is_number(tmp)) {
            *tid = atol(tmp);
        } else {
            return false;
        }

        pos[0] = '\0';
        if (is_number(pidtid)) {
            *pid = atol(pidtid);
        } else {
            return false;
        }
    }

    else {
        return false;
    }

    return (*pid >= 0 && *tid >= 0);
}
