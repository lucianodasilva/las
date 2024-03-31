#pragma once
#ifndef BARRIER_HPP
#define BARRIER_HPP

#include <atomic>

#include <las/details.hpp>
#include <las/system.hpp>

namespace las {

    /// Barrier synchronization primitive
    struct barrier : no_copy {
        static_assert (std::is_standard_layout_v <std::atomic_int32_t>, "std::atomic_int must be standard layout");

        /// barrier constructor
        /// \param COUNT number of threads to synchronize
        explicit barrier (int32_t COUNT);

        /// arrives at the barrier and waits for all threads to arrive
        void arrive_and_wait () {
            auto const WAIT_PHASE = _phase.load ();
            auto wait_counter = (--_counter.atomic);

            if (wait_counter > 0) { // if we are waiters... wait
                while (WAIT_PHASE == _phase.load () && wait_counter > 0) { // NOLINT
                    futex_wait (&_counter.integer, wait_counter);
                    wait_counter = _counter.atomic.load();
                }
            } else if (wait_counter == 0) {
                // complete barrier phase
                ++_phase;
                _counter.atomic.store(COUNTER_RESET_VAL);

                futex_wake_all (&_counter.integer);
            } else {
                throw std::runtime_error ("Barrier is in invalid state");
            }
        }

    private:
        int32_t const COUNTER_RESET_VAL;

        union {
            std::atomic_int32_t atomic;
            int32_t             integer;
        } _counter {0};

        std::atomic_int32_t _phase {0};
    };
}

#endif //BARRIER_HPP
