/*
 * include/ds/LockedPriorityQueue.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 * A simple wrapper around std::PriorityQueue adding a mutex for
 * thread safety.
 *
 */
#pragma once

#include <queue>
#include <mutex>

namespace psudb {

template <class T, class Container = std::vector<T>, class Compare = std::less<typename Container::value_type>>
class LockedPriorityQueue : protected std::priority_queue<T, Container, Compare> {
    typedef std::priority_queue<T, Container, Compare> Parent;

    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return Parent::empty();
    }

    bool size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return Parent::size();
    }

    void push(T &item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        Parent::push(item);
    }

    void push(T &&item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        Parent::push(item);
    }

    void emplace(T &&item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        Parent::emplace(item);
    }

    T pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto t = Parent::top();
        Parent::pop();
        return t;
    }

private:
    std::mutex m_mutex;
};
}

