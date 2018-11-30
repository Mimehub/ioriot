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

#include "generate.h"

#include "../meta/meta.h"
#include "gtask.h"
#include "gwriter.h"
#include "gparser.h"

#include <fcntl.h>

#define _MAX_PROCESSES 1024*1024*10

#define _Perc_filtered (g->num_lines_filtered / (g->lineno/100.0))

static void _graph_destroy_cb(void *data) {
}

generate_s* generate_new(options_s *opts)
{
    generate_s *g = Malloc(generate_s);

    g->writer = NULL;
    g->lineno = 0;
    g->name = opts->name;
    g->replay_fd = NULL;
    g->mps = mounts_new(opts);
    g->num_lines_filtered = 0;
    g->start_time = -1;
    g->vfd_buffer = rbuffer_new(1024);
    g->opts = opts;
    g->reuse_queue = rbuffer_new(1024);
    g->replay_fd = Fopen(opts->replay_file, "w");
    g->graph = graph_new(1024*1024, _graph_destroy_cb);

    return g;
}

void generate_destroy(generate_s *g)
{
    rbuffer_destroy(g->vfd_buffer);
    mounts_destroy(g->mps);

    gtask_s *task = NULL;
    while (NULL != (task = rbuffer_get_next(g->reuse_queue)))
        gtask_destroy(task);
    rbuffer_destroy(g->reuse_queue);

    fclose(g->replay_fd);
    graph_destroy(g->graph);
    free(g);
}

status_e generate_run(options_s *opts)
{
    generate_s *g = generate_new(opts);
    Put("Parsing file %s, writing output to %s", opts->capture_file,
        opts->replay_file);
    FILE *capture_fd = Fopen(opts->capture_file, "r");

    size_t len = 0;
    ssize_t read;
    char *line = NULL;

    set_limits_drop_root(opts->user);

    // Check for correct capture format version
    meta_s *meta = meta_new(capture_fd);
    meta_read_start(meta);

    long capture_version = 0;
    if (meta_read_l(meta, "capture_version", &capture_version)) {
        Put("Capture version is '%ld'", capture_version);
        if (capture_version != CAPTURE_VERSION) {
            Error(".capture file of incompatible version, got %x, expected %x",
                  (int)capture_version, CAPTURE_VERSION);
        }
    }

    meta_destroy(meta);

    // Reserve first few bytes for meta information
    meta = meta_new(g->replay_fd);
    meta_reserve(meta);

    // The writer will write the .replay file
    gwriter_s *writer = gwriter_new(g);

    // The parser will parse every line of the .capture file
    gparser_s *parser = gparser_new(g);

    g->writer = writer;

    // Start one writer and one parser thread!
    gparser_start(parser);
    gwriter_start(writer);

    Out("Processing, it may take a while: ");

    // Process each line of the .capture file. Determine line by line whether
    // the I/O operation makes sense or not. It might be that SystemTap skipped
    // some I/O ops due to system overload or other issues. The result is that
    // some lines may be corrupt or contain I/O operations on unknown file
    // handles. It could also be that there are operations on unknown
    // file handles such as sockets etc. These will be all filtered out by
    // either the parser or the writer thread!

    while ((read = getline(&line, &len, capture_fd)) != -1) {
        if (line[0] == '#')
            continue;

        if (0 > ++g->lineno) {
            Error("lineno:%lu Line number overflow", g->lineno);
        }
        if (strlen(line) >= MAX_LINE_LEN) {
            Error("lineno:%lu Exceeded max line length", g->lineno);
        }

        // Create a new generate task (try to reuse a task object)...
        gtask_s *t = rbuffer_get_next(g->reuse_queue);
        if (!t) {
            t = gtask_new(g);
        } else if (t->ret != 0) {
            g->num_lines_filtered++;
        }
        gtask_init(t, line, g->lineno);

        // ...pass it to the parser queue
        while (!rbuffer_insert(parser->queue, t))
            usleep(100);

        if (g->lineno % 1000000 == 0) {
            Out(" %lu (filtered:%.2lf%%)", g->lineno, _Perc_filtered);
        }
    }

    Put("\nDone reading input file!");

    Put("Waiting for parser thread...");
    gparser_terminate(parser);
    gparser_destroy(parser);

    Put("Waiting for writer thread...");
    gwriter_terminate(writer);
    gwriter_destroy(writer);

    // Retrieve all left over processed tasks to collect the
    // statistics!
    gtask_s *t;
    while (NULL != (t = rbuffer_get_next(g->reuse_queue))) {
        if (t->ret != 0)
            g->num_lines_filtered++;
        gtask_destroy(t);
    }

    Put("Processed %lu lines in total, had to filter out %.2lf%%",
        g->lineno, _Perc_filtered);

    Put("Writing init section to '%s'...", opts->replay_file);
    fprintf(g->replay_fd, "#INIT\n");
    off_t init_offset = ftello(g->replay_fd);

    Put("Writing meta header to '%s'...", opts->replay_file);
    meta_write_start(meta);

    // The meta header is being written to the first line of the .replay
    // file and used by ioriot to do various things (e.g. initializing
    // the test correctly, creating the internal data structures with the
    // correct sizes etc.

    meta_write_l(meta, "replay_version", REPLAY_VERSION);
    meta_write_l(meta, "init_offset", init_offset);
    meta_write_s(meta, "user", opts->user);
    meta_write_s(meta, "name", opts->name);
    meta_write_l(meta, "num_lines", g->lineno - g->num_lines_filtered);

    meta_destroy(meta);
    fclose(capture_fd);

    Put("Generating '%s' done", opts->replay_file);
    generate_destroy(g);

    return SUCCESS;
}
