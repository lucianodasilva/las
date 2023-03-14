#pragma once
#ifndef LAS_IP_LOCK_HPP
#define LAS_IP_LOCK_HPP

#include "las/details.hpp"

#include <memory>
#include <string_view>

namespace las {

    // Inter Process Exclusive Lock
    struct ip_lock : no_copy {
    public:

        explicit ip_lock(std::string_view lock_name);

        ~ip_lock ();

        explicit operator bool () const;

    private:

        struct internal;
        std::unique_ptr < internal > _internal;
    };

}

#endif
