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

#include "../defaults.h"

#include "owriter.h"

owriter_s* owriter_new(char *journal_path)
{
    owriter_s *o = Malloc(owriter_s);
    o->journal_path = Clone(journal_path);
    Out("Opening '%s'", o->journal_path);
    o->fd = Fopen(o->journal_path, "w");
    return o;
}

void owriter_destroy(owriter_s* o)
{
    Out("Closing '%s'", o->journal_path);
    free(o->journal_path);
    fclose(o->fd);
    free(o);
}

int owriter_write(owriter_s* o, const char *line, off_t *start)
{
    *start = fseek(o->fd, 0, SEEK_CUR);
    int bytes = strlen(line) + 1;
    fprintf(o->fd, "%s\n", line);
    return bytes;
}
