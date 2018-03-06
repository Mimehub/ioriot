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

#ifndef RANGES_H
#define RANGES_H

#include "../defaults.h"

#include "btree.h"

/**
 * @brief This represents a range
 */
typedef struct range_s_ {
    long from;
    long to;
} range_s;

/**
 * @brief This represents a list of ranges
 *
 * A range is used to represent data ranges of a file with holes. Whereas
 * not all regions of a file have actually allocated data blocks.
 */
typedef struct ranges_s_ {
    btree_s *btree;
} ranges_s;

/**
 * @brief Creates a new range object
 *
 * @return A new ranges object
 * @param from Range start offset
 * @param to Range stop offset
 */
range_s* range_new(const long from, const long to);

/**
 * @brief Destroys the given range object
 *
 * @param r The range object to be destroyed
 */
void range_destroy(range_s* r);


/**
 * @brief Creates a new ranges object
 *
 * @return A new ranges object
 */
ranges_s* ranges_new();

/**
 * @brief Destroys the given ranges object
 *
 * @param r The ranges object to be destroyed
 */
void ranges_destroy(ranges_s* r);

/**
 * @brief Ensures that a given range is valid
 *
 * @param r The range object
 * @param from Range start offset
 * @param to Range stop offset
 */
void range_ensure(ranges_s* r, long from, long to);

/**
 * @brief Prints the range
 *
 * @param r The range object to be printed
 */
void ranges_print(ranges_s* r);

/**
 * @brief Unit testing the range object
 */
void ranges_test(void);

#endif // RANGES_H
