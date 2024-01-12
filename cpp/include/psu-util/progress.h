/*
 * include/util/progess.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu> 
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 * A simple progress bar implementation.
 *
 */
#pragma once

#include <cstdlib>
#include <cstdio>
#include <string>

static const char *g_prog_bar = "======================================================================";
static const size_t g_prog_width = 70;

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
 *
 * FIXME: In the present implementation, if the terminal buffer width shrinks to less 
 * than the width of the displayed text, it will display on new lines. 
 */
static void progress_update(double percentage, std::string prompt) {
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * g_prog_width);
    int rpad = (int) (g_prog_width - lpad);
    fprintf(stderr, "\r(%3d%%) %s [%.*s%*s]", val, prompt.c_str(), lpad, g_prog_bar, rpad, "");
    fflush(stderr);   
}
