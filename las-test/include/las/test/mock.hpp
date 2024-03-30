#pragma once
#ifndef LAS_TEST_MOCK_H
#define LAS_TEST_MOCK_H

#include <functional>
#include <utility>

#include <las/scope_guards.hpp>

namespace las::test {


    class mock_container;

    class basic_mock {
    public:
        virtual ~basic_mock() = default;

        virtual void reset();

        [[nodiscard]]
        bool called_once() const;

        [[nodiscard]]
        inline bool not_called() const;

        [[nodiscard]]
        inline bool has_failed() const;

    protected:

        void inc_call() const;

        void inc_failed_call() const;

    private:
        mutable std::atomic_size_t _call_count{0};
        mutable std::atomic_size_t _failed_call_count{0};
    };

    template<typename signature_t>
    class mock;

    template<typename result_t, typename ... args_t>
    class mock<result_t(args_t...)> : public basic_mock {
    public:
        explicit mock(mock_container *parent, result_t default_return_value = {});
        result_t operator()(args_t &&... args) const;
        void reset() override;

        mutable std::function<result_t(args_t ...)> function;
        mutable result_t                            return_value;

    private:
        result_t                                    _default_return_value;
    };

    template<typename ... args_t>
    class mock<void(args_t...)> : public basic_mock {
    public:
        explicit mock(mock_container *parent);
        void operator()(args_t &&... args) const;
        void reset() override;

        mutable std::function<void(args_t ...)> function;
    };

    template<>
    class mock<void()> : public basic_mock {
    public:
        inline explicit mock(mock_container *parent);
        inline void operator()() const;
        inline void reset() override;

        mutable std::function<void()> function;
    };

    class mock_container {
    public:
        virtual ~mock_container () = default;

        void attach(basic_mock &call_log_ref);

        void reset();

    private:
        std::vector<basic_mock *> _call_logs;
    };

    template<typename result_t, typename ... args_t>
    mock<result_t(args_t...)>::mock(mock_container *parent, result_t default_return_value) :
            _default_return_value {std::move(default_return_value)}
    {
        mock::reset();
        parent->attach(*this);
    }

    template<typename result_t, typename ... args_t>
    result_t mock<result_t(args_t...)>::operator()(args_t &&... args) const {
        [[maybe_unused]] auto guard = scope_exit([&] {
            this->inc_call();
        });

        [[maybe_unused]] auto fail_guard = scope_fail ([&] {
            this->inc_failed_call();
        });

        if (function) {
            return function(std::forward<args_t>(args)...);
        }

        return return_value;
    }

    template<typename result_t, typename... args_t>
    void mock<result_t(args_t...)>::reset() {
        basic_mock::reset ();

        function = {};
        return_value = _default_return_value;
    }

    template<typename ... args_t>
    mock<void(args_t...)>::mock(mock_container *parent) {
        mock::reset();
        parent->attach(*this);
    }

    template<typename ... args_t>
    void mock<void(args_t...)>::operator()(args_t &&... args) const {
        [[maybe_unused]] auto guard = scope_exit([&] {
            this->inc_call();
        });

        [[maybe_unused]] auto fail_guard = scope_fail ([&] {
            this->inc_failed_call();
        });

        if (function) {
            function(std::forward<args_t>(args)...);
        }
    }

    template<typename... args_t>
    void mock<void(args_t...)>::reset() {
        basic_mock::reset ();
        function = {};
    }

    mock<void()>::mock(mock_container *parent) {
        mock::reset();
        parent->attach(*this);
    }

    void mock<void()>::operator()() const {
        [[maybe_unused]] auto guard = scope_exit([&] {
            this->inc_call();
        });

        [[maybe_unused]] auto fail_guard = scope_fail ([&] {
            this->inc_failed_call();
        });

        if (function) {
            function();
        }
    }

    void mock<void()>::reset() {
        basic_mock::reset ();
        function = {};
    }

}

#endif
