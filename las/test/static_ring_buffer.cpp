#include <catch2/catch_all.hpp>

#include <las/static_ring_buffer.hpp>
#include <las/test/token.hpp>

namespace las::test {

    using test_item = token<int>;

    template < std::size_t capacity_v >
    void init_items(
            static_ring_buffer<test_item, capacity_v> &victim,
            size_t item_count,
            std::shared_ptr<token_counters> counters
    ) {
        for (int i = 0; i < item_count; ++i) {
            victim.emplace_back(counters, i);
        }
    }

    template < std::size_t capacity_v >
    void add_item_sequence(static_ring_buffer<uint8_t, capacity_v> &victim, uint8_t item_count, uint8_t offset) {
        for (uint8_t i = 0; i < item_count; ++i) {
            victim.push_back(i + offset);
        }
    }

    template<std::size_t capacity_v>
    bool match(static_ring_buffer<uint8_t, capacity_v> &v1, uint8_t const (&v2)[capacity_v]) {
        return std::equal(
                std::begin(v2),
                std::end(v2),
                v1.begin()
        );
    }

    // Forces the buffer to fold on itself by adding and removing items until its no longer sequential
    template<std::size_t capacity_v>
    static_ring_buffer <test_item, capacity_v > make_folded (std::shared_ptr<token_counters> counters) {
        static_ring_buffer<test_item, capacity_v> victim;

        // initialize with item in the middle
        init_items(victim, capacity_v, counters);

        // remove half capacity
        for (size_t i = 0; i < next_pow_2 (capacity_v) / 2; ++i) {
            victim.pop_front();
        }

        return victim;
    }

    SCENARIO("static_ring_buffer access", "[static_ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        auto make_item = [&](int v) {
            return test_item(counters, v);
        };

        GIVEN("an empty static_ring_buffer") {
            constexpr uint32_t capacity = 8;
            auto               test1    = make_item(1);
            auto               test2    = make_item(2);
            auto               test3    = make_item(3);

            static_ring_buffer<test_item, capacity> victim;

            counters->reset();

            victim.push_back(test1);
            victim.push_back(test2);
            victim.push_back(test3);

            THEN("front element should be the first pushed element") {
                REQUIRE(victim.front() == test1);
            }

            THEN("back element should be the last pushed element") {
                REQUIRE(victim.back() == test3);
            }
        }

        GIVEN("a folded static_ring_buffer") {
            constexpr size_t capacity = 16;
            constexpr size_t half_capacity    = capacity / 2;

            auto               test1    = make_item(1);
            auto               test2    = make_item(2);

            static_ring_buffer<test_item, capacity> victim;

            counters->reset();

            // initialize with item in the middle
            init_items(victim, half_capacity, counters);
            victim.push_back(test1);

            init_items(victim, half_capacity - 1, counters);

            // remove the first half
            for (int i = 0; i < half_capacity; ++i) {
                victim.pop_front();
            }

            // fill remaining space with items
            init_items(victim, half_capacity - 1, counters);
            victim.push_back(test2);

            THEN("front element should be the first pushed element") {
                REQUIRE(victim.front() == test1);
            }

            THEN("back element should be the last pushed element") {
                REQUIRE(victim.back() == test2);
            }
        }
    }

    SCENARIO("static_ring_buffer push_back", "[static_ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        auto make_item = [&](int v) {
            return test_item(counters, v);
        };

        GIVEN("a containing static_ring_buffer") {
            constexpr size_t capacity = 16;
            constexpr size_t half_capacity    = capacity / 2;

            auto item = make_item(999);
            static_ring_buffer<test_item, capacity> victim;

            // initialize with item in the middle
            init_items(victim, half_capacity, counters);

            THEN("pushing adds element to the end by copy") {
                counters->reset();
                victim.push_back(item);

                REQUIRE(counters->check_copies(1));
                REQUIRE(counters->check_moves(0));
                REQUIRE(victim.back() == item);
            }

            THEN("adds element to the end by move") {
                auto movable_object = item;

                counters->reset();
                victim.push_back(std::move(movable_object));

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(1));
                REQUIRE(victim.back() == item);
            }
        }

        GIVEN("a folded static_ring_buffer") {
            constexpr size_t capacity = 16;

            auto item = make_item(999);
            auto victim = make_folded < capacity > (counters);

            THEN("adds element to the end by copy") {
                counters->reset();
                victim.push_back(item);

                REQUIRE(counters->check_copies(1));
                REQUIRE(counters->check_moves(0));
                REQUIRE(victim.back() == item);
            }

            THEN("adds element to the end by move") {
                auto movable_object = item;

                counters->reset();
                victim.push_back(std::move(movable_object));

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(1));
                REQUIRE(victim.back() == item);
            }
        }

        GIVEN("a full static_ring_buffer") {
            auto victim = static_ring_buffer < test_item, 2 > {};
            init_items (victim, 2, counters);

            THEN("exceeding capacity throws exception") {
                REQUIRE_THROWS_AS(victim.push_back(make_item(999)), std::out_of_range);
            }
        }
    }

    SCENARIO("static_ring_buffer push_front", "[static_ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        auto make_item = [&](int v) {
            return test_item(counters, v);
        };

        GIVEN("a containing static_ring_buffer") {
            constexpr size_t capacity = 16;
            constexpr size_t half_capacity    = capacity / 2;

            auto item = make_item(999);
            static_ring_buffer<test_item, capacity> victim;

            // initialize with item in the middle
            init_items(victim, half_capacity, counters);

            THEN("pushing adds element to the beginning by copy") {
                counters->reset();
                victim.push_front(item);

                REQUIRE(counters->check_copies(1));
                REQUIRE(counters->check_moves(0));
                REQUIRE(victim.front() == item);
            }

            THEN("adds element to the beginning by move") {
                auto movable_object = item;

                counters->reset();
                victim.push_front(std::move(movable_object));

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(1));
                REQUIRE(victim.front() == item);
            }
        }

        GIVEN("a folded static_ring_buffer") {
            constexpr size_t capacity = 16;

            auto item = make_item(999);
            auto victim = make_folded < capacity > (counters);

            THEN("adds element to the end by copy") {
                counters->reset();
                victim.push_front (item);

                REQUIRE(counters->check_copies(1));
                REQUIRE(counters->check_moves(0));
                REQUIRE(victim.front() == item);
            }

            THEN("adds element to the end by move") {
                auto movable_object = item;

                counters->reset();
                victim.push_front(std::move(movable_object));

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(1));
                REQUIRE(victim.front() == item);
            }
        }

        GIVEN("a full static_ring_buffer") {
            auto victim = static_ring_buffer < test_item, 2 > {};
            init_items (victim, 2, counters);

            THEN("exceeding capacity throws exception") {
                REQUIRE_THROWS_AS(victim.push_front(make_item(999)), std::out_of_range);
            }
        }
    }

    SCENARIO("static_ring_buffer emplace_back", "[static_ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        GIVEN("a filled static_ring_buffer") {
            constexpr size_t capacity      = 10;
            constexpr auto half_capacity = capacity / 2;

            static_ring_buffer<test_item, capacity> victim;

            // initialize with item in the middle
            init_items(victim, half_capacity, counters);

            THEN("constructs an element at the end") {
                constexpr auto item_value = 999;
                counters->reset();
                victim.emplace_back(counters, item_value);

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(0));
                REQUIRE(counters->check_ctors(1));

                REQUIRE(victim.back().value() == item_value);
            }
        }

        GIVEN("a folded static_ring_buffer") {
            constexpr size_t capacity = 16;

            auto victim = make_folded < capacity > (counters);

            THEN("constructs an element at the end") {
                constexpr auto item_value = 999;

                counters->reset();
                victim.emplace_back(counters, item_value);
                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(0));
                REQUIRE(counters->check_ctors(1));

                REQUIRE(victim.back().value() == item_value);
            }
        }
    }

    SCENARIO("static_ring_buffer emplace_front", "[static_ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        GIVEN("a filled static_ring_buffer") {
            constexpr size_t capacity      = 10;
            constexpr auto half_capacity = capacity / 2;

            static_ring_buffer<test_item, capacity> victim;

            // initialize with item in the middle
            init_items(victim, half_capacity, counters);

            THEN("constructs an element at the beginning") {
                constexpr auto item_value = 999;
                counters->reset();
                victim.emplace_front(counters, item_value);

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(0));
                REQUIRE(counters->check_ctors(1));

                REQUIRE(victim.front().value() == item_value);
            }
        }

        GIVEN("a folded static_ring_buffer") {
            constexpr size_t capacity = 16;

            auto victim = make_folded < capacity > (counters);

            THEN("constructs an element at the end") {
                constexpr auto item_value = 999;

                counters->reset();
                victim.emplace_front(counters, item_value);
                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(0));
                REQUIRE(counters->check_ctors(1));

                REQUIRE(victim.front().value() == item_value);
            }
        }
    }

    SCENARIO("static_ring_buffer copy", "[static_ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        GIVEN("filled static_ring_buffer") {
            constexpr size_t           capacity      = 16;
            constexpr auto half_capacity = capacity / 2;

            static_ring_buffer<test_item, capacity> victim;

            // initialize with item in the middle
            init_items(victim, half_capacity, counters);

            THEN("all elements should be copied") {
                auto copy = victim;
                for (auto i = 0; i < half_capacity; ++i) {
                    REQUIRE(copy.front() == victim.front());
                    copy.pop_front();
                    victim.pop_front();
                }
            }
        }

        GIVEN("folded static_ring_buffer") {
            constexpr size_t capacity      = 16;
            constexpr auto half_capacity = capacity / 2;
            auto victim = make_folded < capacity > (counters);

            THEN("all elements should be copied") {
                auto copy = victim;
                for (int i = 0; i < half_capacity; ++i) {
                    REQUIRE(copy.front() == victim.front());
                    copy.pop_front();
                    victim.pop_front();
                }
            }
        }
    }

    SCENARIO("static_ring_buffer assignment", "[static_ring_buffer]") {
        auto counters = std::make_shared<token_counters>();

        GIVEN("a filled static_ring_buffer") {
            constexpr size_t capacity      = 16;
            constexpr auto half_capacity = capacity / 2;

            static_ring_buffer<test_item, capacity> source;

            // initialize with item in the middle
            init_items(source, half_capacity, counters);

            AND_GIVEN ("an empty target static_ring_buffer") {
                static_ring_buffer<test_item, capacity> target = {};

                THEN("assignment should copy elements") {
                    counters->reset ();
                    target = source;

                    for (auto i = 0; i < half_capacity; ++i) {
                        REQUIRE(source.front() == target.front());
                        source.pop_front();
                        target.pop_front();
                    }
                }
            }

            AND_GIVEN ("a filled target static_ring_buffer") {
                static_ring_buffer<test_item, capacity> target;

                // initialize with item in the middle
                init_items(target, half_capacity, counters);

                THEN("assignment destroys original elements") {
                    counters->reset ();
                    target = source;
                    REQUIRE (counters->check_dtors (half_capacity));
                    REQUIRE (counters->check_copies (half_capacity));
                }
            }
        }

        GIVEN("a folded static_ring_buffer") {
            constexpr size_t capacity      = 16;
            constexpr size_t half_capacity = capacity / 2;

            static_ring_buffer<test_item, capacity> source;

            // initialize with item in the middle
            init_items(source, capacity, counters);
            // remove half capacity
            for (size_t i = 0; i < half_capacity; ++i) {
                source.pop_front();
            }

            AND_GIVEN ("an empty target static_ring_buffer") {
                static_ring_buffer<test_item, capacity> target = {};

                THEN("assignment should copy elements") {
                    counters->reset ();
                    target = source;

                    for (auto i = 0; i < half_capacity; ++i) {
                        REQUIRE(source.front() == target.front());
                        source.pop_front();
                        target.pop_front();
                    }
                }
            }

            AND_GIVEN ("a filled target static_ring_buffer") {
                static_ring_buffer<test_item, capacity> target;

                // initialize target with item in the middle
                init_items(target, half_capacity, counters);

                THEN("assignment destroys original elements") {
                    counters->reset ();
                    target = source;

                    REQUIRE (counters->check_dtors (half_capacity));
                    REQUIRE (counters->check_copies (half_capacity));
                }
            }
        }
    }
}