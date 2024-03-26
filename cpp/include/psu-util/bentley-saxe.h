/*
 * psu-util/bentley-saxe.h
 *
 * Copyright (C) 2023-2024  Douglas B. Rumbaugh <drumbaugh@psu.edu)
 * Distributed under the Modified BSD License.
 *
 * A general framework implementing the Bentley-Saxe method using C++ generics.
 * The data structure being used must support the BentleyInterface interface
 * described in this file. This condition is enforced using concepts.
 *
 * For more information see,
 *
 * [1] https://jeffe.cs.illinois.edu/teaching/datastructures/notes/01-statictodynamic.pdf
 *
 * [2] J. L. Bentley and J. B. Saxe. Decomposable searching problems I: Static-to-dynamic
 *     transformation. J. Algorithms 1(4):301–358, 1980.
 */
#pragma once

#include <vector>
#include <concepts>
#include <cassert>

namespace psudb { namespace bsm {

template <typename DS, typename R>
concept BentleyInterface = requires(DS ds, R rec, void *q, std::vector<R> recset) {
    {ds.query(q)} -> std::same_as<std::vector<R>>;
    {ds.query_merge(recset, recset)} -> std::same_as<std::vector<R>>;
    {ds.unbuild()} -> std::same_as<std::vector<R>>;
    {DS::build(recset)} -> std::same_as<DS*>;
    {ds.record_count()} -> std::convertible_to<size_t>;
};

template <typename R, BentleyInterface<R> DS>
class BentleySaxe {
    typedef std::vector<R> result_set;
    typedef std::vector<R>::const_iterator record_itr;
    typedef std::vector<R> record_set;

public:
    BentleySaxe() = default;

    ~BentleySaxe() {
        for (size_t i=0; i<m_levels.size(); i++) {
            delete m_levels[i];
        }
    }

    void insert(R &rec) {
        record_set S = {rec};

        /* find the first empty level */
        ssize_t target_idx = -1;
        for (size_t i=0; i<m_levels.size(); i++) {
            if (m_levels[i] == nullptr) {
                target_idx = i;
                break;
            }

            /* deconstruct the level */
            auto tmp = m_levels[i]->unbuild();
            delete m_levels[i];
            m_levels[i] = nullptr;
            
            /* union the level's records into the running set */
            S.insert(S.end(), tmp.begin(), tmp.end());
        }

        /* 
         * If we didn't find an empty level, then we
         * need to grow the structure.
         */
        if (target_idx == -1) {
            m_levels.emplace_back(DS::build(S));
        } else {
            m_levels[target_idx] = DS::build(S);
        }
    }

    result_set query(void *q) {
        assert(q != nullptr);

        result_set results;
        for (size_t i=0; i<m_levels.size(); i++) {
            if (m_levels[i]) {
                auto temp = m_levels[i]->query(q);
                results = m_levels[i]->query_merge(results, temp);
            }
        }

        return std::move(results);
    }

    size_t record_count() {
        size_t total = 0;
        for (size_t i=0; i<m_levels.size(); i++) {
            if (m_levels[i]) {
                total += m_levels[i]->record_count();
            }
        }

        return total;
    }

private:
    std::vector<DS*> m_levels;
};

}}

