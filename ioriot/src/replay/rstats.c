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

#include "rstats.h"

#include <sys/types.h>

rstats_s* rstats_new(options_s *opts)
{
    rstats_s *s = Malloc(rstats_s);

    s->opts = opts;
    s->loadavg_high = 0;
    s->ioops = 0;
    s->duration = 0;
    s->time_ahead = -1;

    if (opts->stats_file)
        s->stats_fd = Fopen(opts->stats_file, "w");
    else
        s->stats_fd = stdout;

    return s;
}

void rstats_destroy(rstats_s *s)
{
    if (s->stats_fd != stdout)
        fclose(s->stats_fd);

    free(s);
}

rworker_stats_s* rworker_stats_new_mmap(options_s *opts)
{
    // Share this object between processes, so that the stats cann be
    // collected by the master process!
    rworker_stats_s *s = Mmapshared(rworker_stats_s);

    s->loadavg_high = 0;
    s->ioops = 0;
    s->time_ahead = -1;

    return s;
}

void rworker_stats_destroy(rworker_stats_s *s)
{
    munmap(s, sizeof(rworker_stats_s));
}


void rstats_start(rstats_s* s)
{
    gettimeofday(&s->start_time, NULL);
}

void rstats_stop(rstats_s* s)
{
    gettimeofday(&s->end_time, NULL);
    s->duration= ((s->end_time.tv_sec - s->start_time.tv_sec) * 1000
                  + (s->end_time.tv_usec - s->start_time.tv_usec) / 1000) / 1000;

}

void rstats_add_from_worker(rstats_s* s, rworker_stats_s* w)
{
    if (s->loadavg_high < w->loadavg_high)
        s->loadavg_high = w->loadavg_high;

    if (s->time_ahead == -1 || s->time_ahead > w->time_ahead)
        s->time_ahead = w->time_ahead;

    s->ioops += w->ioops;
}

void rstats_print(rstats_s* s)
{
    options_s *opts = s->opts;

    if (opts->stats_file) {
        Put("Writing stats to '%s'", opts->stats_file);
    }

    fprintf(s->stats_fd, "Stats of test '%s':\n", opts->name);
    fprintf(s->stats_fd, "\tNum workers: %d\n", opts->num_workers);
    fprintf(s->stats_fd, "\tThreads per worker: %d\n", opts->num_threads_per_worker);
    fprintf(s->stats_fd, "\tThreads total: %d\n",
            opts->num_threads_per_worker * opts->num_workers);
    fprintf(s->stats_fd, "\tHighest loadavg: %.2f\n", s->loadavg_high);
    fprintf(s->stats_fd, "\tPerformed ioops: %ld\n", s->ioops);
    if (s->duration > 0)
        fprintf(s->stats_fd, "\tAverage ioops/s: %.2f\n", s->ioops/s->duration);
    fprintf(s->stats_fd, "\tTime ahead: %lds\n", s->time_ahead/1000);
    fprintf(s->stats_fd, "\tTotal time: %.2fs\n", s->duration);
}

