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

#include "vfd.h"

vfd_s* vfd_new(const int fd, const long mapped_fd, char *path)
{
    vfd_s *vfd = Malloc(vfd_s);
    vfd->path = NULL;
    vfd->debug = false;
    vfd_update(vfd, fd, mapped_fd, path);

    return vfd;
}

void vfd_update(vfd_s *vfd, const int fd, const long mapped_fd, char *path)
{
    vfd->fd = fd;
    vfd->dirfd = NULL;
    vfd->mapped_fd = mapped_fd;
    vfd->offset = 0;

    if (path)
        free(vfd->path);

    vfd->path = Clone(path);
}

void vfd_destroy(vfd_s *vfd)
{
    if (!vfd)
        return;

    if (vfd->path)
        free(vfd->path);

    free(vfd);
}

void vfd_print(vfd_s *vfd)
{
    fprintf(stderr, "virtfd(%p) fd:%x mapped_fd:%lx path:%s\n",
            (void*)vfd, vfd->fd, vfd->mapped_fd, vfd->path);
}
