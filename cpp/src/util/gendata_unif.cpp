/*
 * src/util/gendata_unif.cpp
 *
 * Copyright (C) 2024 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 * A simple uniform data generator. 
 *
 */


#include <cstdlib>
#include <gsl/gsl_rng.h>

void check_inputs(int64_t min, int64_t max, int64_t cnt) {
    if (cnt < 0) {
        fprintf(stderr, "[E]: Invalid count [%ld], negative or too large.\n", cnt);
        exit(EXIT_FAILURE);
    }

    if (min >= max) {
        fprintf(stderr, "[E]: Invalid range, the minimum value must be smaller than the maximum\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: gendata_unif min_value max_value count\n");
        exit(EXIT_FAILURE);
    }

    int64_t min = atol(argv[1]);
    int64_t max = atol(argv[2]);
    int64_t cnt = atol(argv[3]);

    check_inputs(min, max, cnt);

    size_t range = max - min;

    gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);

    // FIXME: This could be easily fixed by repeated calls
    if (gsl_rng_max(rng) < range) {
        fprintf(stderr, "[W]: Requested range exceeds generator capacity. Range of output numbers will be truncated.\n");
    }

    for (size_t i=0; i<cnt; i++) {
        int64_t num = min + gsl_rng_uniform_int(rng, range);
        fprintf(stdout, "%ld\n", num);
    }

    gsl_rng_free(rng);
}


