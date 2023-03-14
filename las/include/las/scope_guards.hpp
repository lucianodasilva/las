#pragma once
#ifndef LAS_SCOPE_GUARDS_HPP
#define LAS_SCOPE_GUARDS_HPP

#include <atomic>
#include <exception>

namespace las {

    namespace details {
        template<typename func_t>
        struct basic_scope_guard {
        public:

            basic_scope_guard() = delete;

            basic_scope_guard(const basic_scope_guard &) = delete;

            basic_scope_guard &operator=(const basic_scope_guard &) = delete;

            basic_scope_guard &operator=(const basic_scope_guard &&) noexcept = delete;

            inline basic_scope_guard(basic_scope_guard &&other) noexcept:
                    _active(other._active),
                    _func(std::move(other._func)) {
                other.dismiss();
            }

            inline explicit basic_scope_guard(func_t &&func) :
                    _active(true),
                    _func(std::forward<func_t>(func)) {}


            inline void trigger() {
                bool expected_active = true;

                if (_active.compare_exchange_strong(expected_active, false)) {
                    _func();
                }
            }

            inline void dismiss() { _active = false; }

        private:
            std::atomic_bool _active;
            func_t _func;
        };

        template<typename func_t>
        struct scope_exit : public basic_scope_guard<func_t> {
        public:

            using basic_scope_guard<func_t>::basic_scope_guard;

            inline ~scope_exit() {
                this->trigger();
            }
        };

        template<typename func_t>
        struct scope_success : public basic_scope_guard<func_t> {
        public:

            using basic_scope_guard<func_t>::basic_scope_guard;

            inline ~scope_success() {
                if (std::uncaught_exceptions() == 0) {
                    this->trigger();
                }
            }
        };

        template<typename func_t>
        struct scope_fail : public basic_scope_guard<func_t> {
        public:

            using basic_scope_guard<func_t>::basic_scope_guard;

            inline ~scope_fail() {
                if (std::uncaught_exceptions() != 0) {
                    this->trigger();
                }
            }
        };
    }

    template < typename func_t >
    inline auto scope_exit (func_t && func) {
        return details::scope_exit < func_t > { std::forward < func_t > (func) };
    }

    template < typename func_t >
    inline auto scope_success (func_t && func) {
        return details::scope_success < func_t > { std::forward < func_t > (func) };
    }

    template < typename func_t >
    inline auto scope_fail (func_t && func) {
        return details::scope_fail < func_t > { std::forward < func_t > (func) };
    }

}

#endif
