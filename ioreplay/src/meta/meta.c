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

#include "meta.h"

#define _MAX_META_LEN 256

meta_s* meta_new(FILE *replay_fd)
{
    meta_s *m = Malloc(meta_s);

    m->replay_fd = replay_fd;
    m->offset = ftello(replay_fd);
    m->read_buf = NULL;

    return m;
}

void meta_destroy(meta_s *m)
{
    if (!m)
        return;

    if (m->read_buf)
        free(m->read_buf);

    free(m);
}

void meta_reserve(meta_s *m)
{
    // TODO: Use a hole in the .replay file to reserve space
    char buf[_MAX_META_LEN];
    Mset(&buf, '#', _MAX_META_LEN-1, char);
    fprintf(m->replay_fd, "%s\n", buf);
}

void meta_write_start(meta_s *m)
{
    fseeko(m->replay_fd, m->offset, SEEK_SET);
    // Write required '#' so that the regular worker processes
    // will ignore that meta line.
    fprintf(m->replay_fd, "#");

    // Required for parsing in 'meta_read_s'
    fprintf(m->replay_fd, "|");
}

void meta_write_s(meta_s *m, char *key, char *val)
{
    fprintf(m->replay_fd, "%s=%s|", key, val);
}

void meta_write_l(meta_s *m, char *key, long val)
{
    char buf[1024];
    sprintf(buf, "%ld", val);
    fprintf(m->replay_fd, "%s=%ld|", key, val);
}

void meta_read_start(meta_s *m)
{
    size_t len = 0;
    m->read_buf = Calloc(_MAX_META_LEN, char);
    getline(&m->read_buf, &len, m->replay_fd);
}

bool meta_read_s(meta_s *m, char *key, char **val)
{
    char *saveptr = NULL;
    char *iterate_buf = Clone(m->read_buf);
    int keylen = strlen(key);

    char *tok = strtok_r(iterate_buf, "|", &saveptr);

    while (tok) {
        if (strncmp(tok, key, keylen) == 0 && tok[keylen] == '=') {
            asprintf(val, "%s", tok+keylen+1);
            free(iterate_buf);
            return true;
        }
        tok = strtok_r(NULL, "|", &saveptr);
    }

    free(iterate_buf);
    return false;
}

bool meta_read_l(meta_s *m, char *key, long *val)
{
    char *buf = NULL;

    if (meta_read_s(m, key, &buf)) {
        *val = atol(buf);
        free(buf);
        return true;
    }

    return false;
}
