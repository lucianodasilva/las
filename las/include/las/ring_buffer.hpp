#pragma once
#ifndef LAS_RING_BUFFER_HPP
#define LAS_RING_BUFFER_HPP

#include <memory>
#include <queue>
#include <stack>
#include <type_traits>

#include "las/bits.hpp"

namespace las {

    template<class value_t, typename alloc_t = std::allocator<value_t> >
    struct ring_buffer {
    private:

        using alloc_traits = std::allocator_traits<alloc_t>;
        using alloc_rebind_t = typename alloc_traits::template rebind_alloc<value_t>;

    public:

        using value_type = typename alloc_traits::value_type;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = typename alloc_traits::pointer;
        using const_pointer = typename alloc_traits::const_pointer;

        using size_type = std::size_t;

        struct iterator {

            using iterator_category = std::forward_iterator_tag;
            using value_type = ring_buffer::value_type;
            using difference_type = std::ptrdiff_t;
            using pointer = ring_buffer::pointer;
            using reference = ring_buffer::reference;

            iterator(ring_buffer &inst, std::size_t index) :
                    _inst(inst),
                    _index(index) {}

            iterator &operator=(iterator const &other) {
                // TODO: check and report for different ring_buffer references
                this->_index = other._index;
                return *this;
            }

            iterator &operator++() {
                ++_index;
                return *this;
            }

            iterator const &operator++() const {
                ++_index;
                return *this;
            }

            iterator operator+(std::size_t offset) const noexcept {
                return {_inst, _index + offset};
            }

            std::ptrdiff_t operator-(iterator const &other) const noexcept {
                return _index - other._index;
            }

            bool operator==(iterator const &it) const {
                return !(operator!=(it));
            }

            bool operator!=(iterator const &it) const {
                return !(&it._inst == &_inst && it._index == _index);
            }

            reference operator*() {
                return *_inst.ptr_to(_index);
            }

            const_reference operator*() const {
                return *_inst.ptr_to(_index);
            }

        private:
            ring_buffer &_inst;
            mutable std::size_t _index;
        };

        using const_iterator = iterator const;

        ~ring_buffer() {
            clear();
        }

        ring_buffer() = default;

        explicit ring_buffer(alloc_t const &alloc) :
                _impl(alloc) {}

        ring_buffer(ring_buffer const &origin) {
            copy_from(origin);
        }

        ring_buffer(ring_buffer const &origin, alloc_t const &alloc) :
                _impl(alloc) {
            copy_from(origin);
        }

        ring_buffer(ring_buffer &&origin) noexcept {
            this->swap(origin);
        }

        ring_buffer(ring_buffer &&origin, alloc_t const &alloc) :
                _impl{alloc} {
            this->swap(origin);
        }

        ring_buffer &operator=(ring_buffer const &origin) {
            if (&origin != this) {
                copy_from(origin);
            }

            return *this;
        }

        ring_buffer &operator=(ring_buffer &&origin) noexcept {
            this->swap(origin);
            return *this;
        }

        void swap(ring_buffer &other)  noexcept {
            std::swap(_index_begin, other._index_begin);
            std::swap(_index_end, other._index_end);

            std::swap(_impl, other._impl);
        }

        void clear() {
            range_destroy();

            _impl.self_deallocate();
            _index_begin = 0;
            _index_end = 0;
        }

        void reserve(std::size_t n) {
            grow(next_pow_2(n));
        }

        [[nodiscard]] bool empty() const noexcept {
            return _index_begin == _index_end;
        }

        [[nodiscard]] size_type size() const noexcept {
            return _index_end - _index_begin;
        }

        iterator begin() {
            return iterator(*this, 0);
        }

        iterator end() {
            return iterator(*this, size());
        }

        reference front() noexcept {
            return *ptr_to(0);
        }

        const_reference front() const noexcept {
            return *ptr_to(0);
        }

        reference back() noexcept {
            return *ptr_to(size() - 1);
        }

        const_reference back() const noexcept {
            return *ptr_to(size() - 1);
        }

        void push_back(const_reference value) {
            emplace_back(value);
        }

        void push_back(value_type &&value) {
            emplace_back(std::forward<value_type>(value));
        }

        template<typename ... args_t>
        void emplace_back(args_t &&... args) {
            if (is_full()) {
                grow();
            }

            alloc_traits::construct(_impl, ptr_to(size()), std::forward<args_t>(args)...);
            ++_index_end;
        }

        void push_front(const_reference value) {
            emplace_front(value);
        }

        void push_front(value_type &&value) {
            emplace_front(std::forward<value_type>(value));
        }

        template<typename ... args_t>
        void emplace_front(args_t &&... args) {
            if (is_full()) {
                grow();
            }

            if (_index_begin == 0) {
                // to simulate folding backwards, shift the transposition frame forward
                auto cap = _impl.capacity();

                _index_begin += cap;
                _index_end += cap;
            }

            --_index_begin;

            alloc_traits::construct(_impl, ptr_to(0), std::forward<args_t>(args)...);
        }

        template<typename input_iterator_t>
        void append(input_iterator_t begin_it, input_iterator_t end_it) {

            // TODO: naive implementation, please reimplement
            for (auto it = begin_it; it != end_it; ++it) {
                push_back(*it);
            }
        }

        void pop_front() {
            if (empty()) { return; }

            // if !pointer inplace destroy
            if (!std::is_pointer<value_t>()) {
                alloc_traits::destroy(_impl, &front());
            }

            // change index
            ++_index_begin;
        }

        void pop_back() {
            if (empty()) { return; }

            // if !pointer inplace destroy
            if (!std::is_pointer<value_t>()) {
                alloc_traits::destroy(_impl, &back());
            }

            // change index
            --_index_end;
        }

        void pop_front_n(std::size_t n) {
            // TODO: naive implementation, please reimplement
            n = std::min(size(), n);

            if (n == size()) {
                clear();
            } else {
                for (std::size_t i = 0; i < n; ++i) {
                    pop_front();
                }
            }
        }

        void pop_back_n(std::size_t n) {
            // TODO: naive implementation, please reimplement
            n = std::min(size(), n);

            if (n == size()) {
                clear();
            } else {
                for (std::size_t i = 0; i < n; ++i) {
                    pop_back();
                }
            }
        }

    private:

        [[nodiscard]] bool is_full() const noexcept {
            return size() == _impl.capacity();
        }

        [[nodiscard]] std::ptrdiff_t transpose_index(std::ptrdiff_t index, std::ptrdiff_t offset = 0) const noexcept {
            return ((index + offset) & _impl.index_mask);
        }

        pointer ptr_to(std::ptrdiff_t index) {
            return _impl.ptr_begin + transpose_index(_index_begin, index);
        }

        const_pointer ptr_to(std::ptrdiff_t index) const {
            return _impl.ptr_begin + transpose_index(_index_begin, index);
        }

        void grow(size_type n) {
            if (n < _impl.capacity()) {
                return;
            }

            if (n < default_minimum_capacity) {
                n = default_minimum_capacity;
            }

            auto new_begin = _impl.allocate(n);
            auto data_size = size();

            if (std::is_trivially_copyable_v < value_type >) {
                range_copy(new_begin);
                range_destroy();
            } else {
            	range_move(new_begin);
            }

            _impl.self_deallocate();
            _impl.set(new_begin, n);

            _index_begin = 0;
            _index_end = data_size;
        }

        void grow() {
            grow(next_pow_2(_impl.capacity() + 1));
        }

        void copy_from(ring_buffer const &origin) {
            // destroy current data
            range_destroy();

            _index_begin = 0;
            _index_end = 0;

            grow(next_pow_2(origin.size()));

            origin.range_copy(_impl.ptr_begin);

            // reset buffer ring
            _index_begin = 0;
            _index_end = origin.size();
        }

        void range_destroy() {
            range_apply(
                    [alloc_impl = &_impl](pointer begin_ptr, pointer end_ptr, std::ptrdiff_t) {
                        for (pointer it = begin_ptr; it != end_ptr; ++it) {
                            alloc_traits::destroy(*alloc_impl, it);
                        }
                    },
                    this);
        }

        void range_copy(pointer dest) const {
            range_apply(
                    [dest](const_pointer begin_ptr, const_pointer end_ptr, std::ptrdiff_t offset) {
                        std::uninitialized_copy(begin_ptr, end_ptr, dest + offset);
                    },
                    this);
        }

        void range_move(pointer dest) {
            range_apply(
                    [dest](pointer begin_ptr, pointer end_ptr, std::ptrdiff_t offset) {
                        // TODO: use when compiler updated
                        std::uninitialized_move(begin_ptr, end_ptr, dest + offset);
//                        auto dst_ptr = dest + offset;
//
//                        for (; begin_ptr != end_ptr; ++begin_ptr, ++dst_ptr) {
//                            ::new(dst_ptr) value_t(std::move(*begin_ptr));
//                        }
                    },
                    this);
        }

        template<typename func_t, typename inst_t>
        static void range_apply(func_t func, inst_t *inst) {
            if (inst->empty()) {
                return;
            }

            auto *begin_ptr = inst->ptr_to(0);
            auto *end_ptr = inst->ptr_to(inst->size());

            if (end_ptr > begin_ptr) {
                // linear call
                func(begin_ptr, end_ptr, 0);
            } else {
                // split call
                func(begin_ptr, inst->_impl.ptr_capacity, 0);
                func(inst->_impl.ptr_begin, end_ptr, inst->_impl.ptr_capacity - end_ptr);
            }
        }

        struct buffer_impl_t : alloc_rebind_t {

            buffer_impl_t() = default;

            explicit buffer_impl_t(
                    alloc_rebind_t const &origin
            ) :
                    alloc_rebind_t(origin) {}

            explicit buffer_impl_t(
                    alloc_rebind_t &&origin
            ) :
                    alloc_rebind_t(std::move(origin)) {}

            void swap(buffer_impl_t &other)  noexcept {
                std::swap(ptr_begin, other.ptr_begin);
                std::swap(ptr_capacity, other.ptr_capacity);
            }

            void set(
                    pointer begin,
                    size_type cap) {
                ptr_begin = begin;
                ptr_capacity = ptr_begin + cap;
                index_mask = cap - 1;
            }

            void self_allocate(std::size_t n) {
                set(alloc_rebind_t::allocate(n), n);
            }

            void self_deallocate() {
                alloc_rebind_t::deallocate(
                        ptr_begin, ptr_capacity - ptr_begin
                );

                set(nullptr, 0);
            }

            [[nodiscard]] size_type capacity() const noexcept {
                return ptr_capacity - ptr_begin;
            }

            pointer
                    ptr_begin{nullptr},
                    ptr_capacity{nullptr};

            uintptr_t
                    index_mask{0};
        };

        buffer_impl_t
                _impl;

        size_type
                _index_begin{0},
                _index_end{0};

        static std::size_t constexpr default_minimum_capacity = 8;
    };

    template<typename value_t, typename alloc_t = std::allocator<value_t> >
    using queue = std::queue<value_t, ring_buffer<value_t, alloc_t> >;

    template<typename value_t, typename alloc_t = std::allocator<value_t> >
    using stack = std::stack<value_t, ring_buffer<value_t, alloc_t> >;

}
#endif
