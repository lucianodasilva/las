#pragma once
#ifndef LAS_BITS_HPP
#define LAS_BITS_HPP

#include <cinttypes>
#include <type_traits>

#include <las/system.hpp>

namespace las {

#pragma region "POW2 Values"

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

#pragma endregion

#pragma region "byte swap and endianness"

#ifdef LAS_COMPILER_MSVC

    inline uint16_t byte_swap (uint16_t value) noexcept {
        return _byteswap_ushort(value);
    }

    inline uint32_t byte_swap (uint32_t value) noexcept {
        return _byteswap_ulong(value);
    }

    inline uint64_t byte_swap (uint64_t value) noexcept {
        return _byteswap_uint64(value);
    }

#else

    inline uint16_t byte_swap (uint16_t value) noexcept {
        return __builtin_bswap16(value);
    }

    inline uint32_t byte_swap (uint32_t value) noexcept {
        return __builtin_bswap32(value);
    }

    inline uint64_t byte_swap (uint64_t value) noexcept {
        return __builtin_bswap64(value);
    }

#endif

    inline uint8_t byte_swap (uint8_t value) noexcept {
        return value;
    }

    template < typename type_t >
    inline type_t byte_swap (type_t value) noexcept {
        return static_cast < type_t > (
                byte_swap (
                        static_cast < typename std::make_unsigned < type_t >::type >(
                                value)));
    }

    template < las::endian ENDIAN,typename type_t >
    inline type_t endian_cast (type_t value) {
        if constexpr (ENDIAN == las::endian::native || ENDIAN == las::endian::unknown) {
            return value;
        } else {
            return byte_swap (value);
        }
    }

#pragma endregion

}

#endif //LAS_BITS_HPP
