#pragma once
#ifndef LAS_DETAILS_HPP
#define LAS_DETAILS_HPP

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
    public:
        using value_type = num_t;

        type_safe () = default;

        inline explicit type_safe (num_t const & val) :
                value { val }
        {}

        value_type value {};
    };

}

#endif
