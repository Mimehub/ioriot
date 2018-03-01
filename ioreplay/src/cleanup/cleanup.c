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

#include "cleanup.h"

#include "../mounts.h"

status_e cleanup_run(options_s *opts)
{
    drop_root(opts->user);
    mounts_s *m = mounts_new(opts);

    if (opts->purge)
        mounts_purge(m);
    else
        mounts_trash(m);

    return SUCCESS;
}
