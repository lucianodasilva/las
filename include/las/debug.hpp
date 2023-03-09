#pragma once
#ifndef LAS_DEBUG_HPP
#define LAS_DEBUG_HPP

#include "system.hpp"

#ifdef NDEBUG // if building in release
#	define LAS_DEBUG_BREAK() {}
#	define LAS_DEBUG_ASSERT(__expression__) {}
#	define LAS_THREAD_GUARD(__expected_thread_id__) {}

#else   // if building in debug
        // and in GNU
#	if defined (LAS_COMPILER_GCC) || defined (LAS_COMPILER_CLANG)
#		include <cstddef>
#		include <csignal>
#		define LAS_DEBUG_BREAK() { std::raise (SIGTRAP); }
        // or in MSCV
#	elif defined (LAS_COMPILER_MSVC)
#		define LAS_DEBUG_BREAK() { __debugbreak(); }
#   else
#       define LAS_DEBUG_BREAK() {} // unsupported
# 	endif

#	define LAS_DEBUG_ASSERT(__expression__) \
		{ if (!(__expression__)) { LAS_DEBUG_BREAK(); } }

#	define LAS_THREAD_GUARD(__expected_thread_id__) \
		{ LAS_DEBUG_ASSERT(std::this_thread::get_id() == (__expected_thread_id__)); }

#endif

#endif
