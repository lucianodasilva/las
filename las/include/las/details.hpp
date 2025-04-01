#pragma once
#ifndef LAS_DETAILS_HPP
#define LAS_DETAILS_HPP

#include "config.hpp"

namespace las {

    class no_copy {
    protected:
        constexpr no_copy() = default;
        ~no_copy() = default;
    public:
        no_copy(const no_copy &) = delete;
        no_copy &operator=(const no_copy &) = delete;
    };

    class no_move {
    protected:
        constexpr no_move() = default;
        ~no_move() = default;
    public:
        no_move(no_move &&) = delete;
        no_move &operator=(no_move &&) = delete;
    };

    /* minimalistic type-safe value wrapper
     * use as:
     * struct [type_name] : type_safe < [wrapped type] > {
     * 		using type_safe <[wrapped type]>::type_safe;
     * };
     */
    template < typename num_t >
    struct type_safe {
        using value_type = num_t;

        type_safe () = default;

        explicit type_safe (num_t const & val) :
            value { val }
        {}

        value_type value {};
    };

    // force inline marker
    #if defined (LAS_COMPILER_GCC) || defined (LAS_COMPILER_CLANG)
    #	define LAS_FORCE_INLINE [[gnu::always_inline]]
    #elif defined (LAS_COMPILER_MSVC)
    #	define LAS_FORCE_INLINE __forceinline
    #else
    #	define LAS_FORCE_INLINE inline
    #endif

}

#endif
