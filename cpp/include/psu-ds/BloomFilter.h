/*
 * include/ds/BloomFilter.h
 *
 * Copyright (C) 2023 Dong Xie <dongx@psu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 */
#pragma once

#include <cmath>
#include <gsl/gsl_rng.h>

#include "psu-ds/BitArray.h"
#include "psu-util/alignment.h"
#include "psu-util/hash.h"

namespace psudb {

/*
 * A generic implementation of a Bloom Filter for tests of approximate set 
 * membership. The K template parameter specifies the type of record to 
 * be inserted--though is mainly used for getting its size. The underlying
 * data is manipulated as a raw byte array for the most part.
 *
 * For more information, see
 *   [1] Bloom, Burton H. (1970), "Space/Time Trade-offs in Hash Coding 
 *   with Allowable Errors", Communications of the ACM, 13 (7): 422–426
 */

template <typename K>
class BloomFilter {
public:
    /*
     * Create a bloom filter with n_bits maximum space utilization and 
     * k hash functions. 
     */
    BloomFilter(size_t n_bits, size_t k)
    : m_n_bits(n_bits), m_n_salts(k), m_bitarray(n_bits) {
        gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);

        salt = (uint16_t*) sf_aligned_alloc(CACHELINE_SIZE, k *sizeof(uint16_t));
        for (size_t i = 0;  i < k; ++i) {
            salt[i] = (uint16_t) gsl_rng_uniform_int(rng, 1 << 16);
        }

        gsl_rng_free(rng);
    }

    /*
     * Create a bloom filter to store up to n keys with a given max_fpr
     * and k hash functions. The size of the filter will be automatically
     * calculated based on the input parameters.
     */ 
    BloomFilter(double max_fpr, size_t n, size_t k)
    : BloomFilter((size_t)(-(double) (k * n) / std::log(1.0 - std::pow(max_fpr, 1.0 / k))), k) {}

    ~BloomFilter() {
        if (salt) free(salt);
    }

    int insert(const K& key) {
        if (m_bitarray.size() == 0) return 0;

        for (size_t i = 0; i < m_n_salts; ++i) {
            m_bitarray.set(hash_bytes_with_salt((const char*)&key, sizeof(K), salt[i]) % m_n_bits);
        }

        return 1;
    }

    bool lookup(const K& key) {
        if (m_bitarray.size() == 0) return false;
        for (size_t i = 0; i < m_n_salts; ++i) {
            if (!m_bitarray.is_set(hash_bytes_with_salt((const char*)&key, sizeof(K), salt[i]) % m_n_bits))
                return false;
        }

        return true;
    }

    void clear() {
        m_bitarray.clear();
    }

    size_t memory_usage() {
        return this->m_bitarray.memory_usage();
    }
private: 
    size_t m_n_bits;
    size_t m_n_salts;
    uint16_t* salt;

    BitArray m_bitarray;
};

}
