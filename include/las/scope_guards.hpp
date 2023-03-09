#pragma once
#ifndef LAS_SCOPE_GUARDS_HPP
#define LAS_SCOPE_GUARDS_HPP

#include <exception>

namespace las {

    template<typename func_t>
    struct basic_scope_guard {
    public:

        basic_scope_guard() = delete;
        basic_scope_guard(const basic_scope_guard &) = delete;

        basic_scope_guard &operator=(const basic_scope_guard &) = delete;
        basic_scope_guard &operator=(const basic_scope_guard &&) noexcept = delete;

        inline basic_scope_guard(basic_scope_guard && other) noexcept:
                _active(other._active),
                _func(std::move(other._func)) {
            other.dismiss();
        }

        inline explicit basic_scope_guard(func_t && func) :
                _active(true),
                _func(std::forward<func_t>(func)) {}


        inline void trigger() {
            if (_active) {
                _func();
            }

            _active = false;
        }

        inline void dismiss() { _active = false; }

    private:
        bool _active;
        func_t _func;
    };

    template<typename func_t>
    struct scope_exit : public basic_scope_guard<func_t> {
    public:

        using basic_scope_guard<func_t>::basic_scope_guard;

        ~scope_exit() {
            this->trigger();
        }
    };

    template<typename func_t>
    struct scope_success : public basic_scope_guard<func_t> {
    public:

        using basic_scope_guard<func_t>::basic_scope_guard;

        ~scope_success() {
            if (std::uncaught_exceptions() == 0) {
                this->trigger();
            }
        }
    };

    template<typename func_t>
    struct scope_fail : public basic_scope_guard<func_t> {
    public:

        using basic_scope_guard<func_t>::basic_scope_guard;

        ~scope_fail() {
            if (std::uncaught_exceptions() != 0) {
                this->trigger();
            }
        }
    };

}

#endif
