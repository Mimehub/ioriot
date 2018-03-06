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

#ifndef CAPTURE_H
#define CAPTURE_H

#include "../defaults.h"
#include "../utils/futils.h"
#include "../options.h"

/**
 * @brief Captures I/O to a .capture file by using stap from SystemTap
 *
 * @param opts The options object
 * @return SUCCESS if everything went fine
 */
status_e capture_run(options_s *opts);

#endif // CAPTURE_H
