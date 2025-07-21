#pragma once
#ifndef LAS_SYSTEM_HPP
#define LAS_SYSTEM_HPP

#include <cstdint>
#include <filesystem>
#include <limits>
#include <optional>
#include <thread>
#include <type_traits>
#include <vector>

#include <las/config.hpp>

#if defined (LAS_OS_GNU_LINUX)
#   include <unistd.h>
#   include <linux/futex.h>
#   include <sys/syscall.h>
#endif

#if defined (LAS_OS_WINDOWS)
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <windows.h>
#endif

namespace las {

    /// Futex value type
    using futex_value_t = int32_t;

    /// Futex wait result type
    enum struct futex_wait_result : uint8_t {
        awake = 0,      ///< futex was woke up, may also a spurious wake up
        unmatched,      ///< futex value did not match the expected value
        timeout,        ///< futex wait timed out
        interrupted,    ///< futex wait was interrupted by a signal
        error           ///< futex wait call resulted in an error
    };

    /// Futex wait on address
    /// \param address address to wait on
    /// \param expected_value expected value
    /// \param TIMEOUT timeout for the wait. If zero, the wait is indefinite
    /// \return true if the futex was woken up, false otherwise
    inline futex_wait_result futex_wait (futex_value_t * address, futex_value_t expected_value, std::chrono::milliseconds TIMEOUT = std::chrono::milliseconds::zero()) noexcept;

    /// Futex wake all threads waiting on the address
    /// \param address address to wake up
    inline void futex_wake_all (futex_value_t * address) noexcept;

    /// Futex wake one thread waiting on the address
    /// \param address address to wake up
    inline void futex_wake_one (futex_value_t * address) noexcept;

    /// CPU core id type
    using core_id_t = std::size_t;

    /// CPU core id for undefined core
    constexpr auto UNDEFINED_CORE_ID = std::numeric_limits< core_id_t >::max();

    /// Get the number and estimated ids for the available pyhsical cpu cores
    /// \return vector of core id
    std::vector < core_id_t > physical_cores ();

    /// Get the native handle for the calling thread
    inline std::thread::native_handle_type this_thread_native_handle ();

    /// Set the affinity of the thread to the specified core
    inline void thread_affinity_set (std::thread & thread, core_id_t core_id);

    /// Set the affinity of the calling thread to the specified core
    inline void this_thread_affinity_set (core_id_t core_id);

    /// Read the content of a file into a string
    /// \param file path to the file
    /// \return optional string with the file content
    std::optional < std::string > file_content (std::filesystem::path const & file);

    /// Count leading zeroes
    /// \param value unsigned integer value
    /// \return the number of leading zero bits
    template < typename value_t >
    uint8_t clz (value_t value, std::enable_if_t < std::is_unsigned_v < value_t >, int > = 0);

    /// Count trailing zeroes
    /// \param value unsigned integer value
    /// \return the number of trailing zero bits
    template < typename value_t >
    uint8_t ctz (value_t value, std::enable_if_t < std::is_unsigned_v < value_t >, int > = 0);

#if defined (LAS_OS_GNU_LINUX)

    inline futex_wait_result futex_wait (futex_value_t * address, futex_value_t expected_value, std::chrono::milliseconds const TIMEOUT) noexcept {
        using namespace std::chrono;

        // convert the timeout to a timespec
        auto const SEC = duration_cast < seconds > (TIMEOUT);
        auto const NSEC = duration_cast < nanoseconds > (TIMEOUT - SEC);

        struct timespec ts { SEC.count(), NSEC.count () };
        auto * ts_ptr = (TIMEOUT == milliseconds::zero () ? nullptr : &ts);

        // wait on the futex
        auto const res = syscall (SYS_futex, address, FUTEX_WAIT_PRIVATE, expected_value, ts_ptr, nullptr, 0); // NOLINT - either syscall or inline assembly

        // check the result
        if (res == 0) { return futex_wait_result::awake; }

        // check for errors
        switch (errno) {
            case ETIMEDOUT:
                return futex_wait_result::timeout;
            case EINTR:
                return futex_wait_result::interrupted;
            case EAGAIN:
                return futex_wait_result::unmatched;
            default:
                return futex_wait_result::error;
        }
    }

    inline void futex_wake_all (futex_value_t * address) noexcept {
        syscall (SYS_futex, address, FUTEX_WAKE_PRIVATE, std::numeric_limits< futex_value_t >::max(), nullptr, nullptr, 0); // NOLINT - either syscall or inline assembly
    }

    inline void futex_wake_one (futex_value_t * address) noexcept {
        syscall (SYS_futex, address, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0); // NOLINT - either syscall or inline assembly
    }

    inline std::thread::native_handle_type this_thread_native_handle () {
        return pthread_self();
    }

    inline void thread_affinity_set (std::thread & thread, core_id_t core_id) {
        cpu_set_t cpuset;
        CPU_ZERO (&cpuset);
        CPU_SET (core_id, &cpuset);
        pthread_setaffinity_np (thread.native_handle(), sizeof (cpu_set_t), &cpuset);
    }

    inline void this_thread_affinity_set (core_id_t core_id) {
        cpu_set_t cpuset;
        CPU_ZERO (&cpuset);
        CPU_SET (core_id, &cpuset);
        pthread_setaffinity_np (this_thread_native_handle (), sizeof (cpu_set_t), &cpuset);
    }

    template<typename value_t>
    uint8_t clz(value_t value, std::enable_if_t<std::is_unsigned_v<value_t>, int> /* unused */) {
        return __builtin_clzg (value, 0);
    }

    template<typename value_t>
    uint8_t ctz(value_t value, std::enable_if_t<std::is_unsigned_v<value_t>, int> /* unused */) {
        return __builtin_ctzg (value, 0);
    }

#endif

#if defined (LAS_OS_WINDOWS)
    inline futex_wait_result futex_wait (futex_value_t* address, futex_value_t expected_value, std::chrono::milliseconds TIMEOUT) noexcept {
		return WaitOnAddress(address, &expected_value, sizeof(futex_value_t), TIMEOUT.count()) == TRUE ?
            futex_wait_result::awake :
            futex_wait_result::timeout;
    }

    inline void futex_wake_all (futex_value_t * address) noexcept {
        WakeByAddressAll (address);
    }

    inline void futex_wake_one (futex_value_t * address) noexcept {
        WakeByAddressSingle (address);
    }

    inline std::thread::native_handle_type this_thread_native_handle () {
        return GetCurrentThread ();
    }

    inline void thread_affinity_set (std::thread & thread, core_id_t core_id) {
        SetThreadAffinityMask (thread.native_handle(), DWORD_PTR(1 << core_id));
    }

    inline void this_thread_affinity_set (core_id_t core_id) {
        SetThreadAffinityMask (this_thread_native_handle(), DWORD_PTR(1 << core_id));
    }
#endif



}

#endif // LAS_SYSTEM_HPP
