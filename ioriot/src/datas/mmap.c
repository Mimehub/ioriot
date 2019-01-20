// Copyright 2019 Mimecast Ltd.
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

#include "mmap.h"

mmap_s* mmap_new(char *name, unsigned long size)
{
    mmap_s *m = Malloc(mmap_s);
    Asprintf(&m->file, "%s.mmap", name);

    m->size = size;
    if (m->size > MAX_MMAP_SIZE) {
        Error("Exceeded max mmap size of '%ld' (%ld)", MAX_MMAP_SIZE, m->size);
    }

    m->fd = open(m->file, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (m->fd == -1) {
        Errno("Could not open file '%s'", m->file);
    }

    // We want a sparse file, so we can mmap 'size' bytes
    if (lseek(m->fd, m->size-1, SEEK_SET) == -1)
    {
        Errno("Could not sparse file '%s' (size:%ld:%ld)", m->file, m->size, size);
    }

    // Write '\0' at the end to the file
    if (write(m->fd, "", 1) == -1)
    {
        Errno("Could not write last byte to file '%s'", m->file);
    }

    m->memory = mmap(0, m->size, PROT_READ | PROT_WRITE, MAP_SHARED, m->fd, 0);
    if (m->memory == MAP_FAILED)
    {
        Error("Error mmapping file '%s'", m->file);
    }

    return m;
}

mmap_s* mmap_open(char *name) {
    mmap_s *m = Malloc(mmap_s);
    Asprintf(&m->file, "%s.mmap", name);

    struct stat st;
    if (-1 == stat(m->file, &st)) {
        Errno("Could not determine file size of file '%s'", m->file);
    }
    m->size = st.st_size;
    if (m->size > MAX_MMAP_SIZE) {
        Error("Exceeded max mmap size of '%ld' (%ld)", MAX_MMAP_SIZE, m->size);
    }

    m->fd = open(m->file, O_RDWR);
    if (m->fd == -1) {
        Errno("Could not open file '%s'", m->file);
    }

    m->memory = mmap(0, m->size, PROT_READ | PROT_WRITE, MAP_SHARED, m->fd, 0);
    if (m->memory == MAP_FAILED)
    {
        Errno("Error mmapping file '%s'", m->file);
    }

    return m;
}

void mmap_destroy(mmap_s *m)
{
    if (munmap(m->memory, m->size) == -1) {
        Error("Error unmapping file '%s'", m->file);
    }
    close(m->fd);
    free(m->file);
}

void mmap_unlink(mmap_s *m)
{
    if (-1 == unlink(m->file)) {
        Errno("Could not delete file '%s'", m->file);
    }
}

void mmap_test_simple()
{
    char *text = "Hello mmap!";

    // First write to a mmapped file
    mmap_s *m = mmap_new("mmap_test_simple", MAX_MMAP_SIZE);
    char *mapped = m->memory;
    strcpy(mapped, text);
    mmap_destroy(m);

    // Now read it from the file
    m = mmap_open("mmap_test_simple");
    mapped = m->memory;
    assert(strncmp(text, mapped, strlen(text)) == 0);
    //mmap_unlink(m);
    mmap_destroy(m);
}

typedef struct _mmap_test_s_ {
    int id;
    char name[24];
} _mmap_test_s;

void mmap_test_complex()
{
    int num_elems = 1000000;

    Put("mmap_test_complex: %d", num_elems);

    // First write to a mmapped file
    mmap_s *m = mmap_new("mmap_test_complex", sizeof(_mmap_test_s) * num_elems);
    _mmap_test_s *mapped = m->memory;
    for (int i = 0; i < num_elems; ++i) {
        mapped[i].id = i * 2;
        strcpy(mapped[i].name, "I have no real name");
    }
    mmap_destroy(m);

    // Now read it from the file
    m = mmap_open("mmap_test_complex");
    mapped = m->memory;
    for (int i = 0; i < num_elems; ++i)
        assert(mapped[i].id == i * 2);
    mmap_unlink(m);
    mmap_destroy(m);
}

void mmap_test()
{
    mmap_test_simple();
    mmap_test_complex();
}
