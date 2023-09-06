/*
 * include/ds/BitArray.h
 *
 * Copyright (C) 2023 Dong Xie <dongx@psu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 */
#pragma once

#include <cstdlib>
#include <memory>
#include <cstring>

#include "psu-util/alignment.h"

namespace psudb {

/*
 *  An uncompressed bit-array type supporting setting, unsetting, and checking the
 *  values of individual bits in a predefined, fixed-size sequence. Allocated memory
 *  is cache aligned and initially set to 0.
 */
class BitArray {
public:
    /*
     * Construct a new bit array with bits number of bits available, initially set to
     * a default value of 0.
     */
    BitArray(size_t bits): m_bits(bits), m_data(nullptr) {
        if (m_bits > 0) {
            size_t n_bytes = (m_bits >> 3) << 3;
            m_data = sf_aligned_alloc(CACHELINE_SIZE, &n_bytes);
            memset(m_data, 0, n_bytes);
        }
    }

    ~BitArray() {
        if (m_data) free(m_data);
    }

    bool is_set(size_t bit) {
        if (bit >= m_bits) return false;
        return m_data[bit >> 3] & (1 << (bit & 7));
    }

    int set(size_t bit) {
        if (bit >= m_bits) return 0;
        m_data[bit >> 3] |= ((char) 1 << (bit & 7));
        return 1;
    }

    int unset(size_t bit) {
        if (bit >= m_bits) return 0;
        m_data[bit >> 3] &= ~((char) 1 << (bit & 7));
        return 1;
    }

    void clear() {
        memset(m_data, 0, (m_bits >> 3) << 3);
    }

    size_t memory_usage() {
        return m_bits >> 3;
    }

    size_t size() {
        return m_bits;
    }
    
private:
    size_t m_bits;
    char* m_data;
};

}
