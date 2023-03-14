#pragma once
#ifndef LAS_SMALL_VECTOR_H
#define LAS_SMALL_VECTOR_H

#include <iterator>
#include <limits>
#include <memory>

#include "bits.hpp"

namespace las {

    namespace details {

        template<class type_t>
        using is_iterator = std::negation<std::is_integral<type_t> >;

        template<class type_t, size_t SIZE>
        struct static_storage {

            alignas(next_pow_2(sizeof(type_t))) uint8_t data[sizeof(type_t) * SIZE];

            inline type_t &operator[](size_t index) {
                return *reinterpret_cast <type_t *> (data + (index * sizeof(type_t)));
            }

            inline type_t const &operator[](size_t index) const {
                return *reinterpret_cast <type_t const *> (data + (index * sizeof(type_t)));
            }

            inline const type_t *begin() const noexcept {
                return reinterpret_cast <const type_t *> (+data);
            };

            inline type_t *begin() noexcept {
                return reinterpret_cast <type_t *> (+data);
            };

            inline type_t *end() noexcept {
                return begin() + SIZE;
            }

            inline const type_t *end() const noexcept {
                return begin() + SIZE;
            }
        };

    }

    template<typename type_t, typename alloc_t = std::allocator<type_t> >
    struct small_vector_base {
    private:
        using alloc_traits_t = std::allocator_traits<alloc_t>;
        using alloc_rebind_t = typename std::allocator_traits<alloc_t>::template rebind_alloc<type_t>;
    public:

        using value_type = typename alloc_traits_t::value_type;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = typename alloc_traits_t::pointer;
        using const_pointer = typename alloc_traits_t::const_pointer;

        using size_type = typename alloc_traits_t::size_type;
        using difference_type = typename alloc_traits_t::difference_type;

        using iterator = pointer;
        using const_iterator = const_pointer;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        using allocator_type = alloc_t;

        inline iterator begin() noexcept { return _impl._begin_ptr; }

        inline const_iterator begin() const noexcept { return _impl._begin_ptr; }

        inline iterator end() noexcept { return _impl._end_ptr; }

        inline const_iterator end() const noexcept { return _impl._end_ptr; }

        inline reverse_iterator rbegin() noexcept { return --end(); }

        inline const_reverse_iterator rbegin() const noexcept { return --end(); }

        inline reverse_iterator rend() noexcept { return --begin(); }

        inline const_reverse_iterator rend() const noexcept { return --begin(); }

        inline const_iterator cbegin() const noexcept { return begin(); }

        inline const_iterator cend() const noexcept { return end(); }

        inline const_reverse_iterator crbegin() const noexcept { return rbegin(); }

        inline const_reverse_iterator crend() const noexcept { return rend(); }

        inline allocator_type get_allocator() const {
            return _impl;
        }

        inline size_type size() const noexcept {
            return _impl._end_ptr - _impl._begin_ptr;
        }

        inline size_type max_size() const noexcept { return std::numeric_limits<size_t>::max(); }

        inline size_type capacity() const noexcept {
            return _impl._capacity_ptr - _impl._begin_ptr;
        }

        [[nodiscard]]
        inline bool empty() const noexcept {
            return _impl._begin_ptr == _impl._end_ptr;
        }

        inline reference front() {
            if (empty()) {
                throw std::runtime_error("front() called for empty vector");
            }
            return begin()[0];
        }

        inline const_reference front() const {
            if (empty()) {
                throw std::runtime_error("front() called for empty vector");
            }
            return begin()[0];
        }

        inline reference back() {
            if (empty()) {
                throw std::runtime_error("back() called for empty vector");
            }
            return *(_impl._end_ptr - 1);
        }

        inline const_reference back() const {
            if (empty()) {
                throw std::runtime_error("back() called for empty vector");
            }
            return *(_impl._end_ptr - 1);
        }

        inline reference operator[](size_type n) {
            return begin()[n];
        }

        inline const_reference operator[](size_type n) const {
            return begin()[n];
        }

        small_vector_base &operator=(const small_vector_base &other) {
            if (this == &other) {
                return *this;
            }

            assign(other.begin(), other.end());

            return *this;
        }

        small_vector_base &operator=(std::initializer_list<value_type> init_list) {
            assign(init_list);
            return *this;
        }

        small_vector_base &operator=(small_vector_base &&other) noexcept {
            swap(other);
            return *this;
        }

        inline void reserve(size_type size) {
            if (size > capacity()) {
                grow_near_pow_2(size);
            }
        }

        inline void shrink_to_fit() noexcept {
            shrink(size());
        }

        template<class input_it_t>
        inline std::enable_if_t<details::is_iterator<input_it_t>::value, void>
        assign(input_it_t first, input_it_t last) {
            clear();

            size_type elements = last - first;

            if (capacity() < elements) {
                grow_near_pow_2(elements);
            }

            _impl._end_ptr = (begin() + elements);
            std::uninitialized_copy(first, last, begin());
        }

        inline void assign(size_type n, const_reference value) {
            clear();

            if (capacity() < n) {
                grow_near_pow_2(n);
            }

            _impl._end_ptr = begin() + n;
            std::uninitialized_fill(begin(), end(), value);
        }

        inline void assign(std::initializer_list<value_type> init_list) {
            assign(init_list.begin(), init_list.end());
        }

        inline reference at(size_type n) {
            if (n >= size()) {
                throw std::out_of_range("SmallVector out of range");
            }
            return begin()[n];
        }

        inline const_reference at(size_type n) const {
            if (n >= size()) {
                throw std::out_of_range("SmallVector out of range");
            }
            return begin()[n];
        }

        inline void clear() noexcept {
            destroy_backward(begin(), end());
            _impl._end_ptr = (begin());
        }

        pointer data() noexcept {
            return _impl._begin_ptr;
        }

        const_pointer data() const noexcept {
            return _impl._begin_ptr;
        }

        inline void push_back(const_reference value) {
            if (_impl._end_ptr >= _impl._capacity_ptr) {
                grow();
            }

            alloc_traits_t::construct(_impl, _impl._end_ptr, value);
            ++_impl._end_ptr;
        }

        inline void push_back(value_type &&value) {
            if (_impl._end_ptr >= _impl._capacity_ptr) {
                grow();
            }

            alloc_traits_t::construct(_impl, _impl._end_ptr, std::forward<value_type>(value));
            ++_impl._end_ptr;
        }

        template<class... args_t>
        inline reference emplace_back(args_t &&... args) {
            if (_impl._end_ptr >= _impl._capacity_ptr) {
                grow();
            }

            alloc_traits_t::construct(_impl, _impl._end_ptr, std::forward<args_t>(args)...);
            ++_impl._end_ptr;

            return back();
        }

        inline void pop_back() {
            if (empty()) {
                return;
            }

            alloc_traits_t::destroy(_impl, _impl._end_ptr - 1);
            --_impl._end_ptr;

            if (next_pow_2(size() - 1) < capacity()) {
                shrink_to_fit();
            }
        }

        template<class ... args_t>
        inline iterator emplace(const_iterator position, args_t &&... args) {
            if (position == end()) {
                emplace_back(std::forward<args_t>(args)...);
                return end() - 1;
            }

            if (position < begin() || position > end()) {
                throw std::out_of_range("emplace () position out of range");
            }

            size_type offset = position - begin();

            if (_impl._end_ptr == _impl._capacity_ptr) {
                grow();
            }

            iterator place = begin() + offset;

            // move items forward
            std::move_backward(place, end(), end() + 1);

            // emplace items
            alloc_traits_t::construct(_impl, place, std::forward<args_t>(args)...);

            ++_impl._end_ptr;

            return place;
        }

        inline iterator insert(const_iterator position, const_reference value) {
            return emplace(position, value);
        }

        inline iterator insert(const_iterator position, value_type &&value) {
            return emplace(position, std::move(value));
        }

        inline iterator insert(const_iterator position, size_type n, const_reference value) {
            size_type offset = position - begin();

            if (n == 0) {
                return begin() + offset;
            }

            if (position < begin() || position > end()) {
                throw std::out_of_range("insert () position out of range");
            }

            if (capacity() < (size() + n)) {
                grow_near_pow_2(size() + n);
            }

            iterator place = begin() + offset;

            // move items forward
            std::move_backward(place, end(), end() + n);
            std::uninitialized_fill_n(place, n, value);

            _impl._end_ptr = (_impl._end_ptr + n);

            return place;
        }

        template<class input_it_t>
        inline std::enable_if_t<details::is_iterator<input_it_t>::value, iterator>
        insert(const_iterator position, input_it_t first, input_it_t last) {

            size_type len = last - first;
            size_type offset = position - begin();

            if (position < begin() || position > end()) {
                throw std::out_of_range("insert () position out of range");
            }

            if (capacity() < (size() + len)) {
                grow_near_pow_2(size() + len);
            }

            auto place = begin() + offset;

            // move items forward
            std::move_backward(place, end(), end() + len);
            std::copy(first, last, begin() + offset);

            _impl._end_ptr = (_impl._end_ptr + len);

            return place;
        }

        inline iterator insert(const_iterator position, std::initializer_list<value_type> init_list) {
            return insert(position, init_list.begin(), init_list.end());
        }

        inline iterator erase(const_iterator position) {

            if (position < begin() || position >= end()) {
                throw std::out_of_range("erase () position out of range");
            }

            size_type offset = position - begin();
            iterator pos = begin() + offset;

            // is last item
            if (pos == end() - 1) {
                pop_back();
                return end();
            }

            std::move(pos + 1, end(), pos);
            pop_back();

            return begin() + offset;
        }

        inline iterator erase(const_iterator first, const_iterator last) {

            if (first < begin() || first >= end()) {
                throw std::out_of_range("erase () out of range");
            }

            if (last < first) {
                throw std::out_of_range("erase () invalid range");
            }

            if (last > end()) {
                throw std::out_of_range("erase () last past the end");
            }

            size_type offset = first - begin();
            size_type offset_last = last - begin();

            iterator place = begin() + offset;
            iterator last_place = begin() + offset_last;

            last_place = std::move(last_place, end(), place);
            destroy_backward(last_place, end());

            _impl._end_ptr = last_place;

            auto npow2 = next_pow_2(size() - 1);

            if (npow2 < capacity()) {
                shrink(npow2);
            }

            return begin() + offset;
        }

        inline void resize(size_type size) {
            resize(size, type_t());
        }

        inline void resize(size_type size, const_reference value) {
            if (size > size()) {
                insert(end(), size, value);
            } else {
                shrink(size);
            }
        }

        inline void swap(small_vector_base &other) {
            if (this == &other) {
                return;
            }

            // if both are "large"
            if (!is_local() && !other.is_local()) {
                // both "large" just swap all buffers
                _impl.swap(other._impl);
            } else {
                if (size() > other.size()) {
                    swap_vectors(*this, other);
                } else {
                    swap_vectors(other, *this);
                }
            }
        }

        [[nodiscard]]
        inline bool is_local() const noexcept {
            return _impl._begin_ptr == _small.begin();
        }

        ~small_vector_base() {
            destroy_backward(begin(), end());
            _impl.deallocate();
        }

    protected:

        struct vector_impl_t : public alloc_rebind_t {
        public:

            vector_impl_t(
                    pointer _begin_p,
                    pointer _end_p,
                    pointer _cap_p,
                    pointer _init_cap
            ) :
                    alloc_rebind_t(),
                    _begin_ptr{_begin_p},
                    _end_ptr{_end_p},
                    _capacity_ptr{_cap_p},
                    _initial_capacity{_init_cap} {}

            explicit vector_impl_t(
                    alloc_rebind_t const &alloc,
                    pointer _begin_p,
                    pointer _end_p,
                    pointer _cap_p,
                    pointer _init_cap
            ) noexcept:
                    alloc_rebind_t(alloc),
                    _begin_ptr{_begin_p},
                    _end_ptr{_end_p},
                    _capacity_ptr{_cap_p},
                    _initial_capacity{_init_cap} {}

            explicit vector_impl_t(
                    alloc_rebind_t &&alloc,
                    pointer _begin_p,
                    pointer _end_p,
                    pointer _cap_p,
                    pointer _init_cap
            ) noexcept:
                    alloc_rebind_t(std::move(alloc)),
                    _begin_ptr{_begin_p},
                    _end_ptr{_end_p},
                    _capacity_ptr{_cap_p},
                    _initial_capacity{_init_cap} {}

            inline void swap(vector_impl_t &v) {
                std::swap(_begin_ptr, v._begin_ptr);
                std::swap(_end_ptr, v._end_ptr);
                std::swap(_capacity_ptr, v._capacity_ptr);
            }

            inline void replace(pointer begin_ptr, pointer end_ptr, pointer capacity_ptr) {
                _begin_ptr = begin_ptr;
                _end_ptr = end_ptr;
                _capacity_ptr = capacity_ptr;
            }

            inline void deallocate() {
                // must not be local
                if (_capacity_ptr != _initial_capacity) {
                    alloc_rebind_t::deallocate(
                            _begin_ptr,
                            _capacity_ptr - _begin_ptr);
                }
            }

            pointer
                    _begin_ptr{},
                    _end_ptr{},
                    _capacity_ptr{};

            pointer const _initial_capacity{};
        };

        inline explicit small_vector_base(alloc_t const &alloc, size_type n) noexcept:
                _small{},
                _impl{
                        alloc,
                        _small.begin(),
                        _small.begin(),
                        _small.begin() + n,
                        _small.begin() + n
                } {}

        inline small_vector_base(size_type n, const_reference value, alloc_t const &alloc, size_type self_size) noexcept
                :
                small_vector_base(alloc, self_size) {
            assign(n, value);
        }

        small_vector_base(const small_vector_base &value, alloc_t const &alloc, size_type self_size) :
                small_vector_base(alloc, self_size) {
            operator=(value);
        }

        small_vector_base(small_vector_base &&value, alloc_t const &alloc, size_type self_size) noexcept:
                small_vector_base(alloc, self_size) {
            swap(value);
        }

        small_vector_base(std::initializer_list<value_type> init_list, alloc_t const &alloc, size_type self_size) :
                small_vector_base(alloc, self_size) {
            assign(init_list);
        }

        template<class input_it_t>
        small_vector_base(
                input_it_t first,
                input_it_t last,
                alloc_t const &alloc,
                size_type self_size
        ) :
                small_vector_base(alloc, self_size) {
            assign(first, last);
        }

        inline void grow(size_type n) {

            if (n <= capacity()) {
                return;
            }

            auto new_begin = _impl.allocate(n);
            auto data_size = size();

            if constexpr (std::is_trivially_copyable_v<type_t>) {
                std::copy(begin(), end(), new_begin);
                destroy_backward(begin(), end());
            } else {
                std::uninitialized_move (begin(), end(), new_begin);
            }

            _impl.deallocate();

            _impl.replace(
                    new_begin,
                    new_begin + data_size,
                    new_begin + n
            );
        }

        inline void grow() {
            grow(get_next_capacity(capacity()));
        }

        inline void grow_near_pow_2(size_type size) {
            if (size == 0) {
                size = 1;
            }

            grow(get_next_capacity(size - 1));
        }

        inline size_type get_next_capacity(size_type size) {
            size_type next_cap = next_pow_2(size + 1);

            if (next_cap == 0) {
                next_cap = this->max_size();
            }

            return next_cap;
        }

        inline void shrink(size_type n) {

            if (is_local() || n >= capacity()) {
                return;
            }

            auto data_size = std::min(size(), n);
            auto cut_point = begin() + data_size;

            pointer new_begin;

            // if required capacity smaller than local storage, restore
            if (n < static_cast < size_type > (_impl._initial_capacity - _small.begin())) {
                new_begin = _small.begin();
                n = _impl._initial_capacity - _small.begin();
            } else {
                new_begin = _impl.allocate(n);
            }

            if constexpr (std::is_trivially_copyable<type_t>::value) {
                std::copy(begin(), cut_point, new_begin);
                destroy_backward(begin(), end());
            } else {
                std::move(begin(), cut_point, new_begin);
                destroy_backward(cut_point, end());
            }

            _impl.deallocate();

            _impl.replace(
                    new_begin, // begin
                    new_begin + data_size, // end
                    new_begin + n
            );
        }

        inline void destroy_backward(pointer b, pointer e) {
            if constexpr (!std::is_trivially_destructible_v<type_t>) {
                while (b < e) {
                    --e;
                    alloc_traits_t::destroy(_impl, e);
                }
            }
        }

        inline static void swap_vectors(small_vector_base &large_v, small_vector_base &small_v) {
            size_type
                    large_size = large_v.size(),
                    small_size = small_v.size();

            if (small_v.capacity() < large_size) {
                small_v.grow_near_pow_2(large_size);
            }

            // move stuff around
            small_v._impl._end_ptr = (small_v.begin() + large_size);
            large_v._impl._end_ptr = (large_v.begin() + small_size);

            // move common range
            std::swap_ranges(
                    small_v.begin(),
                    small_v.begin() + small_size,
                    large_v.begin()
            );

            // move uninitialized range
            std::uninitialized_move(
                    large_v.begin() + small_size,
                    large_v.begin() + large_size,
                    small_v.begin() + small_size);

            // shrink new small
            large_v.shrink(small_size);
        }

        vector_impl_t _impl;

        details::static_storage<type_t, 1> _small;
        // reserved: do not define any variables after _first
    };

    template<typename type_t, std::size_t CAPACITY, typename alloc_t = std::allocator<type_t> >
    struct small_vector : public small_vector_base<type_t, alloc_t> {
    private:
        using base_vector_t = small_vector_base<type_t, alloc_t>;
    public:

        using value_type = typename base_vector_t::value_type;
        using const_reference = typename base_vector_t::const_reference;
        using size_type = typename base_vector_t::size_type;

        inline small_vector() noexcept(noexcept(alloc_t())):
                base_vector_t(alloc_t(), CAPACITY) {}

        inline explicit small_vector(alloc_t const &alloc) noexcept:
                base_vector_t(alloc, CAPACITY) {}

        inline small_vector(size_type n, const_reference value, alloc_t const &alloc = alloc_t()) noexcept:
                base_vector_t(n, value, alloc, CAPACITY) {}

        inline small_vector(const small_vector &other, alloc_t const &alloc = alloc_t()) :
                base_vector_t(other, alloc, CAPACITY) {}

        inline small_vector(small_vector && other, alloc_t const &alloc = alloc_t()) noexcept:
                base_vector_t(std::move(other), alloc, CAPACITY) {}

        inline explicit small_vector(small_vector_base<type_t> &&other, alloc_t const &alloc = alloc_t()) noexcept:
                base_vector_t(std::move(other), alloc, CAPACITY) {}

        inline small_vector(std::initializer_list<value_type> init_list, alloc_t const &alloc = alloc_t()) :
                base_vector_t(init_list, alloc, CAPACITY) {}

        template<class input_it_t>
        inline small_vector(
                input_it_t first,
                input_it_t last,
                alloc_t const &alloc = alloc_t(),
                typename std::enable_if_t<details::is_iterator<input_it_t>::value> * = nullptr
        ) :
                base_vector_t(first, last, alloc, CAPACITY) {}

        inline small_vector &operator=(const small_vector_base<type_t, alloc_t> &v) {
            base_vector_t::operator=(v);
            return *this;
        }

        inline small_vector &operator=(const small_vector &other) {
            base_vector_t::operator=(other);
            return *this;
        }

        inline small_vector &operator=(small_vector_base<type_t, alloc_t> &&other) {
            base_vector_t::operator=(std::move(other));
            return *this;
        }

        inline small_vector &operator=(small_vector &&other) noexcept {
            base_vector_t::operator=(std::move(other));
            return *this;
        }

        inline small_vector &operator=(std::initializer_list<type_t> init_list) {
            base_vector_t::operator=(init_list);
            return *this;
        }

    private:
        details::static_storage<type_t, CAPACITY - 1> _small_data;
    };

}

#endif
