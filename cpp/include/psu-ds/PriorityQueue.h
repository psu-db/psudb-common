/*
 * include/psu-ds/PriorityQueue.h
 *
 * Copyright (C) 2023-2026 Douglas B. Rumbaugh <dbrumbaugh@harrisburgu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
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
class PriorityQueue {
public:
    PriorityQueue(size_t size, R* cmp_baseline=nullptr) : data(size), tail(0), cmp(cmp_baseline) {}

    ~PriorityQueue() = default;

    size_t size() const {
        return tail;
    }

    bool empty() const {
        return tail == 0;
    }

    void pop() {
        assert(this->tail != 0);

        // If there is only one element, just decrement the
        // tail.
        if (this->size() == 1) {
            this->tail--;
            return;
        }

        swap(0, --this->tail);

        ssize_t idx = 0;
        ssize_t child_idx;

        while ((child_idx = min_child(idx)) != -1 && heap_cmp(child_idx, idx)) {
            swap(idx, child_idx);
            idx = child_idx;
        }
    }

    void push(const R* record, size_t version=0) {
        assert(tail != this->data.size());

        size_t new_idx = this->tail++;
        this->data[new_idx] = {record, version};

        while (new_idx != 0 && !heap_cmp(parent(new_idx), new_idx)) {
            swap(parent(new_idx), new_idx);
            new_idx = parent(new_idx);
        }
    }


    queue_record<R> peek() {
        assert(this->tail > 0);
        return this->data[0];
    }

    queue_record<R> peek_second() {
        assert(this->tail > 1);
        ssize_t idx = min_child(0);
        assert(idx != -1);
        return this->data[idx];
    }

private:
    std::vector<queue_record<R>> data;
    size_t tail;
    CMP cmp;

    /*
     * Swap the elements at position a and position
     * b within the heap
     */
    inline void swap(size_t a, size_t b) {
        auto temp = this->data[a];
        this->data[a] = this->data[b];
        this->data[b] = temp;
    }

    inline size_t parent(size_t idx) {
        return (idx - 1) / 2;
    }

    inline ssize_t min_child(size_t idx) {
        size_t left = 2 * idx + 1;
        if (left >= tail)
            return -1;

        size_t right = left + 1;
        if (right >= tail)
            return left;

        return heap_cmp(left, right) ? left : right;
    }

    inline bool heap_cmp(size_t a, size_t b) {
        /* If the values differ, compare by value. Use both directions
         * to distinguish "a < b" from "equal" -- when values are equal,
         * fall through to version/tombstone tiebreaking. */
        if (cmp(data[a].data, data[b].data)) {
            return true;
        } else if (cmp(data[b].data, data[a].data)) {
            return false;
        }

        /* Values are equal -- break ties by version. */
        if (data[a].version != data[b].version) {
            return data[a].version < data[b].version;
        }

        /* Same value and version -- break ties by tombstone status. */
        constexpr bool has_tombstone = requires(const R &r) {
            r.is_tombstone();
        };

        if constexpr (has_tombstone) {
            return data[a].data->is_tombstone() && !data[b].data->is_tombstone();
        } else {
            return false;
        }
    }
};
}
