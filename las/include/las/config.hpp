#pragma once
#ifndef LAS_CONFIG_HPP
#define LAS_CONFIG_HPP

// Detect operating system
#if defined (__linux__) && !defined(__ANDROID__)
#	define LAS_OS_GNU_LINUX 1
#elif defined (__ANDROID__)
#	define LAS_OS_ANDROID 1
#elif defined (__APPLE__)
#	define LAS_OS_DARWIN 1
#elif defined (_WIN32)
#	define LAS_OS_WINDOWS 1
#endif

// Detect compiler
#if defined (__GNUC__)
#	define LAS_COMPILER_GCC 1
#elif defined (_MSC_VER)
#	define LAS_COMPILER_MSVC 1
#elif defined (__clang__)
#	define LAS_COMPILER_CLANG 1
#endif

// Detect architecture
#if defined (__x86_64__) || defined (_M_X64)
#	define LAS_ARCH_X64 1
#elif defined (__i386) || defined (_M_IX86)
#	define LAS_ARCH_X86 1
#elif defined (__arm__) || defined (_M_ARM)
#	define LAS_ARCH_ARM 1
#elif defined (__aarch64__) || defined (_M_ARM64)
#	define LAS_ARCH_ARM64 1
#endif

// Detect endianness
#if defined (__BYTE_ORDER__) && defined (__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#       define LAS_LITTLE_ENDIAN 1
#elif  defined (__BYTE_ORDER__) && defined (__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#       define LAS_BIG_ENDIAN 1
#elif defined (LAS_OS_WINDOWS)
#       define LAS_LITTLE_ENDIAN 1
#endif

namespace las {

    enum struct os : uint8_t{
        unknown = 0,
        windows,
        gnu_linux,
        android,
        darwin,
#if defined (LAS_OS_GNU_LINUX)
        native = gnu_linux,
#elif defined (LAS_OS_ANDROID)
        native = android,
#elif defined (LAS_OS_DARWIN)
        native = darwin,
#elif defined (LAS_OS_WINDOWS)
        native = windows,
#else
        native = unknown,
#endif
    };

    enum struct compiler : uint8_t {
        unknown = 0,
        gcc,
        msvc,
        clang,
#if defined (LAS_COMPILER_GCC)
        native = gcc,
#elif defined (LAS_COMPILER_MSVC)
        native = msvc,
#elif defined (LAS_COMPILER_CLANG)
        native = clang,
#else
        native = unknown
#endif
    };

    enum struct arch : uint8_t {
        unknown = 0,
        x86,
        x64,
        arm,
        arm64,
#if defined (LAS_ARCH_X86)
        native = x86,
#elif defined (LAS_ARCH_X64)
        native = x64,
#elif defined (LAS_ARCH_ARM)
        native = arm,
#elif defined (LAS_ARCH_ARM64)
        native = arm64
#else
        native = unknown
#endif
    };

    enum struct endian : uint8_t {
        unknown = 0,
        big,
        little,
#if defined (LAS_BIG_ENDIAN)
        native = big,
#elif defined (LAS_LITTLE_ENDIAN)
        native = little,
#else
        native = unknown,
#endif
    };

}

#endif //LAS_CONFIG_HPP
