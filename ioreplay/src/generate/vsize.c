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

#include "vsize.h"

#include "generate.h"

// Helper macros

#define _Set_file(v) v->is_file = true; v->unsure = v->is_dir = false
#define _Set_dir(v) v->is_dir = true; v->unsure = v->is_file = false
#define _Set_unsure(v) v->unsure = true
#define _Set_inserted(v) v->inserted = true
#define _Set_renamed(v) v->renamed = true
#define _Set_required(v) v->required = true

vsize_s* vsize_new(char *file_path, const unsigned long id,
                   void *generate)
{
    vsize_s *v = Malloc(vsize_s);

    v->generate = generate;
    v->id = id;
    v->inserted = false;
    v->is_dir = false;
    v->is_file = false;
    v->offset = -1;
    v->path = Clone(file_path);
    v->renamed = false;
    v->required = false;
    v->unsure = false;
    v->updates = 0;
    v->vsize = 0;
    v->vsize_deficit = 0;

    return v;
}

void vsize_destroy(vsize_s *v)
{
    if (!v)
        return;

    free(v->path);
    free(v);
}

void init_parent_dir(vsize_s *v, const char *path)
{
    generate_s *g = v->generate;
    char *clone = Clone(path);
    char *parent = dirname(clone);

    vsize_s *v_parent = hmap_get(g->vsize_map, parent);
    if (!v_parent) {

        // Parent directory does not yet have a vsize!
        // Create a vsize object for it and set it as a pre-requirement
        // so that the directory can be created during init mode.

        v_parent = vsize_new(parent, ++g->num_vsizes, g);
        hmap_insert(g->vsize_map, parent, v_parent);

        _Set_required(v_parent);
        _Set_dir(v_parent);

        // This is for debugging purposes only
        _Set_inserted(v_parent);
        v_parent->updates++;

    } else if (v_parent->unsure) {
        // We now know for sure that this path must be a directory!
        _Set_dir(v_parent);
        v_parent->updates++;
    }

    free(clone);
}

void vsize_open(vsize_s *v, void *vfd, const char *path, const int flags)
{

    // v->first_encounter == false means, that this is the first occurance of
    // this path and we didn't initialise it (means we didn't ensure that
    // we want to create all parent directories etc.

    if (v->updates == 0) {
        // We may use a recycled vfd object! When opening a file we always
        // assume that the offset is 0!
        vfd_s *vfd_ = vfd;
        vfd_->offset = 0;
        init_parent_dir(v, path);

        if (Has(flags, O_DIRECTORY)) {
            _Set_required(v);
            _Set_dir(v);

        } else if (Hasnt(flags, O_CREAT)) {
            _Set_required(v);
            _Set_file(v);
            _Set_unsure(v);
        }
        v->updates++;

    } else if (v->unsure) {
        if (Has(flags, O_DIRECTORY)) {
            // Now we know for sure that this path must be a directory!
            _Set_dir(v);
            v->updates++;
        }
    }
}

void vsize_close(vsize_s *v, void* vfd)
{
    vfd_s *vfd_ = vfd;
    vfd_->offset = 0;
    v->updates++;
}

void vsize_stat(vsize_s *v, const char *path)
{
    if (v->updates == 0) {
        init_parent_dir(v, path);
        _Set_required(v);
        _Set_file(v);

        // We are not 100% sure that this is really a file,
        // the path might be still a directory though!
        _Set_unsure(v);
        v->updates++;
    }
}

void vsize_rename(vsize_s *v, vsize_s *v2,
                  const char *path, const char *path2)
{
    if (v->updates == 0) {
        init_parent_dir(v, path);
        _Set_required(v);
        _Set_file(v);
        _Set_unsure(v);
        v->updates++;
    }

    if (v2->updates == 0) {
        init_parent_dir(v2, path2);
        _Set_file(v2);

        // We are not 100% sure that this is really a file,
        // the path might be still a directory though!
        _Set_unsure(v2);

        // For debugging purposes only
        _Set_renamed(v2);
        v2->updates++;
    }
}

void vsize_adjust(vsize_s *v, vfd_s* vfd)
{
    if (v->vsize >= vfd->offset)
        return;

    long deficit = v->vsize - vfd->offset;
    if (deficit < v->vsize_deficit) {
        v->vsize_deficit = deficit;
        _Set_required(v);
        _Set_file(v);
    }
}

void vsize_read(vsize_s *v, void *vfd, const char *path, const int bytes)
{
    vfd_s *vfd_ = vfd;
    vfd_->offset += bytes;
    vsize_adjust(v, vfd_);
    v->updates++;
}

void vsize_seek(vsize_s *v, void *vfd, const long new_offset)
{
    //vfd_s *vfd_ = vfd;

    // The file's offset can be greater than the file's current size, in which
    // case the next write to the file will extend the file. This is referred
    // to as creating a hole in a file and is allowed. However, this behaviour
    // does not suit the estimation of the file size before we want to run the
    // test.

    // TODO: Implement file hole support!
    //v->updates++;
}

void vsize_write(vsize_s *v, void *vfd, const char *path, const int bytes)
{
    vfd_s *vfd_ = vfd;
    vfd_->offset += bytes;

    if (v->vsize < vfd_->offset)
        v->vsize = vfd_->offset;

    v->updates++;
}

void vsize_mkdir(vsize_s *v, const char *path)
{
    if (v->updates == 0) {
        init_parent_dir(v, path);
        _Set_dir(v);
        v->updates++;
    }
}

void vsize_rmdir(vsize_s *v, const char *path)
{
    if (v->updates == 0) {
        init_parent_dir(v, path);
        _Set_required(v);
        _Set_dir(v);
        v->updates++;
    }
}

void vsize_unlink(vsize_s *v, const char *path)
{
    if (v->updates == 0) {
        init_parent_dir(v, path);
        _Set_required(v);
        if (!v->is_dir) {
            _Set_file(v);
            _Set_unsure(v);
        }
        v->updates++;
    }
}
