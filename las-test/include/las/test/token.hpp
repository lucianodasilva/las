#pragma once
#ifndef LAS_TEST_TOKEN_HPP
#define LAS_TEST_TOKEN_HPP

#include <atomic>
#include <memory>
#include <utility>

#include <las/traits.hpp>

namespace las::test {


    class token_counters {
    public:

        inline void reset() {
            _copies.store(0);
            _moves.store(0);
            _ctors.store(0);
            _dtors.store(0);
        }

        inline void inc_copy () { ++_copies; }

        [[nodiscard]] inline bool check_copies(std::size_t count) const {
            return _copies.load () == count;
        }

        inline void inc_move () { ++_moves;}

        [[nodiscard]] inline bool check_moves(std::size_t count) const {
            return _moves.load () == count;
        }

        inline void inc_ctor () { ++_ctors; }

        [[nodiscard]] inline bool check_ctors(std::size_t count) const {
            return _ctors.load () == count;
        }

        inline void inc_dtor() { ++_dtors; }

        [[nodiscard]] inline bool check_dtors(std::size_t count) const {
            return _dtors.load () == count;
        }

    private:

        std::atomic_size_t _copies{0};
        std::atomic_size_t _moves{0};
        std::atomic_size_t _ctors{0};
        std::atomic_size_t _dtors{0};
    };

    // structures to be moved
    template < typename value_t >
    struct token {

        using value_type = std::decay_t< value_t >;
        using reference = value_type &;
        using const_reference = value_type const &;

        template < typename ... args_t >
        inline explicit token(std::shared_ptr < token_counters > counters, args_t && ... args) :
            _value (std::forward < args_t > (args)...),
            _counters (std::move(counters))
        {
            this->counters ().inc_ctor ();
        }

        template <typename ... args_t >
        inline explicit token(args_t && ... args) :
            token(
                    std::make_shared < token_counters > (),
                    std::forward < args_t > (args)...)
        {}

        ~token() {
            this->counters ().inc_dtor ();
        }

        inline token(token && other) noexcept {
            operator=(std::forward<token>(other));
        }

        inline token(const token & other) noexcept {
            operator=(other);
        }

        // "super specialization" to avoid being caught by
        // the 'a little too universal constructor'
        inline token(token & other) noexcept :
            token (static_cast < token const & > (other))
        {}

        inline token &operator=(const token &other) {
            if (&other == this) {
                return *this;
            }

            this->_counters = other._counters;
            this->_value = other._value;

            this->counters().inc_copy ();

            return *this;
        }

        inline token &operator=(token &&other) noexcept {
            this->swap(other);

            // well... both were moved
            other.counters().inc_move();
            this->counters ().inc_move();

            return *this;
        }

        inline void swap(token &other) {
            std::swap(this->_counters, other._counters);
            std::swap(this->_value, other._value);
        }

        [[nodiscard]]
        inline token_counters & counters () { return *_counters; }

        [[nodiscard]]
        inline token_counters const & counters () const { return *_counters; }

        inline reference value () { return _value; }
        inline const_reference value () const { return _value; }

    private:

        value_type                          _value {};
        std::shared_ptr < token_counters >  _counters
            = std::make_shared < token_counters > ();
    };

    template < typename value_t >
    inline bool operator==(token < value_t > const & lhv, token < value_t > const & rhv) {
        return lhv.value() == rhv.value();
    }

}

#endif
