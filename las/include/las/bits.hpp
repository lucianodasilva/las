#pragma once
#ifndef LAS_BITS_HPP
#define LAS_BITS_HPP

#include <cinttypes>
#include <type_traits>

#include <las/system.hpp>

namespace las {

#pragma region "POW2 Values"

    /// Check if value is a power-of-two
    /// \tparam num_t Unsigned integer type
    template<typename num_t>
    constexpr inline bool is_pow_2(num_t num) {
        static_assert(std::is_unsigned<num_t>::value, "is_pow_2 does not support signed data types!");
        return (num & (num - 1)) == 0;
    }

    /// Calculate the next power-of-two of a power-of-two value
    /// \tparam Unsigned integer type
    /// \param num Value to check
    /// \return The next pow2 following the value parameter. If the value itself is a pow2, then the same value will be returned.
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

    /// Swaps the byte order of a integer value
    template < typename type_t >
    inline type_t byte_swap (type_t value) noexcept {
        static_assert(std::is_integral<num_t>::value, "byte_swap only supports integers!");
        return static_cast < type_t > (
                byte_swap (
                        static_cast < typename std::make_unsigned < type_t >::type >(
                                value)));
    }

    /// Converts value endianess from the 'native' the 'expected' endianness
    /// \tparam type_t Value type
    /// \tparam ENDIAN expected endianness
    /// \return Value with bytes ordered as the expected endianness
    template < las::endian ENDIAN,typename type_t >
    inline type_t endian_cast (type_t value) {
        if constexpr (ENDIAN == las::endian::native || ENDIAN == las::endian::unknown) {
            return value;
        } else {
            return byte_swap (value);
        }
    }

#pragma endregion

#pragma region "Bit set/clear and read"

    template < typename value_t = uint8_t, typename ... bits_t >
    inline value_t bitmask(bits_t const ... BITS) noexcept {
        return ((value_t(0x1) << BITS) | ...);
    }

    template < typename value_t, typename ... bits_t >
    inline value_t bit_set(value_t VALUE, bits_t const ... BITS) noexcept {
        return VALUE | bitmask(BITS...);
    }

    template < typename value_t, typename ... bits_t >
    inline value_t bit_clear(value_t VALUE, bits_t const ... BITS) noexcept {
        return VALUE & ~bitmask(BITS...);
    }

    template < typename value_t >
    inline bool is_bit_set(value_t const VALUE, uint8_t const BIT) noexcept {
        return (VALUE & bitmask(BIT)) != value_t ();
    }

#pragma endregion

}

#endif //LAS_BITS_HPP
