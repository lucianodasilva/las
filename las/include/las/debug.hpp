#pragma once
#ifndef LAS_DEBUG_HPP
#define LAS_DEBUG_HPP

#include "config.hpp"

#ifdef NDEBUG // if building in release

#	define LAS_DEBUG_BREAK() {}
#	define LAS_DEBUG_ASSERT(__expression__) {}
#	define LAS_THREAD_GUARD(__expected_thread_id__) {}

#else   // if building in debug
        // and in GNU
#	if defined (LAS_COMPILER_GCC) || defined (LAS_COMPILER_CLANG)
#		include <csignal>
#		include <cstddef>
#		define LAS_DEBUG_BREAK() { std::raise (SIGTRAP); }
        // or in MSCV
#	elif defined (LAS_COMPILER_MSVC)
#		define LAS_DEBUG_BREAK() { __debugbreak(); }
#   else
#       define LAS_DEBUG_BREAK() {} // unsupported
# 	endif

/// A macro to assert a condition
/// \param __expression__ the expression to assert
#	define LAS_DEBUG_ASSERT(__expression__) \
		{ if (!(__expression__)) { LAS_DEBUG_BREAK(); } }

/// A macro to guard a block of code to be executed only by a specific thread
/// \param __expected_thread_id__ the expected thread id
#	define LAS_THREAD_GUARD(__expected_thread_id__) \
		{ LAS_DEBUG_ASSERT(std::this_thread::get_id() == (__expected_thread_id__)); }

#endif

#endif
