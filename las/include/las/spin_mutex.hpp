#pragma once
#ifndef SPIN_MUTEX_HPP
#define SPIN_MUTEX_HPP

#include <atomic>
#include <immintrin.h>

namespace las {

    /// Spin mutex
    struct spin_mutex {

        /// locks the mutex
        void lock() noexcept {
            for (;;) {
                // Optimistically assume the lock is free on the first try
                if (!_lock.exchange(true, std::memory_order_acquire)) {
                    return;
                }
                // Wait for lock to be released without generating cache misses
                while (_lock.load(std::memory_order_relaxed)) {
                    // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
                    // hyper-threads

                    _mm_pause();
                }
            }
        }

        /// tries to lock the mutex
        /// \return true if the lock was acquired, false otherwise
        [[nodiscard]] bool try_lock() noexcept {
            // First do a relaxed load to check if lock is free in order to prevent
            // unnecessary cache misses if someone does while(!try_lock())
            return !_lock.load(std::memory_order_relaxed) &&
                   !_lock.exchange(true, std::memory_order_acquire);
        }

        /// unlocks the mutex
        void unlock() noexcept {
            _lock.store(false, std::memory_order_release);
        }

    private:
        std::atomic_bool _lock { false };
    };
}

#endif //SPIN_MUTEX_HPP
