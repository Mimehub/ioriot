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

#include "futils.h"

#include <libgen.h>
#include <pwd.h>
#include <unistd.h>
#include <limits.h>

#include "../macros.h"

void _write_random_to_stream(FILE *fp, unsigned long bytes) {
    char *buf = NULL;
    int max_chunk = 50000000; // 50 mebibyetes

    for (;;) {
        if (bytes > max_chunk) {
            if (!buf)
                buf = Calloc(max_chunk+1, char);

            Fill_with_stuff(buf, max_chunk);
            buf[max_chunk] = '\0';
            fprintf(fp, "%s", buf);
            bytes -= max_chunk;

            // Print out a dot every time we wrote 'much' data to a file
            Out(".");

        } else {
            if (!buf)
                buf = Calloc(bytes+1, char);

            Fill_with_stuff(buf, bytes);
            buf[bytes] = '\0';
            fprintf(fp, "%s", buf);

            break;
        }
    }

    if (buf)
        free(buf);
}

void append_random_to_file(char *path, unsigned long bytes)
{
    FILE *fp = Fopen(path, "a");
    if (fp) {
        _write_random_to_stream(fp, bytes);
        fclose(fp);
    }
}

void write_random_to_file(char *path, unsigned long bytes, off_t offset)
{
    FILE *fp = Fopen(path, "w");
    if (fp) {
        fseek(fp, offset, SEEK_SET);
        _write_random_to_stream(fp, bytes);
        fclose(fp);
    }
}

long ensure_dir_exists(const char *path)
{
    long num_dirs_created = 0;
    int ret = mkdir_p(path, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH,
                      &num_dirs_created);
    if (ret != 0) {
        Errno("Could not create dir '%s'", path);
    }

    return num_dirs_created;
}

void ensure_parent_dir_exists(const char *path)
{
    char *clone = Clone(path);
    char *parent = dirname(clone);

    int ret = mkdir_p(parent, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH, NULL);
    if (ret != 0) {
        Errno("Could not create dir %s", parent);
    }

    free(clone);
}

void ensure_dir_empty(const char *path)
{
    DIR *dh = opendir(path);

    if (!dh) {
        Errno("Unable to empty %s", path);
    }

    struct dirent *de;

    while ((de = readdir(dh))) {
        if (0 == strcmp(de->d_name, ".") ||
            0 == strcmp(de->d_name, ".."))
            continue;

        char *absolute;
        asprintf(&absolute, "%s/%s", path, de->d_name);

        if (is_dir(absolute))
            ensure_dir_empty(absolute);

        if (remove(absolute) == -1)
            // Don't throw an error if there is no such file or directory
            if (errno != 2) {
                Errno("Unable to remove %s", absolute);
            }

        free(absolute);
    }

    closedir(dh);
}

int ensure_file_exists(char *path, long *num_dirs_created)
{
    if (is_reg(path))
        return SUCCESS;

    char *dirname = dirname_r(Clone(path));
    *num_dirs_created += ensure_dir_exists(dirname);
    free(dirname);

    FILE *fp = fopen(path, "a");
    if (fp) {
        // We only need some data, less than 1 block in size, this is answer:
        fprintf(fp, "42");
        fclose(fp);
        return SUCCESS;
    }

    return ERROR;
}

char* dirname_r(char *path)
{
    int len = strlen(path);
    int has = 0;
    int i = len-1;

    if (strcmp(path, "..") == 0) {
        return path;
    }

    if (path[i] == '/') {
        // Root directory
        if (len == 1)
            return path;

        // Remove all trailing /
        for (; i >= 0; --i) {
            if (path[i] == '/') {
                path[i] = '\0';
                has = 1;
            } else {
                break;
            }
        }
    }

    // Find next /
    for (; i >= 0; --i) {
        if (path[i] == '/') {
            path[i] = '\0';
            has = 1;
            break;
        }
    }

    // If no /
    if (has == 0) {
        path[0] = '.';
        path[1] = '\0';
    }

    return path;
}

bool is_dir(const char *path)
{
    struct stat path_stat;
    if (stat(path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
        return true;
    return false;
}

bool is_reg(const char *path)
{
    struct stat path_stat;
    if (stat(path, &path_stat) == 0 && S_ISREG(path_stat.st_mode))
        return true;
    return false;
}

bool exists(const char *path)
{
    struct stat path_stat;
    if (stat(path, &path_stat) == 0)
        return true;
    return false;
}

int mkdir_p(const char *path, mode_t mode, long *num_dirs_created)
{
    int res = 0;

    if (is_dir(path))
        return 0;

    if (is_reg(path))
        unlink(path);

    char *top = dirname_r(Clone(path));
    if (0 != mkdir_p(top, mode, num_dirs_created))
        goto cleanup;

    if ((mkdir(path, mode) == -1) && (errno != EEXIST))
        res = -1;

    if (res != -1)
        *num_dirs_created = *num_dirs_created+1;

cleanup:
    free(top);

    return res;
}

void cache_file(const char *file)
{
    Out("Caching file %s... it can take a while", file);
    FILE *fd = Fopen(file, "r");
    char *line = NULL;
    size_t len = 0, read = 0;

    while ((read = getline(&line, &len, fd)) != -1);
    fclose(fd);
}

void drop_caches(void)
{
    Out("Dropping all Linux caches...");

    if (getuid() != 0) {
        Out("\n");
        Error("I need to be root to do this, aborting!");
    }

    // echo 3 > /proc/sys/vm/drop_caches
    char *drop_caches = "/proc/sys/vm/drop_caches";
    FILE *fd = Fopen(drop_caches, "w");
    fprintf(fd, "3");
    fclose(fd);

    Put("done");
}

void chown_path(const char *user, const char *path)
{
    struct passwd *pwd = getpwnam(user);
    if (!pwd) {
        Errno("Unable to retrieve information about system user %s!", user);
    }

    if (chown(path, pwd->pw_uid, -1) == -1) {
        Errno("Could not change ownership of '%s' to '%s'!", path, user);
    }
}

char *absolute_path(const char *path)
{
    if (path[0] == '/')
        return Clone(path);

    char cwd[MAX_LINE_LEN];
    getcwd(cwd, sizeof(char)*MAX_LINE_LEN);

    if (!getcwd(cwd, sizeof(cwd))) {
        Errno("Could not get current working directory");
    }

    char *absolute = NULL;
    if (-1 == asprintf(&absolute, "%s/%s", cwd, path)) {
        Error("Could not get absolute path of '%s'", path);
    }

    return absolute;
}
