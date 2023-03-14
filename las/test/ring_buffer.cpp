#include <catch2/catch_all.hpp>

#include <las/ring_buffer.hpp>
#include <las/test/token.hpp>

namespace las::test {

    using test_item = token<int>;

    inline void init_items(
            ring_buffer<test_item> &victim,
            size_t item_count,
            std::shared_ptr<token_counters> counters
    ) {
        for (int i = 0; i < item_count; ++i) {
            victim.emplace_back(counters, i);
        }
    }

    inline void add_item_sequence(ring_buffer<uint8_t> &victim, uint8_t item_count, uint8_t offset) {
        for (uint8_t i = 0; i < item_count; ++i) {
            victim.push_back(i + offset);
        }
    }

    template<std::size_t SIZE>
    inline bool match(ring_buffer<uint8_t> &v1, uint8_t const (&v2)[SIZE]) {
        return std::equal(
                std::begin(v2),
                std::end(v2),
                v1.begin()
        );
    }

    SCENARIO("ring_buffer accessors", "[ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        auto make_item = [&](int v) {
            return test_item(counters, v);
        };

        GIVEN("empty ring_buffer") {
            auto test1 = make_item(1);
            auto test2 = make_item(2);
            auto test3 = make_item(3);

            ring_buffer<test_item> victim;

            victim.reserve(3);
            counters->reset();

            victim.push_back(test1);
            victim.push_back(test2);
            victim.push_back(test3);

            THEN("check move and copy operations") {
                REQUIRE(counters->check_copies(3));
                REQUIRE(counters->check_moves(0));
            }

            THEN("check front inserted item") {
                REQUIRE(victim.front() == test1);
            }

            THEN("check back inserted item") {
                REQUIRE(victim.back() == test3);
            }
        }

        GIVEN("folded ring_buffer") {
            size_t const INITIAL_CAPACITY = 16;
            size_t const HALF_CAPACITY = INITIAL_CAPACITY / 2;

            auto item = make_item(999);

            ring_buffer<test_item> victim;

            victim.reserve(INITIAL_CAPACITY);
            counters->reset();

            // initialize with item in the middle
            init_items(victim, HALF_CAPACITY, counters);
            victim.push_back(item);

            init_items(victim, HALF_CAPACITY - 1, counters);

            // remove the first half
            for (int i = 0; i < HALF_CAPACITY; ++i) {
                victim.pop_front();
            }

            // fill remaining space with items
            init_items(victim, HALF_CAPACITY - 1, counters);
            victim.push_back(item);

            THEN("check move and copy operations") {
                REQUIRE(counters->check_copies(2));
                REQUIRE(counters->check_moves(0));
            }

            THEN("check front inserted item") {
                REQUIRE(victim.front() == item);
            }

            THEN("check back inserted item") {
                REQUIRE(victim.back() == item);
            }
        }
    }

    SCENARIO("ring_buffer push_back", "[ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        auto make_item = [&](int v) {
            return test_item(counters, v);
        };

        GIVEN("filled ring_buffer") {
            size_t const INITIAL_CAPACITY = 10;
            size_t const HALF_CAPACITY = INITIAL_CAPACITY / 2;

            auto item = make_item(999);
            ring_buffer<test_item> victim;

            // initialize with item in the middle
            init_items(victim, HALF_CAPACITY, counters);

            THEN("add element by copy and evaluate") {
                counters->reset();
                victim.push_back(item);

                REQUIRE(counters->check_copies(1));
                REQUIRE(counters->check_moves(0));
                REQUIRE(victim.back() == item);
            }

            THEN("add element by move and evaluate") {
                auto movable_object = item;

                counters->reset();
                victim.push_back(std::move(movable_object));

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(1));
                REQUIRE(victim.back() == item);
            }
        }

        GIVEN("folded ring_buffer") {
            size_t const INITIAL_CAPACITY = 10;
            size_t const HALF_CAPACITY = INITIAL_CAPACITY / 2;

            auto item = make_item(999);
            ring_buffer<test_item> victim;

            // initialize with item in the middle
            init_items(victim, INITIAL_CAPACITY, counters);

            // remove half capacity
            for (size_t i = 0; i < HALF_CAPACITY; ++i) {
                victim.pop_front();
            }

            THEN("add element by copy and evaluate") {
                counters->reset();
                victim.push_back(item);

                REQUIRE(counters->check_copies(1));
                REQUIRE(counters->check_moves(0));
                REQUIRE(victim.back() == item);
            }

            THEN("add element by move and evaluate") {
                auto movable_object = item;

                counters->reset();
                victim.push_back(std::move(movable_object));

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(1));
                REQUIRE(victim.back() == item);
            }
        }
    }

    SCENARIO("ring_buffer emplace_back", "[ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        GIVEN("filled ring_buffer") {
            size_t const INITIAL_CAPACITY = 10;
            size_t const HALF_CAPACITY = INITIAL_CAPACITY / 2;

            int const ITEM_VALUE = 999;
            ring_buffer<test_item> victim;

            // initialize with item in the middle
            init_items(victim, HALF_CAPACITY, counters);

            THEN("emplace element and evaluate") {
                counters->reset();
                victim.emplace_back(counters, ITEM_VALUE);

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(0));
                REQUIRE(victim.back().value() == ITEM_VALUE);
            }
        }

        GIVEN("folded ring_buffer") {
            size_t const INITIAL_CAPACITY = 10;
            size_t const HALF_CAPACITY = INITIAL_CAPACITY / 2;

            int const ITEM_VALUE = 999;
            ring_buffer<test_item> victim;

            // initialize with item in the middle
            init_items(victim, INITIAL_CAPACITY, counters);

            // remove half capacity
            for (size_t i = 0; i < HALF_CAPACITY; ++i) {
                victim.pop_front();
            }

            THEN("emplace element and evaluate") {
                counters->reset();
                victim.emplace_back(counters, ITEM_VALUE);
                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(0));
                REQUIRE(victim.back().value() == ITEM_VALUE);
            }
        }
    }

    SCENARIO("ring_buffer copy", "[ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        GIVEN("filled ring_buffer") {
            size_t const INITIAL_CAPACITY = 16;
            size_t const HALF_CAPACITY = INITIAL_CAPACITY / 2;

            ring_buffer<test_item> victim;
            victim.reserve(INITIAL_CAPACITY);

            // initialize with item in the middle
            init_items(victim, HALF_CAPACITY, counters);

            THEN("copy elements and evaluate") {
                auto copy = victim;
                for (int i = 0; i < HALF_CAPACITY; ++i) {
                    REQUIRE(copy.front() == victim.front());
                    copy.pop_front();
                    victim.pop_front();
                }
            }
        }

        GIVEN("folded ring_buffer") {
            size_t const INITIAL_CAPACITY = 16;
            size_t const HALF_CAPACITY = INITIAL_CAPACITY / 2;
            ring_buffer<test_item> victim;

            // initialize with item in the middle
            init_items(victim, INITIAL_CAPACITY, counters);
            // remove half capacity
            for (size_t i = 0; i < HALF_CAPACITY; ++i) {
                victim.pop_front();
            }

            THEN("copy elements and evaluate") {
                auto copy = victim;
                for (int i = 0; i < HALF_CAPACITY; ++i) {
                    REQUIRE(copy.front() == victim.front());
                    copy.pop_front();
                    victim.pop_front();
                }
            }
        }
    }

    TEST_CASE("DRIVERS-774-GROW-CORRUPTION", "[ring_buffer]") {
        // create victim
        ring_buffer<uint8_t> victim;
        size_t const INITIAL_SEQUENCE = 16;
        size_t const POP_SEQUENCE = 4;
        size_t const SECOND_SEQUENCE = 8;
        size_t const THIRD_SEQUENCE = 8;

        uint8_t const EXPECTED_FINAL_SEQUENCE[] = {
                4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        };

        // recreate issue state
        add_item_sequence(victim, INITIAL_SEQUENCE, 0);
        victim.pop_front_n(POP_SEQUENCE);
        add_item_sequence(victim, SECOND_SEQUENCE, INITIAL_SEQUENCE);
        add_item_sequence(victim, THIRD_SEQUENCE, INITIAL_SEQUENCE + SECOND_SEQUENCE);
        // evaluate expected sequence
        REQUIRE (match(victim, EXPECTED_FINAL_SEQUENCE));
    }
}