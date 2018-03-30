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

#include "replay.h"

#include "../datas/amap.h"
#include "../meta/meta.h"
#include "../mounts.h"
#include "rworker.h"
#include "rstats.h"

void replay_extract_header(options_s *opts, FILE *replay_fd, long *num_vsizes,
                           long *num_pids, long *num_fds, long *num_lines)
{
    meta_s *m = meta_new(replay_fd);
    meta_read_start(m);

    long version = 0;
    if (meta_read_l(m, "version", &version)) {
        Put("Replay version is '%ld'", version);
        if (version != REPLAY_VERSION) {
            Error(".replay file of incompatible version, got %x, expected %x",
                  (int)version, REPLAY_VERSION);
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

    if (meta_read_l(m, "num_vsizes", num_vsizes)) {
        if (*num_vsizes < 0) {
            Error("Lamport vsize overflow");
        }
        Put("Setting num of vsizes to '%ld'", *num_vsizes);
    }

    if (meta_read_l(m, "num_mapped_pids", num_pids)) {
        if (*num_pids < 0) {
            Error("Process overflow (too many process IDs in .replay)");
        }
        Put("Setting num of PIDs to '%ld'", *num_pids);
    }

    if (meta_read_l(m, "num_mapped_fds", num_fds)) {
        if (*num_fds < 0) {
            Error("FD overflow (too many FDs in .replay)");
        }
        Put("Setting num of FDs to '%ld'", *num_fds);
    }

    if (meta_read_l(m, "num_lines", num_lines)) {
        if (*num_fds < 0) {
            Error("Overflow (too many lines in .replay)");
        }
        Put("Setting num of lines to '%ld'", *num_lines);
    }

    meta_destroy(m);
}

status_e replay_run(options_s *opts)
{
    status_e status = SUCCESS;

    if (opts->drop_caches) {
        drop_caches();
        //cache_file(opts->replay_file);
    }

    // Extract information from the meta header
    FILE *replay_fd = Fopen(opts->replay_file, "r");
    long num_vsizes = 0, num_pids = 0, num_fds = 0, num_lines = 0;
    replay_extract_header(opts, replay_fd, &num_vsizes, &num_pids,
                          &num_fds, &num_lines);
    fclose(replay_fd);

    // A map of all file descriptors used.
    Out("Creating FD map...");
    amap_s *fds_map = NULL;
    if (opts->num_workers > 1) {
        fds_map = amap_new_mmapped(num_fds);
    } else {
        fds_map = amap_new(num_fds);
    }
    Put("done");

    // To collect all individual worker's stats into the global
    // stats object.
    stack_s *all_worker_stats = stack_new();

    // The global stats object
    rstats_s *stats = rstats_new(opts);
    rstats_start(stats);

    // Fork worker processes, each worker process will read the .replay file
    // individually.

    if (opts->num_workers > 1) {
        for (int i = 0; i < opts->num_workers; ++i) {
            rworker_stats_s *worker_stats = rworker_stats_new_mmap();
            stack_push(all_worker_stats, worker_stats);

            pid_t pid = fork();

            if (pid == 0) {
                // One worker object per fork
                rworker_s *w = rworker_new(i, fds_map, num_vsizes, num_pids, opts,
                                           worker_stats);

                // Process the .replay journal line by line
                status_e status = rworker_process_lines(w, num_lines);
                Put("worker(%d): Exiting from %d with status %d", i,
                    pid, status);
                rworker_destroy(w);

                // Exit sub-process
                exit(status);

            } else if (pid < 0) {
                Errno("worker(%d): Unable to create worker process! :'-(", i);

            } else {
                Put("worker(%d): Process with pid %d forked", i, pid);
            }
        }

        set_limits_drop_root(opts->user);

        Put("Waiting for worker processes to finish");
        pid_t pid;
        int rworker_status = SUCCESS;

        while ((pid = wait(&rworker_status)) > 0) {
            if (rworker_status != SUCCESS)
                status = rworker_status;

            Put("Process with pid %d exited with status %d",
                pid, rworker_status);
        }

        Put("All workers finished (%d)!", status);

    } else {
        Put("Only one worker, don't fork sub-processes");

        rworker_stats_s *worker_stats = rworker_stats_new_mmap();
        stack_push(all_worker_stats, worker_stats);

        rworker_s *w = rworker_new(0, fds_map, num_vsizes, num_pids,
                                   opts, worker_stats);
        status = rworker_process_lines(w, num_lines);
        rworker_destroy(w);

        Put("Worker finished work!");
    }

    // Collect all statistics
    rstats_stop(stats);
    while (!stack_is_empty(all_worker_stats)) {
        rworker_stats_s *worker_stats = stack_pop(all_worker_stats);
        rstats_add_from_worker(stats, worker_stats);
        rworker_stats_destroy(worker_stats);
    }
    stack_destroy(all_worker_stats);

    rstats_print(stats);
    rstats_destroy(stats);

    amap_destroy(fds_map);
    return status;
}
