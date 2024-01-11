#include "psu-util/alignment.h"

namespace psudb {

using std::byte;

/*
 * Represents a page offset within a specific file (physical or virtual)
 */
typedef uint32_t PageNum;

/*
 * Byte offset within a page. Also used for lengths of records, etc.,
 * within the codebase. size_t isn't necessary, as the maximum offset
 * is only PAGE_SIZE 
 */
typedef uint16_t PageOffset;

/*
 * A unique identifier for a frame within a buffer or cache.
 */
typedef int32_t FrameId;

/*
 * Invalid values for various IDs. Used to indicate
 * uninitialized values and error conditions.
 */ 
const PageNum INVALID_PNUM = 0;
const FrameId INVALID_FRID = -1;

/*
 * Maximum values for the various IO types
 */
const PageNum MAX_PNUM = UINT32_MAX;
const PageOffset MAX_POFF = PAGE_SIZE;
const FrameId MAX_FRID = INT32_MAX;


/*
 * Convert a Page Number to its associated byte offset within 
 * a file or contiguous buffer
 */
static inline off_t pnum_to_offset(PageNum pnum) {
    return pnum * PAGE_SIZE;
}


/*
 * Returns a pointer to the idx'th page contained within a multi-page
 * buffer. buffer must be page aligned, and idx must be less than the
 * number of pages within the buffer, or the result is undefined.
 */
static inline byte *get_page(byte *buffer, size_t idx) {
    return buffer + (idx * PAGE_SIZE);
}

}
