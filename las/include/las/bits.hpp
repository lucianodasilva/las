#pragma once
#ifndef LAS_BITS_HPP
#define LAS_BITS_HPP

#include <type_traits>

namespace las {

    template<typename num_t>
    constexpr inline bool is_pow_2(num_t num) {
        static_assert(std::is_unsigned<num_t>::value, "is_pow_2 does not support signed data types!");
        return (num & (num - 1)) == 0;
    }

    template<class num_t>
    constexpr inline num_t next_pow_2(num_t value) {
        static_assert(std::is_unsigned<num_t>::value, "next_pow_2 does not support signed data types!");

        if (is_pow_2(value)) {
            return value;
        }

        constexpr num_t BIT_CEIL{sizeof(num_t) * 8U};
        num_t bit = 1U;

        while (bit < BIT_CEIL) {
            value |= (value >> bit);
            bit = bit << 1U;
        }

        return value + 1U;
    }

}

#endif //LAS_BITS_HPP
