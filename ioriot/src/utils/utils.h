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

#ifndef UTILS_H
#define UTILS_H

// For asprintf in stdio.h
#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <pthread.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../macros.h"
#include "../defaults.h"

/**
 * @brief Check whether allocated memory is not NULL
 *
 * This function is used in conjunction with malloc() and co. It
 * introduces an extra sanity check whether the memory could be
 * allocated successfully or not. If not it will print out a error
 * message stating the position in the source code where the memory
 * allocation failed!
 *
 * @param p The pointer being checked
 * @param file The source file name the memory was allocated in
 * @param line The source line number the memory was allocated at
 * @param count The amount of memory being allocated
 * @return The pointer to the allocated memory
 */
void* notnull(void *p, char *file, int line, int count);

/**
 * @brief Check whether opened file handle is not NULL
 *
 * This function is used in conjunction with fopen(). It
 * introduces an extra sanity check whether the file could be
 * opened successfully or not. If not it will print out a error
 * message stating the position in the source code where the open
 * failed!
 *
 * @param fd The fd stream to be checked.
 * @param path The file path opened
 * @param file The source file name
 * @param line The source line number
 * @return The pointer to the allocated memory
 */
FILE* fnotnull(FILE *fd, const char *path, char *file, int line);

/**
 * @brief Check whether a return code is OK.
 *
 * This function is used in conjunction with asprintf() and co. It
 * introduces an extra sanity check whether the memory could be
 * allocated successfully or not. If not it will print out a error
 * message stating the position in the source code where the memory
 * allocation failed!
 *
 * @param not Code in case of an error.
 * @param num Return code of the function.
 * @param line The source line number the memory was allocated at
 * @param count The amount of memory being allocated
 */
void notnum(int not, int num, char *file, int line, int count);

/**
 * @brief Check whether allocated memory via mmap is not null
 *
 * This function is used in conjunction with mmap() and co. It
 * introduces an extra sanity check whether the memory could be
 * allocated successfully or not. If not it will print out a error
 * message stating the position in the source code where the memory
 * allocation failed!
 *
 * @param addr The pointer being checked
 * @param file The source file name the memory was allocated in
 * @param line The source line number the memory was allocated at
 * @return The pointer to the allocated memory
 */
void* mmapok(void *addr, char *file, int line);

/**
 * @brief A version of strtok_r supporting multi char delims
 *
 * @param str The input string
 * @param delim The multi-char delimiter
 * @param saveptr A temp storage location
 * @return The next match if != NULL
 */
char* strtok2_r(char *str, char *delim, char **saveptr);

/**
 * @brief Replaces a character with another one in a string
 *
 * @param str The input string
 * @param replace The character to be replaced
 * @param with The character to replace with
 */
void chreplace(char *str, char replace, char with);

/**
 * @brief Removes quotes from a string
 *
 * @param str The input sting
 */
void strunquote(char *str);

/**
 * @brief Set rlimits and drop root privileges
 *
 * This function firsts sets the user resource limits to SET_RLIMIT_NOFILE and
 * SET_RLIMIT_NPROC and then attempts to drop the root user to the specified
 * one.
 *
 * @param user The user to switch to
 */
void set_limits_drop_root(const char *user);

/**
 * @brief Retrieve current 1 min Linux load average
 *
 * @param readbuf The buffer to store the load average as a string
 */
void get_loadavg_s(char *readbuf);

/**
 * @brief Retrieve current 1 min Linux load average
 *
 * This function is not thread safe!
 *
 * @return The 1 minute load average of the system
 */
double get_loadavg();

/**
 * @brief Check whether a string represents a number
 *
 * @param str The input string
 * @return true if all characters of the input string are a digits
 */
bool is_number(char *str);

/**
 * @brief Wrapper around pthread_create
 *
 * The wrapper also checks whether the thread has been created successfully
 * or not! It will exit the process if not.
 *
 * @param thread The POSIX thread variable
 * @param cb The threadss start callback routine
 * @param data A data pointer passed to the thread.
 */
void start_pthread(pthread_t *thread, void*(*cb)(void*), void *data);

/**
 * @brief Testing various of the utilities
 */
void utils_test(void);

#endif // UTILS_H
