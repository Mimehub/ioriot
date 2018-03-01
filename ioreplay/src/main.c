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

/**
 * @file main.c
 * @author Paul Buetow
 *
 * @brief The entry point of the I/O Replay program.
 */

#include <signal.h>
#include <fcntl.h>

#include "capture/capture.h"
#include "cleanup/cleanup.h"
#include "generate/generate.h"
#include "init/init.h"
#include "mounts.h"
#include "options.h"
#include "replay/replay.h"
#include "utests.h"
#include "utils/utils.h"

/**
 * @brief Do some architecture checks
 *
 * To ensure that I/O replay works correctly we have to check whether some
 * data types are atomic or not. This is what this function does!
 */
static void _arch_check_atomic(void)
{
    if (sizeof(int) > sizeof(sig_atomic_t)) {
        Error("int data type is not atomic on this architecture: %ld > %ld",
              sizeof(int), sizeof(sig_atomic_t));

    } else if (sizeof(bool) > sizeof(sig_atomic_t)) {
        Error("bool data type is not atomic on this architecture: %ld > %ld",
              sizeof(bool), sizeof(sig_atomic_t));
    }
}

/**
 * @brief Prints out version and copyright information
 */
static void _print_version(void)
{
    Put("This is I/O Replay %s - %s", IOREPLAY_VERSION, IOREPLAY_COPYRIGHT);
}

/**
 * @brief Print the synopsis
 */
static void _print_synopsis(void)
{
    _print_version();

    Put("Synopsis:");
    Put("\tioreplay -c io.capture [-x PID] [-m MODULE]");
    Put("\tioreplay -c io.capture -r io.replay [-n str] [-u str] [-w str]");
    Put("\tioreplay -i io.replay");
    Put("\tioreplay -r io.replay [-p #] [-t #] [-D] [-s #]");
    Put("\tioreplay -R io.replay [-p #] [-t #] [-D] [-s #]");
    Put("\tioreplay -d");
    Put("\tioreplay -P");
    Put("\tioreplay -T [-n NAME]");
    Put("\tioreplay -V");
}

/**
 * @brief Print a brief help
 */
static void _print_help(void)
{
    _print_synopsis();

    Put("Help:");
    Put("\t-d            Drop all Linux/FS caches and exit ioreplay");
    Put("\t-D            Don't drop all caches (in conjunction with -r/-R):");
    Put("\t-s SPEED      The speed factor (default: 0 [as fast as possible])");
    Put("\t-h            Print this help");
    Put("\t-c FILE       The capture file");
    Put("\t-n NAME       The name (default: test0)");
    Put("\t-u USER       The test run user (default: mcuser)");
    Put("\t-p #WORKERS   Amount of of parallel worker processes (default: 4)");
    Put("\t-t #THREADS   Threads per worker process (default: 128)");
    Put("\t-i REPLAYFILE The replay file to be initialised");
    Put("\t-r REPLAYFILE The replay file to be replayed");
    Put("\t-R REPLAYFILE Init and replay in one run (-i and -r combined)");
    Put("\t-S STATSFILE  Write a stats file at the end of a test");
    Put("\t-T            Trash data directories");
    Put("\t-P            Purge all trash directories of all tests)");
    Put("\t-V            Print I/O replay program version");
    Put("\t-w WD_BASE    The working directory's base path");
    Put("\t              (default: /usr/local/ioreplay)");
    Put("\t-x PID        To specify a process ID (in conjunction with -c)");
    Put("\t-m MODULE     To specify a module (in conjunction with -c)");
    Put("\nExample (run these commands one after another):");
    Put("\t 1.) sudo ioreplay -c io.capture");
    Put("\t 2.) sudo ioreplay -r io.replay -c io.capture -u paul -n test1");
    Put("\t 3.) sudo ioreplay -i io.replay");
    Put("\t 4.) sudo ioreplay -r io.replay -S");
}

/**
 * @brief I/O Replay's entry point
 *
 * Not much more to document here though!
 * @return The exit code
 */
int main(int argc, char **argv)
{
    _arch_check_atomic();
    status_e ret = UNKNOWN;

    bool dont_drop_caches = false;
    options_s *opts = options_new();
    int opt = 0;

    while ((opt = getopt(argc, argv, "Vr:R:S:c:u:i:hw:n:dDs:w:p:t:UPTx:m:")) != -1) {
        switch (opt) {
        case 'U':
            utests_run();
            Cleanup(SUCCESS);
            break;
        case 'V':
            _print_version();
            Cleanup(SUCCESS);
            break;
        case 'd':
            drop_caches();
            Cleanup(SUCCESS);
            break;
        case 'D':
            dont_drop_caches = true;
            break;
        case 'c':
            opts->capture_file = absolute_path(optarg);
            Put("Capture file: %s", opts->capture_file);
            break;
        case 'P':
            opts->purge = true;
            Put("Purge option set");
            break;
        case 'T':
            opts->trash = true;
            Put("Trash option set");
            break;
        case 'i':
            opts->init = true;
            if (!opts->replay_file) {
                opts->replay_file = absolute_path(optarg);
                Put("Replay file: %s", opts->replay_file);
            }
            break;
        case 'R':
            opts->init = true;
            opts->replay = true;
            if (!opts->replay_file) {
                opts->replay_file = absolute_path(optarg);
                Put("Replay file: %s", opts->replay_file);
            }
            break;
        case 'r':
            opts->replay = true;
            if (!opts->replay_file) {
                opts->replay_file = absolute_path(optarg);
                Put("Replay file: %s", opts->replay_file);
            }
            break;
        case 'S':
            opts->stats_file = Clone(optarg);
            Put("Stats output file: %s", opts->stats_file);
            break;
        case 'w':
            opts->wd_base = optarg;
            Put("WD base: %s", opts->wd_base);
            break;
        case 'u':
            opts->user = optarg;
            Put("User: %s", opts->user);
            break;
        case 'm':
            opts->module = Clone(optarg);
            Put("Module: %s", opts->module);
            break;
        case 'n':
            opts->name = optarg;
            Put("Name: %s", opts->name);
            break;
        case 'h':
            _print_help();
            Cleanup(SUCCESS);
        case 's':
            sscanf(optarg, "%lf", &opts->speed_factor);
            Put("Speed factor: %lf", opts->speed_factor);
            break;
        case 'p':
            opts->num_workers = atoi(optarg);
            if (opts->num_workers < 1)
                opts->num_workers = 1;
            Put("Num worker processes: %d", opts->num_workers);
            break;
        case 't':
            opts->num_threads_per_worker = atoi(optarg);
            if (opts->num_threads_per_worker < 1)
                opts->num_threads_per_worker = 1;
            Put("Num threads per worker: %d", opts->num_threads_per_worker);
            break;
        case 'x':
            opts->pid = atoi(optarg);
            Put("PID: %d", opts->pid);
            break;
        default:
            _print_help();
            Cleanup(ERROR);
        }
    }

    if (opts->purge || opts->trash) {
        // Clean up all temp data of previous test runs
        Cleanup(cleanup_run(opts));

    } else if (opts->capture_file && !opts->replay_file) {
        // We are going to capture I/O 
        Cleanup(capture_run(opts));

    } else if (opts->capture_file && opts->replay_file) {
        // We are going to generate a .replay file from the .capture file
        Cleanup(generate_run(opts));

    } else if (opts->replay_file && opts->init && !opts->replay) {
        // We are going to initialise the test from the .replay file!
        Cleanup(init_run(opts));

    } else if (opts->replay_file && opts->init && opts->replay) {
        // We are going to initialise the test and run the test! Run the
        // initialiser in a sub-process, as it drops root privileges!
        pid_t pid = fork();
        if (pid == 0) {
            Cleanup(init_run(opts));
        } else {
            opts->drop_caches = !dont_drop_caches;
            int init_status;
            waitpid(pid, &init_status, 0);
            // Only proceed if initialisation was successfull!
            Cleanup_unless(SUCCESS, init_status);
            Cleanup(replay_run(opts));
        }

    } else if (opts->replay_file && !opts->init && opts->replay) {
        // We are going to replay the I/O
        opts->drop_caches = !dont_drop_caches;
        Cleanup(replay_run(opts));

    } else {
        _print_help();
        Cleanup(ERROR);
    }

cleanup:
    options_destroy(opts);

    return ret;
}
