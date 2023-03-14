#pragma once
#ifndef LAS_LOCKED_VALUE_HPP
#define LAS_LOCKED_VALUE_HPP

#include <shared_mutex>
#include <mutex>

namespace las {

    /// Provides a concurrently restricted structure that encapsulates a value and
    /// the mutex/lock combinations needed to access its value
    /// \tparam value_t Value Type
    /// \tparam mutex_t Mutex Type (Defaults to: std::shared_mutex)
    template < typename num_t, typename mutex_t = std::shared_mutex >
    struct locked_value {
    public:

        /// Represents a value reference and its corresponding lock
        /// \tparam lock_t Lock Type
        /// \tparam reference_t Reference Type
        template < typename lock_t, typename reference_t >
        struct locked_ref {
        public:

            inline locked_ref(lock_t && lock, reference_t ref) :
                    _lock(std::forward < lock_t >(lock)),
                    _ref(ref)
            {}

            /// LockedValue wrapped variable accessor
            /// \return Reference to the contained variable
            inline reference_t  value() { return _ref; }

            inline lock_t &     lock() { return _lock; }

        private:
            lock_t      _lock;
            reference_t _ref;
        };

        locked_value(locked_value const&) = delete;
        locked_value(locked_value &&) noexcept = delete;

        locked_value & operator = (locked_value const &) = delete;
        locked_value & operator = (locked_value &&) noexcept = delete;

        /// Perfect forwarding constructor. Arguments will be forwarded to the wrapped variable's constructor
        template < typename ... args_t >
        inline explicit locked_value(args_t && ... args) :
                _value{ std::forward < args_t >(args)... }
        {}

        /// Get an exclusive locked reference (read/write)
        inline auto unique() {
            return locked_ref < std::unique_lock < mutex_t >, num_t & >{
                std::unique_lock < mutex_t > {_mutex},
                _value };
        }

        /// Get an exclusive locked reference (read/write)
        /// \param tag behaviour tagging strategy
        template < typename tag_t >
        inline auto unique(tag_t tag) {
            return locked_ref < std::unique_lock < mutex_t >, num_t & >{
                std::unique_lock < mutex_t > {_mutex, tag},
                        _value };
        }

        /// Get a shared locked reference (read)
        inline auto shared () const {
            return locked_ref < std::shared_lock < mutex_t >, num_t const & > {
                std::shared_lock < mutex_t >{_mutex},
                _value};
        }

        /// Get a shared locked reference (read)
        /// \param tag behaviour tagging strategy
        template < typename tag_t >
        inline auto shared (tag_t tag) const {
            return locked_ref < std::shared_lock < mutex_t >, num_t const & > {
                    std::shared_lock < mutex_t >{_mutex, tag},
                    _value};
        }

    private:
        num_t           _value;
        mutable mutex_t _mutex;
    };

}

#endif
