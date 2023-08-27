/*
 * include/util/alignment.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *                    Dong Xie <dongx@psu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 */
#pragma once

#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace psudb {

/* 
 * The correct quantity for use in alignment of buffers to 
 * be compatible with O_DIRECT 
 */
const size_t SECTOR_SIZE = 512;

/*
 * The standard sized block of data (in bytes) for use in IO 
 * operations.
 */
const size_t PAGE_SIZE = 4096;

/*
 * The size of a cacheline
 */
const size_t CACHELINE_SIZE = 64;

/*
 * Returns the (possibly larger) value of size after it has
 * been adjusted to be a multiple of alignment. Necessary for
 * aligned allocations, etc.
 */
size_t TYPEALIGN(size_t alignment, size_t size) {
    return (((size_t) (size) + ((alignment) - 1)) & ~((size_t) ((alignment) - 1)));
}

#define SHORTALIGN(LEN)         TYPEALIGN(2, (LEN))
#define INTALIGN(LEN)           TYPEALIGN(4, (LEN))
#define LONGALIGN(LEN)          TYPEALIGN(8, (LEN))
#define DOUBLEALIGN(LEN)        TYPEALIGN(8, (LEN))
#define MAXALIGN(LEN)           TYPEALIGN(8, (LEN))
#define CACHELINEALIGN(LEN)     TYPEALIGN(CACHELINE_SIZE, (LEN))
#define MAXALIGN_OF             8

/*
 * Returns a pointer to the idx'th page contained within a multi-page
 * buffer. buffer must be page aligned, and idx must be less than the
 * number of pages within the buffer, or the result is undefined.
 */
static inline std::byte *get_page(std::byte *buffer, size_t idx) {
    return buffer + (idx * PAGE_SIZE);
}

/*
 * A safe aligned allocation function. Automatically pads the
 * size to be a multiple of alignment prior to allocating memory.
 *
 * This function will never return nullptr--if a memory allocation
 * fails, it will write an error message to stderr and exit the program.
 */
static inline std::byte *sf_aligned_alloc(size_t alignment, size_t size) {
    size_t p_size = TYPEALIGN(alignment, size);

    char *alloc = (char *) std::aligned_alloc(alignment, p_size);
    if (alloc == nullptr) {
        fprintf(stderr, "[E]: Memory allocation failed; out of memory\n");
        exit(EXIT_FAILURE);
    }

    return alloc;
}

/*
 * A safe aligned allocation function. Automatically pads the cnt*size
 * to be a multiple of alignment prior to allocating memory.  The
 * returned memory will be zeroed.
 *
 * This function will never return nullptr--if a memory allocation
 * fails, it will write an error message to stderr and exit the program.
 */
static inline std::byte *sf_aligned_calloc(size_t alignment, size_t cnt, 
                                           size_t size) {
    size_t p_size = TYPEALIGN(alignment, size*cnt);

    char *alloc = (char *) std::aligned_alloc(alignment, p_size);
    if (alloc == nullptr) {
        fprintf(stderr, "[E]: Memory allocation failed; out of memory\n");
        exit(EXIT_FAILURE);
    }

    memset(alloc, 0, p_size);

    return alloc;
}

}
