/*
 * src/util/gendata_zipf.cpp
 *
 * Copyright (C) 2024 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 * A simple Zipfian data generator. 
 *
 * FIXME: adjust the zipf_distribution class to use gsl_rng for consistency
 *
 */


#include <cstdlib>
#include <gsl/gsl_rng.h>
#include <random>

#include "psu-dist/zipf.h"

void check_inputs(int64_t max, double skew, int64_t cnt) {
    if (cnt < 0) {
        fprintf(stderr, "[E]: Invalid count [%ld], negative or too large.\n", cnt);
        exit(EXIT_FAILURE);
    }

    if (max <= 0) {
        fprintf(stderr, "[E]: Invalid range, the max must be positive.\n");
        exit(EXIT_FAILURE);
    }

    if (skew <= 0 || skew > 1) {
        fprintf(stderr, "[E]: Invalid skew, must be on the interval (0, 1]\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: gendata_unif max_value skew count\n");
        exit(EXIT_FAILURE);
    }

    int64_t max = atol(argv[1]);
    double skew = atof(argv[2]);
    int64_t cnt = atol(argv[3]);

    check_inputs(max, skew, cnt);

    auto dist = psudb::zipf_distribution<uint64_t, double>(max, skew);
    auto rng = std::mt19937(std::random_device{}());

    for (size_t i=0; i<cnt; i++) {
        fprintf(stdout, "%ld\n", dist(rng));
    }
}


