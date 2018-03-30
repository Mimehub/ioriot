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

#include "utils.h"

#include <sys/resource.h>
#include <sys/time.h>

void* notnull(void *p, char *file, int line, int count)
{
    if (p == NULL) {
        Errno("%s:%d count:%d Could not allocate memory", file, line, count);
    }
    return p;
}


FILE* fnotnull(FILE *fd, const char *path, char *file, int line)
{
    if (fd == NULL) {
        Errno("%s:%d Could not open file '%s'", file, line, path);
    }
    return fd;
}

void* mmapok(void *p, char *file, int line)
{
    if (p == MAP_FAILED) {
        Errno("%s:%d: Mmap failed", file, line);
    }
    return p;
}

char* strtok2_r(char *str, char *delim, char **saveptr)
{
    int len = strlen(delim);

    if (str == NULL)
        str = *saveptr;

    char *next = strstr(str, delim);
    if (next) {
        next[0] = '\0';
        for (int i = 0; i < len; ++i)
            next++;
        *saveptr = next;
        return str;
    }

    return NULL;
}

void chreplace(char *str, char replace, char with)
{
    for (int i = 0; ; ++i) {
        if (str[i] == '\0')
            break;
        if (str[i] == replace)
            str[i] = with;
    }
}

void strunquote(char *str)
{
    int len = strlen(str);

    if (str[0] == '"') {
        if (str[len-1] == '"')
            str[len-1] = '\0';
        for (int i = 1; i < len; ++i)
            str[i-1] = str[i];
    }
}

void set_limits_drop_root(const char *user)
{
    if (getuid() == 0) {
        struct rlimit rl;
        rl.rlim_cur = rl.rlim_max = SET_RLIMIT_NOFILE;
        if (0 != setrlimit(RLIMIT_NOFILE, &rl)) {
            Errno("Could not set RLIMIT_NOFILE to '%lld'!",
                    (long long) SET_RLIMIT_NOFILE)
        }
        rl.rlim_cur = rl.rlim_max = SET_RLIMIT_NPROC;
        if (0 != setrlimit(RLIMIT_NPROC, &rl)) {
            Errno("Could not set RLIMIT_NPROC to '%lld'!",
                    (long long) SET_RLIMIT_NPROC)
        }

        if (!Eq("root", user)) {
            Put("Dropping root privileges to user '%s'", user);
            struct passwd *pw = getpwnam(user);

            /* process is running as root, drop privileges */
            if (setgid(pw->pw_gid) != 0) {
                Errno("Unable to drop group privileges!");
            }
            if (setuid(pw->pw_uid) != 0) {
                Errno("Unable to drop user privileges!");
            }
        }
    }

    /*
       getrlimit(RLIMIT_NOFILE, &rl);
       Put("Max open files: '%lld'", (long long) rl.rlim_cur);
       getrlimit(RLIMIT_NPROC, &rl);
       Put("Max open processes : '%lld'", (long long) rl.rlim_cur);
       */
}

void get_loadavg_s(char *readbuf)
{
    FILE *fp = Fopen("/proc/loadavg", "r");
    fgets(readbuf, 128, fp);
    char *pos = strchr(readbuf, ' ');
    pos[0] = '\0';
    fclose(fp);
}

double get_loadavg()
{
    // Not thread safe, but multi processing safe
    static char buf[128];
    get_loadavg_s(buf);

    return atof(buf);
}

bool is_number(char *str)
{
    for (int i = 0; ; ++i) {
        if (str[i] == '\0')
            return true;
        if (isdigit(str[i]) == 0 && str[i] != '-')
            return false;
    }

    return true;
}

void start_pthread(pthread_t *thread, void*(*cb)(void*), void *data)
{
    int rc = pthread_create(thread, NULL, cb, data);

    switch (rc) {
        case 0:
            break;
        case EAGAIN:
            Error("Out of resources while creating pthread (%d)", rc);
            break;
        case EINVAL:
            Error("Ivalid settings while creating pthread (%d)", rc);
            break;
        case EPERM:
            Error("No permissions to configure pthread (%d)", rc);
        default:
            Error("Unknown error while creating pthread (%d)", rc);
            break;
    }
}

void utils_test(void)
{
    if (getuid() == 0) {
        set_limits_drop_root("nobody");
        struct rlimit rl;

        getrlimit(RLIMIT_NOFILE, &rl);
        assert(rl.rlim_cur == SET_RLIMIT_NOFILE);
        assert(rl.rlim_max == SET_RLIMIT_NOFILE);

        getrlimit(RLIMIT_NPROC, &rl);
        assert(rl.rlim_cur == SET_RLIMIT_NPROC);
        assert(rl.rlim_max == SET_RLIMIT_NPROC);
    }
}
