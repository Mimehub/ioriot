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

#include "init.h"

#include "../datas/stack.h"
#include "itask.h"
#include "ithread.h"
#include "../meta/meta.h"
#include "../mounts.h"
#include "../utils/futils.h"

init_s *init_new(options_s *opts)
{
    init_s *i = Malloc(init_s);

    i->opts = opts;
    i->mounts = mounts_new(opts);
    i->threads_map = amap_new(i->mounts->count);
    i->reuse_queue = rbuffer_new(4096);
    i->replay_fd = Fopen(opts->replay_file, "r");

    pthread_mutex_init(&i->reuse_queue_mutex, NULL);

    return i;
}

void init_destroy(init_s *i)
{
    amap_destroy(i->threads_map);
    mounts_destroy(i->mounts);

    itask_s *task = NULL;
    while (NULL != (task = rbuffer_get_next(i->reuse_queue))) {
        itask_destroy(task);
    }
    rbuffer_destroy(i->reuse_queue);

    fclose(i->replay_fd);
    pthread_mutex_destroy(&i->reuse_queue_mutex);

    free(i);
}

void init_extract_header(init_s *i, off_t *init_offset)
{
    options_s *opts = i->opts;
    meta_s *m = meta_new(i->replay_fd);
    meta_read_start(m);

    long replay_version = 0;
    if (meta_read_l(m, "replay_version", &replay_version)) {
        Put("Replay version is '%ld'", replay_version);
        if (replay_version != REPLAY_VERSION) {
            Error(".replay file of incompatible version, got %x, expected %x",
                  (int)replay_version, REPLAY_VERSION);
        }
    }

    char *user;
    if (meta_read_s(m, "user", &user)) {
        Put("Setting user to '%s'", user);
        opts->user = user;
    }

    char *name;
    if (meta_read_s(m, "name", &name)) {
        Put("Setting name to '%s'", name);
        opts->name = name;
    }

    if (meta_read_l(m, "init_offset", init_offset)) {
        if (*init_offset < 0) {
            Error("Offset overflow (init offset too large in .replay)");
        }
        Put("Setting init offset to '%ld'", *init_offset);
    }

    meta_destroy(m);
}

status_e init_run(options_s *opts)
{
    status_e ret = SUCCESS;
    init_s *i = init_new(opts);

    off_t init_offset;
    init_extract_header(i, &init_offset);

    // Ensure that all ./replay/NAME directories exist
    mounts_init(i->mounts);

    // Don't do messy stuff as super user
    set_limits_drop_root(opts->user);

    // We need to clean up garbish from previous runs!
    if (opts->purge)
        mounts_purge(i->mounts);
    else
        mounts_trash(i->mounts);

    Out("Creating all files and directories requried for test '%s'...",
        opts->name);

    // Seek to the INIT section
    fseeko(i->replay_fd, init_offset, SEEK_SET);

    bool is_file = false, is_dir = false;
    long offset = 0;
    long bytes = 0;
    char *path;

    // Stats
    long dirs_created = 0;
    long files_created = 0;
    long files_total_size = 0;

    // Helper variables for getline
    char *line = NULL;
    size_t len = 0, read = 0;
    char *saveptr;

    stack_s *all_threads = stack_new();

    // Process the INIT section of the .replay file line by line.

    while ((read = getline(&line, &len, i->replay_fd)) != -1) {
        //Debug(line);
        char *tok = strtok_r(line, "|", &saveptr);

        for (int ntok = 0; tok; ntok++) {
            switch (ntok) {
            case 0:
                is_dir = atoi(tok) == 1;
                break;
            case 1:
                is_file = atoi(tok) == 1;
                break;
            case 2:
                offset = atol(tok);
                if (offset < 0) {
                    Error("Offset overflow: '%ld'", offset);
                }
                break;
            case 3:
                bytes = atol(tok);
                if (bytes < 0) {
                    Error("Size overflow: '%ld'", bytes);
                }
                break;
            case 4:
                path = tok;
                break;
            default:
                break;
            }

            tok = strtok_r(NULL, "|", &saveptr);
        }

        itask_s *task = rbuffer_get_next(i->reuse_queue);

        if (!task) {
            task = itask_new();

        } else {
            itask_extract_stats(task, &dirs_created, &files_created,
                                &files_total_size);
        }

        // Set new task values
        if (is_dir) {
            task->is_dir = true;

        } else if (is_file) {
            task->is_file = true;
            task->bytes = bytes;
            task->offset = offset;
        }
        task->path = Clone(path);

        // We run one init thread per mount point
        int mnr = mounts_get_mountnumber(i->mounts, path);
        ithread_s *t = amap_get(i->threads_map, mnr);

        if (!t) {
            t = ithread_new(i);
            amap_set(i->threads_map, mnr, t);
            stack_push(all_threads, t);
            ithread_start(t);
        }

        //itask_print(task);
        while (!rbuffer_insert(t->queue, task))
            usleep(1000);
    }

    ithread_s *t = NULL;
    while (NULL != (t = stack_pop(all_threads))) {
        ithread_terminate(t);
        ithread_destroy(t);
    }
    stack_destroy(all_threads);

    itask_s *task = NULL;
    while (NULL != (task = rbuffer_get_next(i->reuse_queue))) {
        itask_extract_stats(task, &dirs_created, &files_created,
                            &files_total_size);
        itask_destroy(task);
    }

    Put("Done!");

    Put("Created %ld files (net total size: %.2fg) and %ld directories!",
        files_created, files_total_size/(1024*1024*1024.0),
        dirs_created);

    init_destroy(i);

    Put("You are ready to fire up the test now");

    return ret;
}
