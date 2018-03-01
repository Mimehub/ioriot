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

#include "rtask.h"

#include "rthread.h"
#include "rworker.h"

rtask_s* rtask_new()
{
    rtask_s *task = Malloc(rtask_s);

    *task = (rtask_s) {
        .worker = NULL, .process = NULL
    };
    task->line[0] = '\0';

#ifdef THREAD_DEBUG
    task->clone = NULL;
#endif

    return task;
}

void rtask_destroy(rtask_s *task)
{
    if (task)
        free(task);
}

void rtask_update(rtask_s *task, void *worker, void *process, char *line,
                  const long lineno, const long vsize)
{
    task->worker = worker;
    task->process = process;
    task->lineno = lineno;
    task->vsize = vsize;
    strcpy(task->line, line);
}
