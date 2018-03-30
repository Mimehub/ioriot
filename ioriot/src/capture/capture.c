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

#include "capture.h"

#include <sys/utsname.h>

status_e capture_run(options_s *opts)
{
    int status = 0;
    struct utsname uts;

    // To make it nicer we should iterate over PATH instead
    char *staprun_paths[3] = {
        "/usr/bin/staprun",
        "/usr/local/bin/staprun",
        "/bin/staprun"
    };
    int num_staprun_paths = 3;

    if (0 != uname(&uts)) {
        Errno("Could not identify release of currently running Kernel!");
    }

    Put("Release of currently running Kernel: %s", uts.release);
    char modules_dir[128];
    sprintf(modules_dir, "/opt/ioriot/systemtap/%s", uts.release);
    Put("Changing directory to module path: %s/", modules_dir);

    if (0 != chdir(modules_dir)) {
        Errno("Could not change into '%s', please ensure that the compiled "
              "SystemTap modules correspond to the currently running Kernel "
              "and that these are installed properly!\n",
              modules_dir);
    }

    if (0 != access(opts->module, R_OK)) {
        Errno("Module '%s/%s' can't be read, please make sure that the "
              "SystemTap Kernel modules are installed!",
              modules_dir, opts->module);
    }

    char *staprun_path = NULL;
    for (int i = 0; i < num_staprun_paths; ++i) {
        if (0 == access(staprun_paths[i], X_OK)) {
            staprun_path = staprun_paths[i];
            //Put("SystemTap command path: %s", staprun_path);
            break;
        }
    }

    if (staprun_path == NULL) {
        Errno("Can't find 'staprun' command, please ensure to have the SystemTap "
              "runtime (usually package 'systemtap-runtime') installed!");
    }

    char staprun_command[128];
    if (opts->pid >= 0) {
        sprintf(staprun_command, "%s %s -v -o %s -x %d", staprun_path, opts->module,
                opts->capture_file, opts->pid);
    } else {
        sprintf(staprun_command, "%s %s -v -o %s", staprun_path, opts->module,
                opts->capture_file);
    }

    Out("NOTICE: It is good practise first to stop all processes, then to ");
    Out("start capturing, and then to start all processes again. The reason ");
    Out("is that processes may have already open file handles. In that case ");
    Out("I/O Riot would be unable to replay these! This may be improved ");
    Put("in a future release!");
    Put("To abort capturing now send Ctrl+C, otherwise wait 1h");
    Put("Capturing I/O via: '%s'", staprun_command);

    char buf[1024];
    FILE *fp;

    if ((fp = popen(staprun_command, "r")) == NULL) {
        Errno("Unable to invoke staprun command!");
    }
    while (fgets(buf, 1024, fp) != NULL)
        Out("stapio: %s", buf);

    if (0 != pclose(fp)) {
        Error("Problems invoking staprun command!");
    }

    return status;
}
