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
                size_t n_bytes = m_bits >> 3;
                if (n_bytes % CACHELINE_SIZE == 0 && (m_bits & 0x7) != 0) n_bytes++;

                m_memory_usage = sf_aligned_alloc(CACHELINE_SIZE, n_bytes, &m_data);
                memset(m_data, 0, m_memory_usage);
            }
        }

        // copy constructor
        BitArray(const BitArray& orig) : m_bits(orig.m_bits), m_data(nullptr) {
            m_memory_usage = sf_aligned_alloc(CACHELINE_SIZE, orig.m_memory_usage, &m_data);
            memcpy(m_data, orig.m_data, memory_usage());
        }

        // copy assignment operators
        // move & copy operator based on https://codereview.stackexchange.com/a/211253
        BitArray& operator=(const BitArray& other) noexcept {
            BitArray temp(other);
            swap(temp);
            return *this;
        }

        // move constructor
        BitArray(BitArray&& move_me) noexcept : m_bits(move_me.m_bits), m_memory_usage(move_me.m_memory_usage),
                                                m_data(move_me.m_data) {
            move_me.m_data = nullptr;
        }

        // move assignment operator
        BitArray& operator=(BitArray&& other) noexcept {
            swap(other);
            return *this;
        }

        virtual ~BitArray() {
            free(m_data);
        }

        // if out of bounds, returns false
        [[nodiscard]] inline bool is_set(size_t bit) const {
            if (bit >= m_bits) return false;
            return static_cast<char>(m_data[bit >> 3]) & (1 << (bit & 7));
        }

        // returns 0 if out of bounds
        inline int set(size_t bit) {
            if (bit >= m_bits) return 0;
            m_data[bit >> 3] |= static_cast<std::byte>(1) << (bit & 7);
            return 1;
        }

        // returns 0 if out of bounds
        inline int unset(size_t bit) {
            if (bit >= m_bits) return 0;
            m_data[bit >> 3] &= ~(static_cast<std::byte>(1) << (bit & 7));
            return 1;
        }

        inline void clear() {
            memset(m_data, 0, m_memory_usage);
        }

        [[nodiscard]] inline size_t memory_usage() const {
            return m_memory_usage;
        }

        [[nodiscard]] inline size_t size() const {
            return m_bits;
        }

        void swap(BitArray& other) noexcept {
            std::swap(other.m_bits, m_bits);
            std::swap(other.m_memory_usage, m_memory_usage);
            std::swap(other.m_data, m_data);
        }

    private:
        size_t m_bits;
        size_t m_memory_usage;
        std::byte* m_data;
    };
}
