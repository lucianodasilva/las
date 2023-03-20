#include <catch2/catch_all.hpp>
#include <las/view.hpp>

namespace las::test {

	std::unique_ptr < int[] > make_test_buffer(std::size_t length) {
		auto buffer = std::make_unique < int[] >(length);

		for (std::size_t i = 0; i < length; ++i) {
			buffer[i] = i;
		}

		return buffer;
	}

	template < typename lh_t, typename rh_t >
	bool match(lh_t const & lhv, rh_t const & rhv) {

		if (std::size(lhv) != std::size(rhv)) {
			return false;
		}

		return std::equal(
			std::begin(lhv),
			std::end(lhv),
			std::begin(rhv));
	}

	template < typename lh_t, typename rh_t >
	bool match(lh_t const& lhv, rh_t * rh_ptr, std::size_t rh_len) {

		if (std::size(lhv) != rh_len) {
			return false;
		}

		return std::equal(
			std::begin(lhv),
			std::end(lhv),
			rh_ptr);
	}

	SCENARIO("constructors", "[view]") {

		GIVEN("a memory buffer") {
			std::size_t const SOURCE_SIZE = 16;
			auto buffer = make_test_buffer (SOURCE_SIZE);

			WHEN("constructing with ptr/size pair") {
				view < int > victim (buffer.get(), SOURCE_SIZE);

				THEN("view must be properly constructed") {
					REQUIRE(match(victim, buffer.get(), SOURCE_SIZE));
				}
			}

			WHEN("constructing with const ptr/size pair") {
				view < int const > victim (static_cast < int const * > (buffer.get()), SOURCE_SIZE);

				THEN("view must be properly constructed") {
					REQUIRE(match(victim, buffer.get(), SOURCE_SIZE));
				}
			}
		}

	}

}