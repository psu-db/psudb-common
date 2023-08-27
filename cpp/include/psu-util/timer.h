/*
 * include/util/timer.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 */
#pragma once

#include <chrono>

#ifdef ENABLE_TIMER
/*
 * Create the variables timer_start and timer_stop, to used by
 * the timer functions of this module. Must be called first, before
 * TIMER_START, TIMER_STOP, or TIMER_RESULT are called. Can only be 
 * called once in a given scope, as it defines two variables.
 *
 * NOP if ENABLE_TIMER is not defined.
 */
#define TIMER_INIT() \
    auto timer_start = std::chrono::high_resolution_clock::now(); \
    auto timer_stop = std::chrono::high_resolution_clock::now();

/*
 * Start the timer. Assigns current time to timer_start variable.
 *
 * NOP if ENABLE_TIMER is not defined.
 */
#define TIMER_START() \
    timer_start = std::chrono::high_resolution_clock::now()

/*
 * Stop the timer. Assigns current time to timer_stop variable.
 *
 * NOP if ENABLE_TIMER is not defined.
 */
#define TIMER_STOP() \
    timer_stop = std::chrono::high_resolution_clock::now()

/*
 * Return the amount of time, in nanoseconds, that elapsed between the
 * most recent call to TIMER_START and the most recent call to TIMER_STOP.
 * The result is undefined if TIMER_START() and TIMER_STOP() were not
 * called, or were called in the wrong order.
 *
 * Always returns 0 if ENABLE_TIMER is not defined.
 */
#define TIMER_RESULT() \
    std::chrono::duration_cast<std::chrono::nanoseconds>(timer_stop - timer_start).count()

#else
/*
 * Create the variables timer_start and timer_stop, to used by
 * the timer functions of this module. Must be called first, before
 * TIMER_START, TIMER_STOP, or TIMER_RESULT are called. Can only be 
 * called once in a given scope, as it defines two variables.
 *
 * NOP if ENABLE_TIMER is not defined.
 */
#define TIMER_INIT() \
    do {} while(0)

/*
 * Start the timer. Assigns current time to timer_start variable.
 *
 * NOP if ENABLE_TIMER is not defined.
 */
#define TIMER_START() \
    do {} while(0)

/*
 * Stop the timer. Assigns current time to timer_stop variable.
 *
 * NOP if ENABLE_TIMER is not defined.
 */
#define TIMER_STOP() \
    do {} while(0)
/*
 * Return the amount of time, in nanoseconds, that elapsed between the
 * most recent call to TIMER_START and the most recent call to TIMER_STOP.
 * The result is undefined if TIMER_START() and TIMER_STOP() were not
 * called, or were called in the wrong order.
 *
 * Always returns 0 if ENABLE_TIMER is not defined.
 */
#define TIMER_RESULT() \
    0l

#endif /* ENABLE_TIMER */

