/*
 * include/util/zbuff.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *                    Dong Xie <dongx@psu.edu>
 *
 * All rights reserved. Published under the Modified BSD License.
 *
 */
#pragma once

#include "util/alignment.h"
namespace psudb {

// The number of bytes of zeroes available in ZEROBUF. Will be
// a multiple of the parm::PAGE_SIZE.
constexpr size_t ZEROBUFF_SIZE = 8 * PAGE_SIZE;

// A large, preallocated, buffer of zeroes used for pre-allocation
// of pages in a file.
alignas(SECTOR_SIZE) const char ZEROBUFF[ZEROBUFF_SIZE] = {0};

}