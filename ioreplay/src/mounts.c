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

#include "mounts.h"

#include "utils/futils.h"

#define _PATH_INSERT "/.ioreplay/"
#define _PATH_INSERT_LEN 11 // strlen of _PATH_INSERT

void mounts_read(mounts_s *m)
{
    char *mounts = "/proc/mounts";
    size_t len = 0;
    char *line = NULL;
    char *saveptr = NULL;

    Put("Reading '%s'", mounts);

    FILE *fp = Fopen(mounts, "r");
    Out("Adding supported file systems to replay paths:");

    while (getline(&line, &len, fp) != -1) {
        bool ignore = true;

        char *dev = strtok_r(line, " ", &saveptr);
        if (dev == NULL) {
            Error("Could not parse device from %s", mounts);
        }

        char *mp = strtok_r(NULL, " ", &saveptr);
        if (mp == NULL) {
            Error("Could not parse mountpoint from %s", mounts);
        }

        char *fs = strtok_r(NULL, " ", &saveptr);
        if (fs == NULL) {
            Error("Could not parse file system from %s", mounts);
        }
#ifdef MP_DEBUG
        Debug("fs:%s", fs);
#endif
        // TODO: Make file system types configurable
        if (Eq(fs, "ext2")) {
            ignore = false;
        } else if (Eq(fs, "ext5")) {
            ignore = false;
        } else if (Eq(fs, "ext4")) {
            ignore = false;
        } else if (Eq(fs, "xfs")) {
            ignore = false;
        } else if (Eq(fs, "zfs")) {
            ignore = false;
        } else if (Eq(fs, "btrfs")) {
            ignore = false;
        }

        if (ignore) {
            if (strcmp(mp, "/") != 0) {
                m->ignore_mps[m->ignore_count] = Clone(mp);
                m->ignore_count++;
            }

        } else if (m->count >= MAX_MOUNTPOINTS) {
            Error("Exceeded max mount points: %d\n", m->count);

        } else {
            Out(" %s (%s)", mp, fs);
            m->mps[m->count] = Clone(mp);
            m->lengths[m->count] = strlen(mp);
            m->count++;
        }
    }

    fclose(fp);
    Out("\n");
}

mounts_s *mounts_new(options_s *opts)
{
    mounts_s *m = Malloc(mounts_s);

    m->opts = opts;
    m->count = 0;
    m->ignore_count = 0;
    mounts_read(m);

    return m;
}

void mounts_destroy(mounts_s *m)
{
    if (!m)
        return;
    for (int i = 0; i < m->count; i++)
        free(m->mps[i]);
    free(m);
}

void mounts_trash(mounts_s *m)
{
    options_s *opts = m->opts;
    drop_root(opts->user);
    Put("Moving all old files to trash (of previous tests)...");

    struct timeval tv;
    gettimeofday(&tv, NULL);

    char *wd_path = NULL;
    asprintf(&wd_path, "%s/%s", opts->wd_base, opts->name);

    char *trash_path = NULL;
    asprintf(&trash_path, "%s/.trash/%ld", opts->wd_base, tv.tv_sec);

    if (is_dir(wd_path)) {
        ensure_dir_exists(trash_path);
        chown_path(opts->user, trash_path);
        if (rename(wd_path, trash_path)) {
            Errno("Could not move '%s' to '%s'", wd_path, trash_path);
        }
    }
    free(wd_path);
    free(trash_path);

    for (int i = 0; i < m->count; i++) {
        char *mp = m->mps[i];
        char *path = NULL;
        asprintf(&path, "%s/%s/%s", mp, _PATH_INSERT, opts->name);
        asprintf(&trash_path, "%s/%s/.trash/%ld",
                 mp, _PATH_INSERT, tv.tv_sec);

        if (is_dir(path)) {
            ensure_dir_exists(trash_path);
            chown_path(opts->user, trash_path);
            if (rename(path, trash_path)) {
                Errno("Could not move '%s' to '%s'", path, trash_path);
            }
        }

        free(path);
        free(trash_path);
    }

    Put("Done trashing!");
    Put("Once the drives fill up you may want to purge old data (-P)");
}

void mounts_purge(mounts_s *m)
{
    options_s *opts = m->opts;
    drop_root(opts->user);

    Out("Purging all data from the following directories:");

    int active_purgers = 0, max_purgers = 16;
    if (opts->num_workers > max_purgers)
        max_purgers = opts->num_workers;

    char *purge_path = NULL;
    asprintf(&purge_path, "%s", opts->wd_base);
    if (is_dir(purge_path)) {
        Out(" %s", purge_path);
        pid_t pid = fork();

        if (pid == 0) {
            ensure_dir_empty(purge_path);
            free(purge_path);
            exit(0);

        } else if (pid < 0) {
            Errno("\nUnable to create cleaner process! :'-(");
        }
        active_purgers++;
    }
    free(purge_path);

    int cleaner_status = SUCCESS;

    for (int i = 0; i < m->count; i++) {
        char *mp = m->mps[i];
        char *purge_path = NULL;
        asprintf(&purge_path, "%s/%s", mp, _PATH_INSERT);

        if (is_dir(purge_path)) {
            if (active_purgers+1 >= max_purgers) {
                wait(&cleaner_status);
                active_purgers--;
            }

            // TODO: Use threading model same way as in init/init.c
            pid_t pid = fork();
            if (pid == 0) {
                Out(" %s", purge_path);
                ensure_dir_empty(purge_path);
                free(purge_path);
                exit(0);
            } else if (pid < 0) {
                Errno("Unable to create cleaner process! :'-(");
            }
            active_purgers++;
        }
        free(purge_path);
    }

    while (wait(&cleaner_status) > 0)
        active_purgers--;
    Put("\nCleaning done!");
}

void mounts_init(mounts_s *m)
{
    options_s *opts = m->opts;
    char *wd_path = NULL;
    asprintf(&wd_path, "%s/%s", opts->wd_base, opts->name);
    ensure_dir_exists(wd_path);
    chown_path(opts->user, opts->wd_base);
    chown_path(opts->user, wd_path);

    if (chdir(wd_path)) {
        Errno("Could not chdir into '%s'!", wd_path);

    } else {
        Put("Chdir into '%s'", wd_path);
    }

    free(wd_path);

    for (int i = 0; i < m->count; i++) {
        char *mp = m->mps[i];
        char *path = NULL;

        // Create .ioreplay/ directory on MP
        asprintf(&path, "%s/%s", mp, _PATH_INSERT);
        ensure_dir_exists(path);
        chown_path(m->opts->user, path);
        free(path);
        path = NULL;

        // Create .ioreplay/NAME directory on MP
        asprintf(&path, "%s/%s/%s", mp, _PATH_INSERT, opts->name);
        ensure_dir_exists(path);
        chown_path(m->opts->user, path);
        free(path);
    }
}

bool mounts_ignore_path(mounts_s *m, const char *path)
{
    // CentOS 7 specific, ignore temp namespace mounts!
    char *pos = strstr(path, "/tmp/namespace-");
    if (pos == path)
        return true;

    // iterate backwards through all mount points.
    for (int i = m->ignore_count-1; i >= 0; --i) {
        char *mountpoint = m->ignore_mps[i];
        pos = strstr(path, mountpoint);
        // Ignore this path as it is in the ignore mp list
        if (pos == path)
            return true;
    }

    return false;
}

bool mounts_transform_path(mounts_s *m, const char *name,
                           char *path, char **path_r)
{
    char *tmp = NULL;
#ifdef DEBUG_TRANSFORM_PATH
    char *original_path = path;
#endif
    bool line_ok = true;

    // First figure out whether there are '..' in any paths. If so we have to
    // tokenize the path and remove '..'. Example:
    // transform '/foo/bar/../' into '/foo/'.
    // Also remove double '/' from paths.

    if (strstr(path, "..") || strstr(path, "//")) {
        // tmp will be freed under label 'cleanup' at end of function.
        tmp = Calloc(strlen(path)+1, char);

        // stack to put the tokens on
        stack_s *s = stack_new();

        // we need a copy of the path, so we can tokenize it into the stack
        char* clone = Clone(path);

        char *saveptr = NULL;
        char *tok = strtok_r(clone, "/", &saveptr);

        // Add each part of the path to the stack.
        while (tok) {
            if (strcmp(tok, "..") == 0) {
                stack_pop(s);
            } else {
                stack_push(s, tok);
            }
            tok = strtok_r(NULL, "/", &saveptr);
        }

        if (stack_is_empty(s)) {
            strcpy(tmp, ".");

        } else {
            s = stack_new_reverse_from(s);
            strcpy(tmp, "/");
            strcat(tmp, (char*)stack_pop(s));

            while(!stack_is_empty(s)) {
                strcat(tmp, "/");
                strcat(tmp, (char*)stack_pop(s));
            }
        }

        stack_destroy(s);
        free(clone);

        // This is the path without '..' and '//' (and '///' ... etc')
        path = tmp;
    }

    // Now heck whether the path is on a supported file system. If not, ignore!
    if (mounts_ignore_path(m, path)) {
        line_ok = false;
        goto cleanup;
    }

    // So the path is on a valid mount point! Now we need to insert
    // .ioreplay/NAME to each mount point, e.g. /usr/local/.ioreplay/NAME/...

    // Iterate backwards through all mount points.
    for (int i = m->count-1; i >= 0; --i) {
        char *mountpoint = m->mps[i];
        int mp_len = m->lengths[i];

        if (strncmp(path, mountpoint, mp_len) == 0) {
            // Found a path to replace
            // Now insert .ioreplay/NAME/ into the file path.
            *path_r = Calloc(strlen(path) + strlen(name)+1
                             + _PATH_INSERT_LEN+1, char);

            if (strcmp(mountpoint, "/") == 0) {
                // Root path
                strcpy(*path_r, _PATH_INSERT);
                strcat(*path_r, name);
                strcat(*path_r, path);

            } else {
                strcpy(*path_r, mountpoint);
                strcat(*path_r, _PATH_INSERT);
                strcat(*path_r, name);
                char *pos = path;
                pos += mp_len * (int) sizeof(char);
                strcat(*path_r, pos);
            }

            goto cleanup;
        }
    }

    if (tmp)
        free(tmp);

    return line_ok;

cleanup:
#ifdef DEBUG_TRANSFORM_PATH
    Debug("Transform path '%s' -> '%s' -> '%s'", original_path, path, *path_r);
#endif
    if (tmp)
        free(tmp);

    return line_ok;
}

int mounts_get_mountnumber(mounts_s *m, const char *path)
{
    for (int i = m->count-1; i >= 0; --i) {
        char *mountpoint = m->mps[i];
        int mp_len = m->lengths[i];

        if (strncmp(path, mountpoint, mp_len) == 0)
            return i;
    }

    return 0;
}
