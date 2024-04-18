/*
 * psu-ds/StaticISAM.h
 *
 * Copyright (C) 2023-2024 Douglas B. Rumbaugh <drumbaugh@psu.edu)
 * Distributed under the Modified BSD License.
 *
 * A simple map implementation based on an in-memory, static ISAM tree 
 *
 */
#pragma once

#include <cmath>
#include <cassert>
#include <vector>
#include <array>

#include "psu-util/alignment.h"

namespace psudb {

template <typename K, typename V, size_t FANOUT=64>
class ISAMTree {
private:
    typedef std::pair<K, V> R;

    struct internal_node {
        std::array<K, FANOUT> m_keys;
        std::array<byte *, FANOUT> m_ptrs;
    };

   
    static constexpr size_t leaf_sz = sizeof(R) * FANOUT;
    static_assert(FANOUT > 0 && (FANOUT & (FANOUT - 1)) == 0, "Fanout parameter must be a power of two");
    typedef internal_node inode;

public:
    struct RangeQueryParameters {
        K lower_bound;
        K upper_bound;
    };

public:
    static ISAMTree *build(std::vector<R> &records) {
        std::sort(records.begin(), records.end());
        return new ISAMTree(records);
    }

    static ISAMTree *build_presorted(std::vector<R> &records) {
        return new ISAMTree(records);
    }

    std::vector<R> unbuild() {
        return std::move(m_data);
    }

    std::vector<R> query(void *q) {
        std::vector<R> rs;

        /* return an empty result set if q is invalid */
        if (q == nullptr) {
            return rs;
        }

        auto parms = (ISAMTree::RangeQueryParameters*) q;

        size_t idx = lower_bound(parms->lower_bound);

        while (idx < m_data.size() && m_data[idx].first < parms->upper_bound) {
            rs.emplace_back(m_data[idx++]);
        }

        return std::move(rs);
    }

    std::vector<R> query_merge(std::vector<R> &rsa, std::vector<R> &rsb) {
        rsa.insert(rsa.end(), rsb.begin(), rsb.end());
        return std::move(rsa);
    }

    size_t record_count() {
        return m_data.size();
    }

    size_t internal_node_count() {
        return m_root - m_inodes + 1 - (m_data.size() == 0);
    }

    ~ISAMTree() {
        free(m_inodes);
    }


private:
    std::vector<R> m_data;
    inode *m_root;
    inode *m_inodes;

    ISAMTree(std::vector<R> &records) : m_inodes(nullptr), m_root(nullptr) {
        m_data = std::move(records);

        if (m_data.size() > 0) {
            m_root = build_internal_levels();
        }
    }

    inline bool is_leaf(const byte *ptr) {
        return ptr >= (byte *) m_data.begin().base() && ptr < (byte *) m_data.end().base();
    }

    inode *build_internal_levels() {
        /*
         * Allocate memory to store the internal nodes. These will all be stored contiguously
         * as an array; the ISAM Tree's index structure is static, so there's no need to use
         * a more flexible representation.
         */
        size_t lnode_cnt = psudb::TYPEALIGN(leaf_sz, m_data.size() * sizeof(R)) / leaf_sz;
        size_t inode_cnt = required_internal_node_cnt(lnode_cnt, FANOUT);
        m_inodes = (inode *) psudb::sf_aligned_calloc(psudb::CACHELINE_SIZE, inode_cnt, sizeof(inode));

        /*
         * Construct the first level of internal nodes based upon the leaf data
         */
        inode *cur_inode = m_inodes;
        size_t i = 0;
        for (auto itr = m_data.begin(); itr < m_data.end(); itr += FANOUT) {
            (cur_inode->m_keys)[i] = itr->first;
            (cur_inode->m_ptrs)[i] = (byte *) itr.base();

            i = (i + 1) & (FANOUT - 1);
            cur_inode += (i == 0);
        }

        /*
         * If there's only one internal node total, it is the root and so we
         * are done. 
         */
        if (inode_cnt == 1) {
            return cur_inode - (i == 0);
        }

        /* If we didn't fully fill the last node (in which case cur_inode
         * is already pointing at the next empty internal node), advance to 
         * the next internal node.
         */
        if (i > 0) cur_inode++;

        /*
         * Construct the remaining internal levels based on the separator keys
         * in the level below.
         */
        inode *level_start = m_inodes;
        inode *level_stop = cur_inode;
        do {
            i=0;
            for (inode *itr = level_start + 1; itr < level_stop; itr++) {
                cur_inode->m_keys[i] = (itr-1)->m_keys[0];
                cur_inode->m_ptrs[i] = (byte *) (itr-1);

                i = (i + 1) & (FANOUT - 1);
                cur_inode += (i == 0);
            } 

            level_start = level_stop;
            level_stop = cur_inode;
        } while (level_stop - level_start + 1 > 1);

        return cur_inode - (i == 0);
    }

    size_t lower_bound(K key) {
        /*
         * If the tree is empty, just return 0. 
         */
        if (!m_root) {
            return 0;
        }

        const inode *current = m_root;

        /*
         * Use the internal node separator keys to traverse the tree
         * and identify the leaf node which will contain the lower bound
         * on the requested key.
         */
        while (!is_leaf((byte *) current)) {
            const byte *next = nullptr;
            // note: skip the last separator key, this case is handled
            // manually below
            for (size_t i=0; i<FANOUT-1; i++) { 
                if (current->m_ptrs[i+1] == nullptr || key <= current->m_keys[i+1]) {
                    next = current->m_ptrs[i];
                    break;
                }
            }

            current = next ? (inode *) next : (inode *) current->m_ptrs[FANOUT-1];
        }

        /*
         * If current falls within the leaf-node range, then it is
         * safe to cast from an internal_node to a leaf node pointer.
         */
        R *leaf_ptr = (R *) current;

        /*
         * Scan the leaf node for the lower bound on the record.
         * FIXME: for larger fanouts, it may be more effective to
         * do a binary search here. This could be a simple if constexpr
         * substitution.
         */
        for (/* intentionally blank */; leaf_ptr < m_data.end().base(); leaf_ptr++) {
            if (leaf_ptr->first >= key) {
                break;
            }
        }

        return leaf_ptr - m_data.data();
    }


    /*
     * Determines the number of internal nodes required to build an ISAM Tree
     * with specified fanout over a given number of leaf nodes. This function
     * uses the formula for the sum of a geometric series of (1/f)^i to calculate 
     * the total number of nodes for all levels but the root, and then adds the 
     * root node and subtracts off the leaf nodes to get the final result.
     */
    static inline size_t required_internal_node_cnt(size_t node_cnt, size_t fanout) {
        /*
         * I'm tired of fighting with numerical typing...
         */
        double df = fanout;
        double dnc = node_cnt; 

        size_t inode_cnt = ceil(dnc * (1.0 - pow(1.0/df, log(dnc)/log(df))) / (1.0 - 1.0/df)) - (dnc - 1);

        /* if inode_cnt > 1, then we'll need to account for the extra root node too */
        if (inode_cnt > 1) inode_cnt++;

        /* 
         * if inode_cnt == 0, then we'll add one to it. This is a special case 
         * that could probably be avoided 
         */
        if (inode_cnt == 0) inode_cnt++;

        return inode_cnt;
    }
};

}
