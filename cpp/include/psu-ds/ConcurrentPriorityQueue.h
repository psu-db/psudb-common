/*
 * include/ds/ConcurrentConcurrentPQ.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 * A lock-free priority queue based on Skiplists, using the
 * procedure described by Lotan and Shavit in
 *
 * [1] Skiplist-Based Concurrent Priority Queues. IPDPS 2000: 263-268
 *
 */
#pragma once

#include <vector>
#include <cassert>

namespace psudb {

template <typename R>
struct queue_record {
    const R *data;
    size_t version;
};

template <typename R>
class standard_minheap {
public:
    standard_minheap(R *baseline) {}
    inline bool operator()(const R* a, const R* b) {
        return *a < *b;
    }
};

template <typename R>
class standard_maxheap {
public:
    standard_maxheap(R *baseline) {}
    inline bool operator()(const R* a, const R* b) {
        return *a > *b;
    }
};

template <typename R, typename CMP=standard_minheap<R>>
class ConcurrentPQ {
public:
    ConcurrentPQ() {}

    ~ConcurrentPQ() = default;

    size_t size() const {

    }

    queue_record<R> pop() {

    }

    void push(const R* record, size_t version=0) {

    }

private:
    std::vector<queue_record<R>> data;
    CMP cmp;

};
}
