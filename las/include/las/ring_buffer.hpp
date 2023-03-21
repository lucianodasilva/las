#pragma once
#ifndef LAS_RING_BUFFER_HPP
#define LAS_RING_BUFFER_HPP

#include <memory>
#include <queue>
#include <stack>
#include <type_traits>

#include "las/bits.hpp"

namespace las {

    template<class num_t, typename alloc_t = std::allocator<num_t> >
    struct ring_buffer {
    private:

        using alloc_traits = std::allocator_traits<alloc_t>;
        using alloc_rebind_t = typename alloc_traits::template rebind_alloc<num_t>;

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

            inline iterator(ring_buffer &inst, std::size_t index) :
                    _inst(inst),
                    _index(index) {}

            inline iterator &operator=(iterator const &other) {
                // TODO: check and report for different ring_buffer references
                this->_index = other._index;
                return *this;
            }

            inline iterator &operator++() {
                ++_index;
                return *this;
            }

            inline iterator const &operator++() const {
                ++_index;
                return *this;
            }

            inline iterator operator+(std::size_t offset) const noexcept {
                return {_inst, _index + offset};
            }

            inline std::ptrdiff_t operator-(iterator const &other) const noexcept {
                return _index - other._index;
            }

            inline bool operator==(iterator const &it) const {
                return !(operator!=(it));
            }

            inline bool operator!=(iterator const &it) const {
                return !(&it._inst == &_inst && it._index == _index);
            }

            inline reference operator*() {
                return *_inst.at(_index);
            }

            inline const_reference operator*() const {
                return *_inst.at(_index);
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

        inline explicit ring_buffer(alloc_t const &alloc) :
                buffer_impl_t(alloc) {}

        inline ring_buffer(ring_buffer const &origin) {
            copy_from(origin);
        }

        inline ring_buffer(ring_buffer const &origin, alloc_t const &alloc) :
                buffer_impl_t(alloc) {
            copy_from(origin);
        }

        inline ring_buffer(ring_buffer &&origin) noexcept {
            this->swap(origin);
        }

        inline ring_buffer(ring_buffer &&origin, alloc_t const &alloc) :
                _impl{alloc} {
            this->swap(origin);
        }

        inline ring_buffer &operator=(ring_buffer const &origin) {
            if (&origin != this) {
                copy_from(origin);
            }

            return *this;
        }

        inline ring_buffer &operator=(ring_buffer &&origin) noexcept {
            this->swap(origin);
            return *this;
        }

        inline void swap(ring_buffer &other) {
            std::swap(_index_begin, other._index_begin);
            std::swap(_index_end, other._index_end);

            std::swap(_impl, other._impl);
        }

        inline void clear() {
            range_destroy();

            _impl.self_deallocate();
            _index_begin = 0;
            _index_end = 0;
        }

        inline void reserve(std::size_t n) {
            grow(next_pow_2(n));
        }

        [[nodiscard]] inline bool empty() const noexcept {
            return _index_begin == _index_end;
        }

        [[nodiscard]] inline size_type size() const noexcept {
            return _index_end - _index_begin;
        }

        inline iterator begin() {
            return iterator(*this, 0);
        }

        inline iterator end() {
            return iterator(*this, size());
        }

        inline reference front() noexcept {
            return *at(0);
        }

        inline const_reference front() const noexcept {
            return *at(0);
        }

        inline reference back() noexcept {
            return *at(size() - 1);
        }

        inline const_reference back() const noexcept {
            return *at(size() - 1);
        }

        inline void push_back(const_reference value) {
            emplace_back(value);
        }

        inline void push_back(value_type &&value) {
            emplace_back(std::forward<value_type>(value));
        }

        template<typename ... args_t>
        inline void emplace_back(args_t &&... args) {
            if (is_full()) {
                grow();
            }

            alloc_traits::construct(_impl, at(size()), std::forward<args_t>(args)...);
            ++_index_end;
        }

        inline void push_front(const_reference value) {
            emplace_front(value);
        }

        inline void push_front(value_type &&value) {
            emplace_front(std::forward<value_type>(value));
        }

        template<typename ... args_t>
        inline void emplace_front(args_t &&... args) {
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

            alloc_traits::construct(_impl, at(0), std::forward<args_t>(args)...);
        }

        template<typename input_iterator_t>
        inline void append(input_iterator_t begin_it, input_iterator_t end_it) {

            // TODO: naive implementation, please reimplement
            for (auto it = begin_it; it != end_it; ++it) {
                push_back(*it);
            }
        }

        inline void pop_front() {
            if (empty()) { return; }

            // if !pointer inplace destroy
            if (!std::is_pointer<num_t>()) {
                alloc_traits::destroy(_impl, &front());
            }

            // change index
            ++_index_begin;
        }

        inline void pop_back() {
            if (empty()) { return; }

            // if !pointer inplace destroy
            if (!std::is_pointer<num_t>()) {
                alloc_traits::destroy(_impl, &back());
            }

            // change index
            --_index_end;
        }

        inline void pop_front_n(std::size_t n) {
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

        inline void pop_back_n(std::size_t n) {
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

        [[nodiscard]] inline bool is_full() const noexcept {
            return size() == _impl.capacity();
        }

        [[nodiscard]] inline std::ptrdiff_t transpose_index(std::ptrdiff_t index, std::ptrdiff_t offset = 0) const noexcept {
            return ((index + offset) & _impl.index_mask);
        }

        inline pointer at(std::ptrdiff_t index) {
            return _impl.ptr_begin + transpose_index(_index_begin, index);
        }

        inline const_pointer at(std::ptrdiff_t index) const {
            return _impl.ptr_begin + transpose_index(_index_begin, index);
        }

        inline void grow(size_type n) {
            if (n < _impl.capacity()) {
                return;
            }

            if (n < DEFAULT_MINIMUM_CAPACITY) {
                n = DEFAULT_MINIMUM_CAPACITY;
            }

            auto new_begin = _impl.allocate(n);
            auto data_size = size();

            if (std::is_trivially_copyable < value_type >::value) {
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

        inline void grow() {
            grow(next_pow_2(_impl.capacity() + 1));
        }

        inline void copy_from(ring_buffer const &origin) {
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

        inline void range_destroy() {
            range_apply(
                    [alloc_impl = &_impl](pointer begin_ptr, pointer end_ptr, std::ptrdiff_t) {
                        for (pointer it = begin_ptr; it != end_ptr; ++it) {
                            alloc_traits::destroy(*alloc_impl, it);
                        }
                    },
                    this);
        }

        inline void range_copy(pointer dest) const {
            range_apply(
                    [dest](const_pointer begin_ptr, const_pointer end_ptr, std::ptrdiff_t offset) {
                        std::uninitialized_copy(begin_ptr, end_ptr, dest + offset);
                    },
                    this);
        }

        inline void range_move(pointer dest) {
            range_apply(
                    [dest](pointer begin_ptr, pointer end_ptr, std::ptrdiff_t offset) {
                        // TODO: use when compiler updated
                        // std::uninitialized_move(b, e, dest + offset);
                        auto dst_ptr = dest + offset;

                        for (; begin_ptr != end_ptr; ++begin_ptr, ++dst_ptr) {
                            ::new(dst_ptr) num_t(std::move(*begin_ptr));
                        }
                    },
                    this);
        }

        template<typename func_t, typename inst_t>
        inline static void range_apply(func_t func, inst_t *inst) {
            if (inst->empty()) {
                return;
            }

            auto *begin_ptr = inst->at(0);
            auto *end_ptr = inst->at(inst->size());

            if (end_ptr > begin_ptr) {
                // linear call
                func(begin_ptr, end_ptr, 0);
            } else {
                // split call
                func(begin_ptr, inst->_impl.ptr_capacity, 0);
                func(inst->_impl.ptr_begin, end_ptr, inst->_impl.ptr_capacity - end_ptr);
            }
        }

        struct buffer_impl_t : public alloc_rebind_t {
        public:

            buffer_impl_t() = default;

            explicit buffer_impl_t(
                    alloc_rebind_t const &origin
            ) :
                    alloc_rebind_t(origin) {}

            explicit buffer_impl_t(
                    alloc_rebind_t &&origin
            ) :
                    alloc_rebind_t(std::move(origin)) {}

            inline void swap(buffer_impl_t &other) {
                std::swap(ptr_begin, other.ptr_begin);
                std::swap(ptr_capacity, other.ptr_capacity);
            }

            inline void set(
                    typename ring_buffer::pointer begin,
                    size_type cap) {
                ptr_begin = begin;
                ptr_capacity = ptr_begin + cap;
                index_mask = cap - 1;
            }

            inline void self_allocate(std::size_t n) {
                set(alloc_rebind_t::allocate(n), n);
            }

            inline void self_deallocate() {
                alloc_rebind_t::deallocate(
                        ptr_begin, ptr_capacity - ptr_begin
                );

                set(nullptr, 0);
            }

            inline typename ring_buffer::size_type capacity() const noexcept {
                return ptr_capacity - ptr_begin;
            }

            typename ring_buffer::pointer
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

        static std::size_t constexpr DEFAULT_MINIMUM_CAPACITY = 8;
    };

    template<typename num_t, typename alloc_t = std::allocator<num_t> >
    using queue = std::queue<num_t, ring_buffer<num_t, alloc_t> >;

    template<typename num_t, typename alloc_t = std::allocator<num_t> >
    using stack = std::stack<num_t, ring_buffer<num_t, alloc_t> >;

}
#endif
