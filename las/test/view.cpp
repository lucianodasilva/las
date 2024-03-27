#include <catch2/catch_all.hpp>
#include <las/view.hpp>

#include <vector>

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

		GIVEN("a dynamically allocated buffer") {
			std::size_t const SOURCE_SIZE = 16;
			auto buffer = make_test_buffer (SOURCE_SIZE);

			WHEN("constructing with ptr/size pair") {
				auto victim = view (buffer.get(), SOURCE_SIZE);

				THEN("view must be properly constructed") {
					REQUIRE(victim.data() == buffer.get());
					REQUIRE(victim.size() == SOURCE_SIZE);
				}
			}

			WHEN("constructing with const ptr/size pair") {
				auto const* const_ptr = buffer.get();
				auto victim = view (const_ptr, SOURCE_SIZE);

				THEN("view must be properly constructed") {
					REQUIRE(victim.data() == buffer.get());
					REQUIRE(victim.size() == SOURCE_SIZE);
				}
			}

			WHEN("constructing with begin/end pair") {
				auto victim = view (buffer.get(), buffer.get() + SOURCE_SIZE);

				THEN("view must be properly constructed") {
					REQUIRE(victim.data() == buffer.get());
					REQUIRE(victim.size() == SOURCE_SIZE);
				}
			}

			WHEN("constructing with const begin/end pair") {
				auto const* const_ptr = buffer.get();
				view < int const > victim(const_ptr, const_ptr + SOURCE_SIZE);

				THEN("view must be properly constructed") {
					REQUIRE(victim.data() == buffer.get());
					REQUIRE(victim.size() == SOURCE_SIZE);
				}
			}
		}

		GIVEN("a 'c' array") {
			std::size_t const SOURCE_SIZE = 16;

			WHEN("constructing with 'c' array") {
				int buffer[SOURCE_SIZE];
				auto victim = view(buffer);

				THEN("view must be properly constructed") {
					REQUIRE(victim.data() == buffer);
					REQUIRE(victim.size() == SOURCE_SIZE);
				}
			}

			WHEN("constructing with const 'c' array") {
				int const buffer[SOURCE_SIZE]{ 0 };
				auto victim = view(buffer);

				THEN("view must be properly constructed") {
					REQUIRE(victim.data() == buffer);
					REQUIRE(victim.size() == SOURCE_SIZE);
				}
			}
		}

		GIVEN("a vector") {
			std::size_t const SOURCE_SIZE = 16;

			WHEN("constructing with a vector") {
				std::vector < int > source (SOURCE_SIZE);
				auto victim = view(source);

				THEN("view must be properly constructed") {
					REQUIRE(victim.data() == source.data());
					REQUIRE(victim.size() == SOURCE_SIZE);
				}
			}

			WHEN("constructing with a const vector") {
				std::vector < int > const source (SOURCE_SIZE);
				auto victim = view (source);

				THEN("view must be properly constructed") {
					REQUIRE(victim.data() == source.data());
					REQUIRE(victim.size() == SOURCE_SIZE);
				}
			}
		}

        GIVEN("a std::array") {
            std::size_t const SOURCE_SIZE = 16;

            WHEN("constructing with std::array") {
                std::array < int, SOURCE_SIZE > buffer;
                auto victim = view(buffer);

                THEN("view must be properly constructed") {
                    REQUIRE(victim.data() == buffer.data());
                    REQUIRE(victim.size() == SOURCE_SIZE);
                }
            }

            WHEN("constructing with const std::array") {
                std::array < int, SOURCE_SIZE > const buffer {0};
                auto victim = view(buffer);

                THEN("view must be properly constructed") {
                    REQUIRE(victim.data() == buffer.data());
                    REQUIRE(victim.size() == SOURCE_SIZE);
                }
            }
        }

        GIVEN("another view") {
            std::size_t const SOURCE_SIZE = 16;
            std::array < int, SOURCE_SIZE > buffer;

            WHEN("constructing with view") {
                auto src_view = view(buffer);
                auto victim = view(src_view);

                THEN("view must be properly constructed") {
                    REQUIRE(victim.data() == buffer.data());
                    REQUIRE(victim.size() == SOURCE_SIZE);
                }
            }

            WHEN("constructing with const view") {
                auto const SRC_VIEW = view (buffer);
                auto victim = view(SRC_VIEW);

                THEN("view must be properly constructed") {
                    REQUIRE(victim.data() == buffer.data());
                    REQUIRE(victim.size() == SOURCE_SIZE);
                }
            }

            WHEN("constructing a const view with a non const view") {
                auto src_view = view(buffer);
                view < int > const VICTIM (src_view);

                THEN("view must be properly constructed") {
                    REQUIRE(VICTIM.data() == buffer.data());
                    REQUIRE(VICTIM.size() == SOURCE_SIZE);
                }
            }
        }
	}

	SCENARIO("access state", "[view]") {

		GIVEN("an empty view") {
			auto victim = view < int >();

			THEN("empty should return true") {
				REQUIRE(victim.empty());
			}

			THEN("data should return nullptr") {
				REQUIRE(victim.data() == nullptr);
			}

			THEN("size should return 0(zero)") {
				REQUIRE(victim.size() == 0);
			}
		}

		GIVEN("a view pointing to data") {
			std::size_t const SOURCE_SIZE = 16;

			auto buffer = make_test_buffer(SOURCE_SIZE);
			auto victim = view(buffer.get(), SOURCE_SIZE);

			THEN("empty should return false") {
				REQUIRE_FALSE(victim.empty());
			}

			THEN("data should point to data") {
				REQUIRE(victim.data() == buffer.get());
			}

			THEN("size should return the correct length") {
				REQUIRE(victim.size() == SOURCE_SIZE);
			}
		}
	}

	SCENARIO("iterate", "[view]") {

		GIVEN("a 'filled' view") {
			std::size_t const SOURCE_SIZE = 16;

			auto buffer = make_test_buffer(SOURCE_SIZE);
			auto victim = view(buffer.get(), SOURCE_SIZE);

			THEN("iterating elements should match the source") {
				REQUIRE(match(victim, buffer.get(), SOURCE_SIZE));
			}
		}

		GIVEN("a const 'filled' view") {
			std::size_t const SOURCE_SIZE = 16;

			auto buffer = make_test_buffer(SOURCE_SIZE);
			auto const victim = view(buffer.get(), SOURCE_SIZE);

			THEN("iterating elements should match the source") {
				REQUIRE(match(victim, buffer.get(), SOURCE_SIZE));
			}
		}
	}

	SCENARIO("random access", "[view]") {

		GIVEN("a 'filled' view") {
			std::size_t const SOURCE_SIZE = 16;

			auto buffer = make_test_buffer(SOURCE_SIZE);
			auto victim = view(buffer.get(), SOURCE_SIZE);

			THEN("accessing elements by subscript operator should match the source") {
				for (int i = 0; i < SOURCE_SIZE; ++i) {
					REQUIRE(victim[i] == buffer[i]);
				}
			}

			THEN("accessing elements by calling 'at' should match the source") {
				for (int i = 0; i < SOURCE_SIZE; ++i) {
					REQUIRE(victim.at (i) == buffer[i]);
				}
			}

			THEN("accessing elements out of range should raise an exception") {
				REQUIRE_THROWS_AS(victim.at(SOURCE_SIZE + 1), std::out_of_range);
			}
		}

		GIVEN("a const 'filled' view") {
			std::size_t const SOURCE_SIZE = 16;

			auto buffer = make_test_buffer(SOURCE_SIZE);
			auto const victim = view(buffer.get(), SOURCE_SIZE);

			THEN("accessing elements by subscript operator should match the source") {
				for (int i = 0; i < SOURCE_SIZE; ++i) {
					REQUIRE(victim[i] == buffer[i]);
				}
			}

			THEN("accessing elements by calling 'at' should match the source") {
				for (int i = 0; i < SOURCE_SIZE; ++i) {
					REQUIRE(victim.at(i) == buffer[i]);
				}
			}

			THEN("accessing elements out of range should raise an exception") {
				REQUIRE_THROWS_AS(victim.at(SOURCE_SIZE + 1), std::out_of_range);
			}
		}
	}

}