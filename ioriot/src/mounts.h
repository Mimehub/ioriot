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

#ifndef MOUNTPOINTS_H
#define MOUNTPOINTS_H

#include "datas/stack.h"
#include "defaults.h"
#include "options.h"

#define MAX_MOUNTPOINTS 1024

/**
 * @brief Represents data parsed from /proc/mounts
 *
 * This is used to determine the file systems and the file system types
 * currently mounted on the Linux system. I/O replay only replays I/O
 * on specific file systems such as ext4 or xfs and will ignore any special
 * or pseudo file systems such as tmpfs, devfs, sysfs. It does not make sense
 * to replay I/O on these because there is actually no underlying block device
 * attached to these.
 *
 * A mounts object helps to determine whether a path relies on a valid file
 * system or not. All I/O operations on invalid file systems are being filtered
 * out!
 *
 * The mounts object also does more things such as purging temp test data from
 * the mountpoints etc...
 */
typedef struct mounts_s_ {
    int count; /**< The amount of mount points */
    char *mps[MAX_MOUNTPOINTS]; /**< The mp paths */
    int lengths[MAX_MOUNTPOINTS]; /**< The mp lenghts */
    int ignore_count; /**< The amount of ignored mount points */
    char *ignore_mps[MAX_MOUNTPOINTS]; /**< The ignored mp paths */
    options_s *opts; /**< A pointer to the options object */
} mounts_s;

/**
 * @brief Creates a new mounts object
 *
 * @param opts The options object
 * @return The new mounts object
 */
mounts_s *mounts_new(options_s *opts);

/**
 * @brief Destroys the mounts object
 *
 * @param m The mounts object
 */
void mounts_destroy(mounts_s *m);

/**
 * @brief moves all files within replay mounts to trash
 *
 * It moves all files of the .ioriot/NAME directories to
 * .ioriot/NAME.trashEPOCH directories for all available mount points.
 * It does the same for the working dorectory of the current test.
 *
 * @param m The responsible mounts object
 */
void mounts_trash(mounts_s *m);

/**
 * @brief Deletes all files within replay mounts
 *
 * It deletes all files from the .ioriot/ directories for all availabe
 * mount points. It also deletes the working directory of all tests. The
 * function forks one sub-process per mount point, so it is cleaning all drives
 * in parallel.
 *
 * It can take a significant amount of time to actually delete all these files.
 * That's why there is also a mounts_trash function, which will not delete the
 * files but move them to trash folders so they can be deleted at a later
 * point.
 *
 * @param m The responsible mounts object
 */
void mounts_purge(mounts_s *m);

/**
 * @brief Ensures all mounts have a .ioriot/NAME directory
 *
 * These directories are used by ioriot to run the I/O replay tests in.
 * The function also ensures to have the correct user permissions for these
 * directories.
 *
 * @param m The responsible mounts object
 */
void mounts_init(mounts_s *m);

/**
 * @brief Reads /proc/mounts to determine which mounts are available
 *
 * @param m The mounts object
 */
void mounts_read(mounts_s *m);

/**
 * @brief Determines whether a path should be ignored
 *
 * ioriot replays I/O only on known mount points of known
 * file system types. This function helps to determine whether
 * a path is on a valid mount point or not.
 *
 * @param m The responsible mounts object
 * @param path The path to check
 * @return true if path has to be ignored
 */
bool mounts_ignore_path(mounts_s *m, const char *path);

/**
 * @brief Inserts ./ioriot/NAME into a path
 *
 * This function inserts ./ioriot/NAME into a given file path.
 * The function also checks whether the path is on a supported replay
 * path or not. E.g. we want to ignore file systems such as devfs, sysfs,
 * procfs.. etc.
 *
 * @param m The responsible mountpoint object
 * @param name The name of the test
 * @param path The original path
 * @param path_r The tansformed path (has to be freed if not NULL)
 * @return False if this path is to be ignored
 */
bool mounts_transform_path(mounts_s *m, const char *name,
                           char *path, char **path_r);


/**
 * @brief Get's the mount point number of a path
 *
 * Used by init.c to determine which thread to use to initialise a file
 * or directory on a given path.
 *
 * @param m The responsible mountpoint object
 * @param path The file/directory path
 * @return The mountpoint number
 */
int mounts_get_mountnumber(mounts_s *m, const char *path);

#endif // MOUNTPOINTS_H
