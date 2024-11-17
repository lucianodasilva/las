#pragma once
#ifndef LAS_FIXED_RING_BUFFER_HPP
#define LAS_FIXED_RING_BUFFER_HPP

#include <array>
#include <memory>
#include <utility>

#include "bits.hpp"
#include "static_storage.hpp"

namespace las {

	template < typename value_t, std::size_t capacity_v >
	struct static_ring_buffer {
		using value_type = std::decay_t < value_t >;
		using reference = value_type &;
		using const_reference = value_type const &;
		using pointer = value_type *;
		using const_pointer = const value_type *;

		using size_type = std::size_t;

		struct iterator {

			using iterator_category = std::forward_iterator_tag;
			using value_type = static_ring_buffer::value_type;
			using difference_type = std::ptrdiff_t;
			using pointer = static_ring_buffer::pointer;
			using reference = static_ring_buffer::reference;

			iterator(static_ring_buffer &inst, std::size_t index) :
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
			static_ring_buffer & _inst;
			mutable std::size_t _index;
		};

		using const_iterator = iterator const;


		~static_ring_buffer() {
			clear();
		}

		constexpr static_ring_buffer() = default;

		constexpr static_ring_buffer(static_ring_buffer const &origin) {
			copy_from(origin);
		}

		constexpr static_ring_buffer(static_ring_buffer &&origin) noexcept {
			this->swap(origin);
		}

		constexpr static_ring_buffer &operator=(static_ring_buffer const &origin) {
			if (&origin != this) {
				copy_from(origin);
			}

			return *this;
		}

		constexpr static_ring_buffer &operator=(static_ring_buffer &&origin) noexcept {
			this->swap(origin);
			return *this;
		}

		constexpr void swap(static_ring_buffer & other)  noexcept {
			std::swap(_index_begin, other._index_begin);
			std::swap(_index_end, other._index_end);
		}

		constexpr void clear() {
			range_destroy ();

			_index_begin = 0;
			_index_end = 0;
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return _index_begin == _index_end;
		}

		[[nodiscard]] constexpr bool full() const noexcept {
			return size () == capacity;
		}

		[[nodiscard]] constexpr size_type size() const noexcept {
			return _index_end - _index_begin;
		}

		[[nodiscard]] constexpr iterator begin() {
			return iterator(*this, 0);
		}

		[[nodiscard]] constexpr iterator end() {
			return iterator(*this, size());
		}

		[[nodiscard]] constexpr reference front() noexcept {
			return *ptr_to(0);
		}

		[[nodiscard]] constexpr const_reference front() const noexcept {
			return *ptr_to(0);
		}

		[[nodiscard]] constexpr reference back() noexcept {
			return *ptr_to(size() - 1);
		}

		[[nodiscard]] constexpr const_reference back() const noexcept {
			return *ptr_to(size() - 1);
		}

		constexpr void push_back(const_reference value) {
			emplace_back(value);
		}

		constexpr void push_back(value_type &&value) {
			emplace_back(std::forward<value_type>(value));
		}

		template<typename ... args_t>
		constexpr void emplace_back(args_t &&... args) {
			if (full()) {
				throw std::out_of_range ("Fixed ring buffer capacity exceeded");
			}

			new (ptr_to(size())) value_type (std::forward < args_t >(args)...);

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
			if (full()) {
				throw std::out_of_range ("Fixed ring buffer capacity exceeded");
			}

			if (_index_begin == 0) {
				// to simulate folding backwards, shift the transposition frame forward
				auto cap = capacity;

				_index_begin += cap;
				_index_end += cap;
			}

			--_index_begin;

			// construct
			new (ptr_to(0)) value_type (std::forward<args_t>(args)...);
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
				std::destroy_at (&front());
			}

			// change index
			++_index_begin;
		}

		void pop_back() {
			if (empty()) { return; }

			// if !pointer inplace destroy
			if (!std::is_pointer<value_t>()) {
				std::destroy_at (&back());
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

		static constexpr auto capacity { next_pow_2 (capacity_v) };
	private:

		[[nodiscard]] std::ptrdiff_t transpose_index(std::ptrdiff_t index, std::ptrdiff_t offset = 0) const noexcept {
			return ((index + offset) & _index_mask);
		}

		pointer ptr_to(std::ptrdiff_t index) {
			return _data.begin () + (transpose_index(_index_begin, index));
		}

		const_pointer ptr_to(std::ptrdiff_t index) const {
			return _data.begin () + (transpose_index(_index_begin, index));
		}

		void copy_from(static_ring_buffer const &origin) {
			// destroy current data
			range_destroy();

			_index_begin = 0;
			_index_end = 0;

			origin.range_copy(ptr_to (0));

			// reset buffer ring
			_index_begin = 0;
			_index_end = origin.size();
		}

		void range_destroy() {
			range_apply(
					[](pointer begin_ptr, pointer end_ptr, std::ptrdiff_t) {
						std::destroy (begin_ptr, end_ptr);
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
						std::uninitialized_move(begin_ptr, end_ptr, dest + offset);
					},
					this);
		}

		template<typename func_t, typename inst_t >
		static void range_apply(func_t func, inst_t * inst) {
			if (inst->empty()) {
				return;
			}

			auto *begin_ptr = inst->ptr_to (0);
			auto *end_ptr = inst->ptr_to(inst->size());

			if (end_ptr > begin_ptr) {
				// linear call
				func(begin_ptr, end_ptr, 0);
			} else {
				// split call
				func(begin_ptr, inst->_data.end(), 0);
				func(inst->_data.begin(), end_ptr, inst->_data.end() - end_ptr);
			}
		}

		static_storage < value_t, capacity > _data;

		size_type _index_begin {0};
		size_type _index_end {0};
		size_type _index_mask { capacity - 1 };
	};

}

#endif
