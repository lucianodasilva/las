#pragma once
#ifndef LAS_FLAG_HPP
#define LAS_FLAG_HPP

#include <type_traits>

namespace las {

    template < typename enum_t >
    struct flag {
    public:

        static_assert (std::is_enum_v < enum_t >, "flag type supports only enum types");

        using underlying_type = typename std::underlying_type < enum_t >::type;

        constexpr flag () = default;
        constexpr flag (flag const &) = default;

        constexpr flag (enum_t const & value_v) : // NOLINT
                value {value_v }
        {}

        constexpr flag & operator = (enum_t const & other_v) {
            value = other_v;
            return *this;
        }

        constexpr operator enum_t () const { return value; } // NOLINT

        [[nodiscard]]
        constexpr bool contains (enum_t value_v) const {
            return 	(get_as_underlying(value) & get_as_underlying(value_v))
                      == get_as_underlying(value_v);
        }

        constexpr flag & append (enum_t value_v) {
            value = static_cast < enum_t > (get_as_underlying(value) | get_as_underlying(value_v));
            return *this;
        }

        enum_t value {};

        constexpr static underlying_type get_as_underlying (enum_t value_v) noexcept {
            return static_cast < underlying_type > (value_v);
        }
    };

}

#endif //LAS_FLAG_HPP
