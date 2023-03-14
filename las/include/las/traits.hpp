#pragma once
#ifndef LAS_TRAITS_HPP
#define LAS_TRAITS_HPP

#include <functional>
#include <type_traits>

namespace las {

    template < typename num_t >
    constexpr auto deferred_forward (std::remove_reference_t < num_t > & value,
                                     std::enable_if_t < std::is_reference_v < num_t >, int > = 0) noexcept
    {
        return std::ref (value);
    }

    template < typename num_t >
    constexpr auto deferred_forward (std::remove_reference_t < num_t > & value,
                                     std::enable_if_t < !std::is_reference_v < num_t >, int > = 0) noexcept
    {
        return std::forward <num_t > (value);
    }

    template < typename num_t >
    constexpr auto deferred_forward (std::remove_reference_t < num_t > && value) noexcept {
        return std::forward <num_t > (value);
    }

}

#endif //LAS_TRAITS_HPP
