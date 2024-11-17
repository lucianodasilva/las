#pragma once
#ifndef LAS_STATIC_STORAGE_HPP
#define LAS_STATIC_STORAGE_HPP

namespace las {

	template<class type_t, std::size_t capacity_v>
	struct static_storage {

		type_t &operator[](std::size_t index) {
			return *reinterpret_cast <type_t *> (data + (index * sizeof(type_t)));
		}

		type_t const &operator[](std::size_t index) const {
			return *reinterpret_cast <type_t const *> (data + (index * sizeof(type_t)));
		}

		const type_t *begin() const noexcept {
			return reinterpret_cast <const type_t *> (+data);
		};

		type_t *begin() noexcept {
			return reinterpret_cast <type_t *> (+data);
		};

		type_t *end() noexcept {
			return begin() + capacity_v;
		}

		const type_t *end() const noexcept {
			return begin() + capacity_v;
		}

		alignas(next_pow_2(sizeof(type_t))) uint8_t data[sizeof(type_t) * capacity_v];

		static constexpr auto capacity { capacity_v };
	};
}

#endif
