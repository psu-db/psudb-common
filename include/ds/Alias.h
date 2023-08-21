/*
 * include/ds/Alias.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *                    Dong Xie <dongx@psu.edu>
 *
 * All rights reserved. Published under the Simplified BSD License.
 *
 */
#pragma once

#include <gsl/gsl_rng.h>
#include <vector>

namespace de {

/*
 * An implementation of Walker's Alias Structure for weighted set sampling. 
 *
 * See the following papers for details,
 *   [1] Walker, A. J. (April 1974). "New fast method for generating discrete random 
 *   numbers with arbitrary frequency distributions". Electronics Letters. 10 (8): 127.
 *
 *   [2] Vose, Michael D. (September 1991). "A linear algorithm for generating random 
 *   numbers with a given distribution" (PDF). IEEE Transactions on Software Engineering. 
 *   17 (9): 972–975. 
 */
class Alias {
public:
    /*
     *  Create a new alias structure based on a vector of weights. This vector
     *  _must_ be normalized. The structure will then support independent weighted 
     *  set sampling, returning the index of the selected item within the weight
     *  vector. The required information is copied and stored internally, so the
     *  input vector can be repurposed following this call without affecting the
     *  alias structure.
     */
    Alias(const std::vector<double>& normalized_weights)
    : m_alias(normalized_weights.size()), m_cutoff(normalized_weights.size()) {
        size_t n = normalized_weights.size();
        auto overfull = std::vector<size_t>();
        auto underfull = std::vector<size_t>();
        overfull.reserve(n);
        underfull.reserve(n);

        // initialize the probability_table with n*p(i) as well as the overfull and
        // underfull lists.
        for (size_t i = 0; i < n; i++) {
            m_cutoff[i] = (double) n * normalized_weights[i];
            if (m_cutoff[i] > 1) {
                overfull.emplace_back(i);
            } else if (m_cutoff[i] < 1) {
                underfull.emplace_back(i);
            } else {
                m_alias[i] = i;
            }
        }

        while (overfull.size() > 0 && underfull.size() > 0) {
            auto i = overfull.back(); overfull.pop_back();
            auto j = underfull.back(); underfull.pop_back();

            m_alias[j] = i;
            m_cutoff[i] = m_cutoff[i] + m_cutoff[j] - 1.0;

            if (m_cutoff[i] > 1.0) {
                overfull.emplace_back(i);
            } else if (m_cutoff[i] < 1.0) {
                underfull.emplace_back(i);
            }
        }
    }

    size_t get(const gsl_rng* rng) {
        double coin1 = gsl_rng_uniform(rng);
        double coin2 = gsl_rng_uniform(rng);

        size_t k = ((double) m_alias.size()) * coin1;
        return coin2 < m_cutoff[k] ? k : m_alias[k];
    }

private:
    std::vector<size_t> m_alias;
    std::vector<double> m_cutoff;
};

}
