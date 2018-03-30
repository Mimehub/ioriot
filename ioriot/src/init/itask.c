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

#include "itask.h"

itask_s* itask_new()
{
    itask_s *task = Malloc(itask_s);

    task->path = NULL;
    itask_reset_stats(task);

    return task;
}

void itask_destroy(itask_s *task)
{
    if (task->path)
        free(task->path);

    free(task);
}

void itask_reset_stats(itask_s *task)
{
    task->is_dir = task->is_file = false;
    task->sizes_created = task->offset = task->bytes = 0;
    task->dirs_created = task->files_created = 0;

    if (task->path) {
        free(task->path);
        task->path = NULL;
    }
}

void itask_extract_stats(itask_s *task, long* dirs_created, long *files_created,
                         long *files_total_size)
{
    *dirs_created += task->dirs_created;
    *files_created += task->files_created;
    *files_total_size += task->sizes_created;

    if (*dirs_created < 0 || *files_created < 0 || *files_total_size < 0) {
        Error("Size overflow");
    }

    itask_reset_stats(task);
}

void itask_print(itask_s *task)
{
    Put("itask(%p): is_dir:%d is_file:%d offset:%ld bytes:%ld path:%s",
        (void*)task, task->is_dir, task->is_file,
        task->offset, task->bytes, task->path);
}
