/*
 * src/util/genquery_range.cpp
 *
 * Copyright (C) 2024 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 * A simple range query generator. Will read a provided data file (must be
 * able to store entire file contents in memory) and will output a
 * specified number of query ranges with a specified selectivity. It will
 * only consider values in the first column of input, and assumes each
 * record is on its own line. You may need to pre-process the input with
 * cut(1) if the key field resides in a different location. All keys are
 * assumed to be integers.
 */


#include <cstdlib>
#include <string>
#include <gsl/gsl_rng.h>
#include <fstream>
#include <istream>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

void check_inputs(std::string fname, double sel, int64_t cnt) {
    if (cnt < 0) {
        fprintf(stderr, "[E]: Invalid count [%ld], negative or too large.\n", cnt);
        exit(EXIT_FAILURE);
    }

    if (sel <= 0 || sel >=1) {
        fprintf(stderr, "[E]: Invalid selectivity, must fall on the interval (0,1)\n");
        exit(EXIT_FAILURE);
    }
}

std::vector<int64_t> read_data(std::istream &input) {
    std::vector<int64_t> keys;

    std::string line;
    while (std::getline(input, line, '\n')) { 
        std::stringstream ls(line);
        std::string key;
        if (ls >> key) {
            int64_t k = std::stol(key);
            keys.emplace_back(k);
        }
    }

    return keys;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: genquery_range filename selectivity count\n");
        exit(EXIT_FAILURE);
    }

    std::string fname = std::string(argv[1]);
    double sel = atof(argv[2]);
    int64_t cnt = atol(argv[3]);

    check_inputs(fname, sel, cnt);

    std::vector<int64_t> data;
    if (fname == "-") {
        data = read_data(std::cin); 
    } else {
        std::fstream file {fname, std::ios::in};
        data = read_data(file);
    }

    std::sort(data.begin(), data.end());

    gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);

    if (data.size() > gsl_rng_max(rng)) {
        fprintf(stderr, "[W]: data size exceeds rng range--queries may not uniformly access the larger value of the dataset.\n");
    }

    size_t range_length = data.size() * sel;
    size_t i=0;
    while (i < cnt) {
        size_t start = gsl_rng_uniform_int(rng, data.size());

        /* 
         * Move backwards over any duplicate keys to ensure correct
         * selectivity 
         */
        while (start > 0 && data[start - 1] == data[start]) {
            start--;
        }

        // FIXME: This is to ensure an exact match on the request
        // selectivity, but it may result in infinite looping in some
        // adversarial cases.
        size_t end = start + range_length;
        if (end < data.size() - 1 && data[end] == data[end+1]) {
            continue;
        }

        fprintf(stdout, "%ld %ld %lf\n", start, end, sel);

        i++;
    }

    gsl_rng_free(rng);
}


