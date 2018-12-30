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

gprocess_s* gprocess_new(const long pid)
{
    gprocess_s* gp = Malloc(gprocess_s);

    gp->pid = pid;
    gp->fd_map = hmap_new_l(1024);
    gp->fd_map->data_destroy = _gprocess_vfd_map_destroy_cb;

    return gp;
}

void gprocess_destroy(gprocess_s *gp)
{
    hmap_destroy(gp->fd_map);
    free(gp);
}
