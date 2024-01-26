/*
 * include/psu-thrd/threadpool.h
 *
 * Copyright (C) 2023 Douglas Rumbaugh <drumbaugh@psu.edu>
 *
 * All rights reserved. Published under the Revised BSD License.
 *
 * A simple thread pool using std::jthread. Note that this pool does not
 * maintain an internal job queue--jobs can only be scheduled when there
 * is an idle thread on which to run it.
 *
 * The pool exposes each jthread's std::top_token to allow for preemption,
 * though this functionality requires coordination between thread, the
 * pool, and the scheduler.
 */
#pragma once

#include <thread>

namespace psudb {

class ThreadPool {
public:

    ThreadPool(size_t max_thread_cnt) {

    }

    ~ThreadPool() {

    }

    /*
     * Returns the number of currently active threads in
     * the pool.
     */
    size_t get_active_thread_count() {
        return 0;
    }

    /*
     * Returns the number of currently idle threads in
     * the pool.
     */
    size_t get_idle_thread_count() {
        return 0;
    }

    /*
     * Initiate a shutdown of the thread pool. If a non-preemptive
     * shutdown is request, the thread pool will stop accepting
     * new jobs and will wait for all threads to terminate. If a preemptive
     * shutdown is requested, the pool will signal all running threads
     * to terminate immediately, although whether or not they do depends
     * upon the threads themselves.
     *
     * In either case, this call will block until all active threads have
     * terminated
     */
    void shutdown(bool preemptive=false) {

    }

    /*
     * Runs the provided function on a thread within the
     * pool, assuming that an idle thread is available. If
     * an idle thread is not available, 
     *
     */
    void initiat_job() {

    }



private:

};


}


