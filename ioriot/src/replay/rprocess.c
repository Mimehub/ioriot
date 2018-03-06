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

#include "rprocess.h"

rprocess_s* rprocess_new(const int pid, amap_s *fds_map)
{
    rprocess_s *p = Malloc(rprocess_s);

    p->fds_map = fds_map;
    p->pid = pid;
    p->terminate = 0;
    p->lineno = 0;

    return p;
}

void rprocess_destroy(rprocess_s *p)
{
    if (!p)
        return;
    free(p);
}
