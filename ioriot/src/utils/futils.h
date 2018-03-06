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

#ifndef FUTILS_H
#define FUTILS_H

#include "../defaults.h"

/**
 * @brief Thread safe version of dirname()
 *
 * @param path The full file path
 * @return The directory path
 */
char* dirname_r(char *path);

/**
 * @brief Ensures that a file exists
 *
 * @param path The file path
 * @param num_dirs_created Holds a count of how many sub dirs have been created
 * @return -1 on error, 0 on success.
 */
int ensure_file_exists(char *path, long *num_dirs_created);

/**
 * @brief Checks whether path exists
 *
 * @param path The path
 * @return true if the path exists
 */
bool exists(const char *path);

/**
 * @brief Check if path is a directory
 *
 * @param path The directory path
 * @return true if the path is a directory, false otherwise
 */
bool is_dir(const char *path);

/**
 * @brief Check if path is a regular file
 *
 * @param path The file path
 * @return true if the file at path is a regular fike
 */
bool is_reg(const char *path);

/**
 * @brief Create a directory recursively
 *
 * @param path The directory path
 * @param mode The mode
 * @param num_dirs_created Counts how many directories have been created
 * @return -1 on error
 */
int mkdir_p(const char *path, mode_t mode, long *num_dirs_created);

/**
 * @brief Appends data to a file
 *
 * @param path The file path
 * @param bytes The amount of bytes
 */
void append_random_to_file(char *path, unsigned long bytes);

/**
 * @brief Ensures that a directory exists
 *
 * @param path The directory path
 * @return The amount of directories created (including parent directories)
 */
long ensure_dir_exists(const char *path);

/**
 * @brief Ensures that a parent directory exists
 *
 * @param path The directory path
 */
void ensure_parent_dir_exists(const char *path);

/**
 * @brief Ensures that a directory is empty
 *
 * @param path The directory path
 */
void ensure_dir_empty(const char *path);

/**
 * @brief Loading a file into the file system cache
 *
 * @param file The path to the file
 */
void cache_file(const char *file);

/**
 * @brief Drop all Linux caches
 *
 * This function drops all Linux caches, which includes all file
 * system caches.
 */
void drop_caches(void);

/**
 * @brief Changes owner of a path
 *
 * Terminates the process with an error message if failed.
 *
 * @param user The new owner
 * @param path The path
 */
void chown_path(const char *user, const char *path);

/**
 * @brief Retrieves the absolute path of a given path
 *
 * @param path The path
 * @return The absolute path. It must be freed manually.
 */
char *absolute_path(const char *path);

#endif // FUTILS_H
