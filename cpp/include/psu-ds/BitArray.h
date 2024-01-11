/*
 * include/ds/BitArray.h
 *
 * Copyright (C) 2023 Dong Xie <dongx@psu.edu>
 *                    Max Norfolk <mnorfolk@psu.edu>
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
        explicit BitArray(size_t bits) : m_bits(bits), m_data(nullptr) {
            if (m_bits > 0) {
                size_t n_bytes = (m_bits >> 3);
                if (n_bytes == 0)
                    n_bytes = 1;
                m_data = sf_aligned_alloc(CACHELINE_SIZE, n_bytes);
                memset(m_data, 0, n_bytes);
            }
        }

        // copy constructor
        BitArray(const BitArray &orig) : m_bits(orig.m_bits), m_data(nullptr) {
            m_data = sf_aligned_alloc(CACHELINE_SIZE, memory_usage());
            memcpy(m_data, orig.m_data, memory_usage());
        }

        // copy assignment operators
        // move & copy operator based on https://codereview.stackexchange.com/a/211253
        BitArray &operator=(const BitArray &other) noexcept {
            BitArray temp(other);
            swap(temp);
            return *this;
        }

        // move constructor
        BitArray(BitArray &&move_me) noexcept : m_data(move_me.m_data), m_bits(move_me.m_bits) {
            move_me.m_data= nullptr;
        }

        // move assignment operator
        BitArray &operator=(BitArray &&other) noexcept {
            swap(other);
            return *this;
        }

        virtual ~BitArray() {
            free(m_data);
        }

        // if out of bounds, returns false
        [[nodiscard]] inline bool is_set(size_t bit) const {
            if (bit >= m_bits) return false;
            return ((char) m_data[bit >> 3]) & (1 << (bit & 7));
        }

        // returns 0 if out of bounds
        inline int set(size_t bit) {
            if (bit >= m_bits) return 0;
            m_data[bit >> 3] |= ((std::byte) 1 << (bit & 7));
            return 1;
        }

        // returns 0 if out of bounds
        inline int unset(size_t bit) {
            if (bit >= m_bits) return 0;
            m_data[bit >> 3] &= ~((std::byte) 1 << (bit & 7));
            return 1;
        }

        inline void clear() {
            memset(m_data, 0, (m_bits >> 3) << 3);
        }

        [[nodiscard]] inline size_t memory_usage() const {
            if (m_bits <= 8)
                return 1;
            return m_bits >> 3;
        }

        [[nodiscard]] inline size_t size() const {
            return m_bits;
        }

        void swap(BitArray& other){
            std::swap(other.m_bits, m_bits);
            std::swap(other.m_data, m_data);
        }

    private:
        size_t m_bits;
        std::byte *m_data;
    };
}
