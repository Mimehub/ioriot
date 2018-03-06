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

#include "gprocess.h"

#include "../vfd.h"
#include "gioop.h"

void _gprocess_vfd_map_destroy_cb(void *data)
{
    vfd_destroy(data);
}

gprocess_s* gprocess_new(const long pid, const long mapped_pid)
{
    gprocess_s* gp = Malloc(gprocess_s);

    gp->pid = pid;
    gp->mapped_pid = mapped_pid;
    gp->max_mapped_fd = 0;
    gp->fd_map = hmap_new_l(1024);
    gp->vfd_map = hmap_new_l(1024);
    gp->vfd_map->data_destroy = _gprocess_vfd_map_destroy_cb;

    return gp;
}

void gprocess_destroy(gprocess_s *gp)
{
    hmap_destroy(gp->vfd_map);
    hmap_destroy(gp->fd_map);
    free(gp);
}

void gprocess_create_vfd_by_realfd(gprocess_s *gp, gtask_s *t, generate_s *g)
{
    if (t->fd < 0)
        return;

    // Check whether the real FD is still open according to the .capture log
    long old_mapped = (long) hmap_get_l(gp->fd_map, t->fd);
    if (old_mapped) {

        // That real file descriptor is already with a mapping to a virtual
        // file descriptor.  This may happen when SystemTap missed to trace a
        // 'close' syscall. We are inserting a close now...

        t->vfd = hmap_get_l(gp->vfd_map, old_mapped);

        hmap_remove_l(gp->fd_map, t->fd);
        hmap_remove_l(gp->vfd_map, old_mapped);

        if (t->vfd) {
            generate_vsize_by_path(g, t, t->vfd->path);
            Gioop_write(CLOSE, "%ld|%d|close inserted", old_mapped, 0);
            vsize_close(t->vsize, t->vfd);
            if (!(rbuffer_insert(g->vfd_buffer, t->vfd)))
                vfd_destroy(t->vfd);
        }
    }

    t->vfd = rbuffer_get_next(g->vfd_buffer);
    t->mapped_fd = ++g->num_mapped_fds;
    if (!t->vfd)
        t->vfd = vfd_new(t->fd, t->mapped_fd, t->path);
    else
        vfd_update(t->vfd, t->fd, t->mapped_fd, t->path);
    t->vfd->free_path = t->path_r != NULL;

    hmap_insert_l(gp->vfd_map, t->mapped_fd, t->vfd);
    hmap_insert_l(gp->fd_map, t->fd, (void*)t->mapped_fd);
}

status_e gprocess_vfd_by_realfd(gprocess_s *gp, gtask_s *t)
{
    t->mapped_fd = (long) hmap_get_l(gp->fd_map, t->fd);
    if (t->mapped_fd == 0) {
        // No corresponding virtual fd number mapping
        t->has_fd = false;

    } else {
        t->vfd = hmap_get_l(gp->vfd_map, t->mapped_fd);
        if (!t->vfd) {
            return ERROR;
        }
        t->mapped_fd = t->vfd->mapped_fd;
    }

    return SUCCESS;
}
