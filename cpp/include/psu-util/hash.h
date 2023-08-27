/*
 * include/util/hash.h
 *
 * Copyright (C) 2023 Dong Xie <dongx@psu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 */
#pragma once

#include <cstdlib>
#include <cstdint>

namespace psudb {

/*
 * A magic constant value that is used for the hash function. A prime
 * number with a good distribution of bits.
 */
const uint64_t kHashMagicNum1 = 40343;

/*
 * A magic constant value that is used for the hash function. A prime
 * number with a good distribution of bits.
 */
const uint64_t kHashMagicNum2 = 38299;

/*
 * rotr64 bit manipulation procedure--for use by hash and its related
 * functions.
 */
inline uint64_t rotr64(uint64_t x, size_t n) 
{
    return (((x) >> n) | ((x) << (64 - n)));
}

/*
 * Calculate the hash of an 8 byte value using rotr64. The quality of
 * the output is adjusted using a magic_number, two good options are
 * provided in this header as kHashMagicNum1 and kHashMagicNum2.
 */
inline uint64_t hash(uint64_t input, uint64_t magic_num=kHashMagicNum1) 
{
    uint64_t local_rand = input;
    uint64_t local_rand_hash = 8;
    local_rand_hash = magic_num * local_rand_hash
                      + ((local_rand) & 0xFFFF);

    local_rand_hash = magic_num * local_rand_hash
                      + ((local_rand >> 16) & 0xFFFF);

    local_rand_hash = magic_num * local_rand_hash
                      + ((local_rand >> 32) & 0xFFFF);

    local_rand_hash = magic_num * local_rand_hash
                      + (local_rand >> 48);

    local_rand_hash = magic_num * local_rand_hash;
    return rotr64(local_rand_hash, 43);
}

/*
 * Calculate the hash of an arbitrarily long sequence of bytes using
 * rotr64. The quality of the output is adjusted using a magic_number,
 * two good options are provided in this header as kHashMagicNum1 and
 * kHashMagicNum2.
 */
inline uint64_t hash_bytes(const std::byte* str, size_t len, 
                           uint64_t magic_num=kHashMagicNum1) 
{
    uint64_t hashState = len;

    for (size_t idx = 0; idx < len; ++idx) {
      hashState = magic_num * hashState + (unsigned char) str[idx];
    }

    /*
     * The final scrambling helps with short keys that vary only on the
     * high order bits. Low order bits are not always well distributed 
     * so shift them to the high end, where they'll form part of the 
     * 14-bit tag.
     */
    return rotr64(magic_num * hashState, 6);
}

/*
 * Calculate the hash of an arbitrarily long sequence of bytes with a
 * specified salt using rotr64. The quality of the output is adjusted 
 * using a magic_number, two good options are provided in this header 
 * as kHashMagicNum1 and kHashMagicNum2.
 */
inline uint64_t hash_bytes_with_salt(const char* str, size_t len, 
                                     uint16_t salt, 
                                     uint64_t magic_num=kHashMagicNum1) 
{
    uint64_t hashState = len;

    for (size_t idx = 0; idx < len; ++idx) {
        hashState = magic_num * hashState + (unsigned char) str[idx];
    }

    hashState = magic_num * hashState + salt;

    return rotr64(magic_num * hashState, 6);
}

}
