/*
 * include/util/zbuff.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *                    Dong Xie <dongx@psu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 */
#pragma once

#include "psu-util/alignment.h"

namespace psudb {

/*
 * The number of bytes of zeroes available in ZEROBUFF. Will be
 * a multiple of the PAGE_SIZE.
 */
constexpr size_t ZEROBUFF_SIZE = 8 * PAGE_SIZE;

/*
 * A large sector aligned buffer of zeroes. Useful for initializing
 * pages in a file.
 */
alignas(SECTOR_SIZE) const char ZEROBUFF[ZEROBUFF_SIZE] = {0};

}
