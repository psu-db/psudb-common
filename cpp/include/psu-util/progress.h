/*
 * include/util/progess.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 * A simple progress bar implementation.
 *
 * NOTE: If the progress bar's total width exceeds the width of the terminal
 *       the bar will not function correctly. The change_progress_width 
 *       function is provided to adjust the total width of the progress bar,
 *       but it is up to the user to set it correctly.
 *
 * FIXME: The above caveat may be fixable using the $COLUMNS environment
 *        variable to automatically get the terminal width, but I don't 
 *        think that this variable will be updated on terminal resize 
 *        following the forking off of the process.
 */
#pragma once

#include <cstdlib>
#include <cstdio>
#include <string>

namespace psudb {

static const std::string g_prog_bar = "======================================================================";
static size_t g_prog_width = 70;

/*
 * Display a simple progress bar to standard error based on a provided percentage.  
 * Percentage should be a float between 0 and 1, and prompt represents arbitrary 
 * text that is displayed before the bar.
 *
 * The format of the bar is,
 * (PERCENTAGE%) PROMPT [===....]
 *
 * The bar uses a carriage return, and so it will sit on the same line unless other 
 * I/O is interspersed with it. 
 */
[[maybe_unused]] static void progress_update(double percentage, const std::string &prompt, FILE *target=stderr) {
    int val =  static_cast<int>(percentage * 100);
    int lpad = static_cast<int>(percentage * g_prog_width);
    int rpad = static_cast<int>(g_prog_width - lpad);
    fprintf(target, "\r(%3d%%) %s [%.*s%*s]", val, prompt.c_str(), lpad, g_prog_bar.c_str(), rpad, "");
    if (std::abs(1.0 - percentage) < .0001) {
        fprintf(target, "\n");
    }
    fflush(target);   
}

/*
 * Update the "width" of the progress bar. This controls how many
 * characters width the contents of the bar (between the []) is. The
 * provided width must be [1, 70], though strange behavior might be exhibited
 * with bars less than 10 units long.
 *
 * Returns true if the value is updated and false if it is not (due to being
 * out of range).
 */
[[maybe_unused]] static bool change_progress_width(size_t new_width) {
    if (new_width >= 1 && new_width <= g_prog_bar.length()) {
        g_prog_width = new_width;
        return true;
    }

    return false;
}
}
