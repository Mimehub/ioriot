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

generate_s* generate_new(options_s *opts)
{
    generate_s *g = Malloc(generate_s);

    g->writer = NULL;
    g->lineno = 0;
    g->name = opts->name;
    g->replay_fd = NULL;
    g->mps = mounts_new(opts);
    g->num_lines_filtered = 0;
    g->num_vsizes = 0;
    g->start_time = -1;
    g->pid_map = amap_new(_MAX_PROCESSES);
    g->vsize_map = hmap_new(_MAX_PROCESSES);
    g->mmap_map = hmap_new(1024*1024);
    g->vfd_buffer = rbuffer_new(1024);
    g->num_mapped_pids = 0;
    g->num_mapped_fds = 10;
    g->opts = opts;
    g->reuse_queue = rbuffer_new(1024);
    g->replay_fd = Fopen(opts->replay_file, "w");

    return g;
}

void generate_destroy(generate_s *g)
{
    // TODO: Also clean the contets of these maps
    amap_destroy(g->pid_map);
    hmap_destroy(g->vsize_map);
    hmap_destroy(g->mmap_map);
    rbuffer_destroy(g->vfd_buffer);
    mounts_destroy(g->mps);

    gtask_s *task = NULL;
    while (NULL != (task = rbuffer_get_next(g->reuse_queue)))
        gtask_destroy(task);
    rbuffer_destroy(g->reuse_queue);

    fclose(g->replay_fd);
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

    // Reserve first few bytes for meta information
    meta_s *meta = meta_new(g->replay_fd);
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
    hmap_run_cb(g->vsize_map, generate_write_init_cb);

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

    meta_write_l(meta, "num_vsizes", g->num_vsizes);
    meta_write_l(meta, "num_mapped_pids", g->num_mapped_pids);
    meta_write_l(meta, "num_mapped_fds", g->num_mapped_fds);
    meta_write_l(meta, "num_lines", g->lineno - g->num_lines_filtered);

    meta_destroy(meta);
    fclose(capture_fd);

    Put("Generating '%s' done", opts->replay_file);
    generate_destroy(g);

    return SUCCESS;
}

void _write_ranges_cb(long start, void *data, void *data2)
{
    vsize_s *v = data2;
    generate_s *g = v->generate;
    long end = (long) data;
    long bytes = end-start;
    if (bytes > 0) {
        fprintf(g->replay_fd, "%d|%d|%ld|%ld|%s|\n",
                v->is_dir, v->is_file, start, bytes, v->path);
    }
}

void generate_write_init_cb(void *data)
{
    vsize_s *v = data;
    generate_s *g = v->generate;

    if (v->required && strlen(v->path) > 0) {
        if (v->read_ranges) {
            btree_run_cb2(v->read_ranges, _write_ranges_cb, data);
        } else if (v->bytes >= 0) {
            fprintf(g->replay_fd, "%d|%d|%ld|%ld|%s|\n",
                    v->is_dir, v->is_file, 0L, v->bytes, v->path);
        }
    }
}

vsize_s* generate_vsize_by_path(generate_s *g, gtask_s *t,
                                char *path)
{
    vsize_s *v = NULL;

    if (!path && t)
        path = t->path;

    Error_if(!path, "No path specified");
    v = hmap_get(g->vsize_map, path);

    if (!v) {
        v = vsize_new(path, ++g->num_vsizes, g);
        hmap_insert(g->vsize_map, path, v);
    }

    if (t)
        t->vsize = v;

    return v;
}

void generate_gprocess_by_realpid(generate_s *g, gtask_s *t)
{
    // Get the virtual process data object from the virtual PID space.
    t->gprocess = amap_get(g->pid_map, t->pid);
    if (t->gprocess == NULL) {
        t->gprocess = gprocess_new(t->pid, ++g->num_mapped_pids);
        if (amap_set(g->pid_map, t->pid, t->gprocess)) {
            Error("lineno:%lu Can not insert PID %ld", t->lineno, t->pid);
        }
    }
}
