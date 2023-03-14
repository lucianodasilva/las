#pragma once
#ifndef LAS_SYSTEM_HPP
#define LAS_SYSTEM_HPP

namespace las {

    enum struct os_t {
        unknown = 0,
        windows,
        gnu_linux,
        android,
        darwin
    };

    enum struct compiler_t {
        unknown = 0,
        gcc,
        msvc,
        clang
    };

    enum struct arch_t {
        unknown = 0,
        x86,
        x64,
        arm,
        arm64
    };

    // --- config info macros ---
    // os
#if defined (__linux__) && !defined(__ANDROID__)
#	define LAS_OS_GNU_LINUX
    constexpr os_t os{os_t::gnu_linux};
#elif defined (__ANDROID__)
    #	define LAS_OS_ANDROID
	constexpr os_t os { os_t::android};
#elif defined (__APPLE__)
#	define LAS_OS_DARWIN
	constexpr os_t os { os_t::darwin };
#elif defined (_WIN32)
#	define LAS_OS_WINDOWS
	constexpr os_t os { os_t::windows };
#else
	constexpr os_t os { os_t::unknown };
#endif

// compiler
#if defined (__GNUC__)
#	define LAS_COMPILER_GCC
    constexpr compiler_t compiler{compiler_t::gcc};
#elif defined (_MSC_VER)
    #	define LAS_COMPILER_MSVC
	constexpr compiler_t compiler { compiler_t::msvc };
#elif defined (__clang__)
#	define LAS_COMPILER_CLANG
	constexpr compiler_t compiler { compiler_t::clang };
#else
	constexpr compiler_t compiler { compiler_t::unknown };
#endif

// build architecture
#if defined (__x86_64__) || defined (_M_X64)
#	define LAS_ARCH_X64
    constexpr arch_t arch{arch_t::x64};
#elif defined (__i386) || defined (_M_IX86)
    #	define LAS_ARCH_X86
	constexpr arch_t arch { arch_t::x86 };
#elif defined (__arm__) || defined (_M_ARM)
#	define LAS_ARCH_ARM
	constexpr arch_t arch { arch_t::arm };
#elif defined (__aarch64__) || defined (_M_ARM64)
#	define LAS_ARCH_ARM64
	constexpr arch_t arch { arch_t::arm64 };
#else
	constexpr arch_t arch { arch_t::unknown };
#endif

}

#endif //LAS_SYSTEM_HPP
