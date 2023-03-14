#include <catch2/catch_all.hpp>

#include <las/small_vector.hpp>
#include <las/test/token.hpp>

#include <vector>

namespace las::test {

    using test_item = token<int>;

    inline void init_items(
            small_vector_base<test_item> &victim,
            size_t item_count,
            std::shared_ptr<token_counters> &counters
    ) {
        for (int i = 0; i < item_count; ++i) {
            victim.emplace_back(counters, i);
        }
    }

    SCENARIO("small_vector push_back / emplace operations", "[small_vector]") {

        auto counters = std::make_shared<token_counters>();

        auto make_item = [&](int value) {
            return test_item(counters, value);
        };

        GIVEN("a vector containing some items") {

            size_t const INITIAL_SIZE = 10;
            size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;

            small_vector<test_item, INITIAL_CAPACITY> victim;

            init_items(victim, INITIAL_SIZE, counters);

            counters->reset();

            WHEN("an item is added as lvalue") {
                const auto ADDED_VALUE = make_item(999);

                victim.push_back(ADDED_VALUE);

                THEN("size must change and value must be present by copy") {
                    REQUIRE(victim.size() == INITIAL_SIZE + 1);
                    REQUIRE(victim.back() == ADDED_VALUE);

                    REQUIRE(counters->check_moves(0));
                    REQUIRE(counters->check_copies(1));
                }
            }WHEN("an item is added as rvalue") {
                int const ITEM_VALUE = 999;
                auto added_value = make_item(ITEM_VALUE);

                victim.push_back(std::move(added_value));

                THEN("size must change and value must be present by move") {
                    REQUIRE(victim.size() == INITIAL_SIZE + 1);
                    REQUIRE(victim.back().value() == ITEM_VALUE);

                    REQUIRE(counters->check_moves(1));
                    REQUIRE(counters->check_copies(0));
                }
            }WHEN("an item is created in place") {
                int const ITEM_VALUE = 999;

                victim.emplace_back(counters, ITEM_VALUE);

                THEN("size must change and value must be present by move") {
                    REQUIRE(victim.size() == INITIAL_SIZE + 1);
                    REQUIRE(victim.back().value() == ITEM_VALUE);

                    REQUIRE(counters->check_ctors(1));
                    REQUIRE(counters->check_moves(0));
                    REQUIRE(counters->check_copies(0));
                }
            }
        } GIVEN("a full vector") {
            size_t const INITIAL_CAPACITY = 10;
            size_t const EXPECTED_CAPACITY = 16;

            small_vector<test_item, INITIAL_CAPACITY> victim;

            init_items(victim, INITIAL_CAPACITY, counters);
            counters->reset();

            WHEN("an item is added as lvalue") {
                const auto ADDED_VALUE = make_item(999);

                victim.push_back(ADDED_VALUE);

                THEN("capacity and size must change and value must be present by copy") {
                    REQUIRE(victim.capacity() == EXPECTED_CAPACITY);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim.back() == ADDED_VALUE);

                    REQUIRE(counters->check_moves(INITIAL_CAPACITY));
                    REQUIRE(counters->check_copies(1));
                }
            }WHEN("an item is added as rvalue") {
                int const ITEM_VALUE = 999;
                auto added_value = make_item(ITEM_VALUE);

                victim.push_back(std::move(added_value));

                THEN("capacity and size must change and value must be present by move") {
                    REQUIRE(victim.capacity() == EXPECTED_CAPACITY);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim.back().value() == ITEM_VALUE);

                    REQUIRE(counters->check_moves(1 + INITIAL_CAPACITY));
                    REQUIRE(counters->check_copies(0));
                }
            }WHEN("an item is created in place at the back") {
                int const ITEM_VALUE = 999;

                victim.emplace_back(counters, ITEM_VALUE);

                THEN("size must change and value must be present by move") {
                    REQUIRE(victim.capacity() == EXPECTED_CAPACITY);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim.back().value() == ITEM_VALUE);

                    REQUIRE(counters->check_ctors(1));
                    REQUIRE(counters->check_moves(INITIAL_CAPACITY));
                    REQUIRE(counters->check_copies(0));
                }
            }
        }
    }

    SCENARIO("small_vector construction", "[small_vector]") {
        size_t const INITIAL_SIZE = 10;
        size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;

        GIVEN("a small_vector construction") {
            WHEN("default") {
                small_vector<int, INITIAL_CAPACITY> victim;

                THEN("capacity must be as set") {
                    REQUIRE(victim.empty());
                    REQUIRE(victim.capacity() == INITIAL_CAPACITY);
                }
            }WHEN("copy iterators") {
                std::vector<int> const EXPECTANCY = {
                        1, 3, 5, 8, 13
                };

                small_vector<int, INITIAL_CAPACITY> victim(
                        EXPECTANCY.begin(), EXPECTANCY.end()
                );

                THEN("construction by iterative copy") {
                    REQUIRE(victim.size() == EXPECTANCY.size());
                    REQUIRE(victim.capacity() == INITIAL_CAPACITY);
                    REQUIRE(std::equal(
                            EXPECTANCY.begin(), EXPECTANCY.end(),
                            victim.begin(), victim.end()
                    ));
                }
            }WHEN("copy constructor") {
                small_vector<int, INITIAL_CAPACITY> const EXPECTANCY = {
                        1, 3, 5, 8, 13
                };

                small_vector<int, INITIAL_CAPACITY> victim = EXPECTANCY;

                THEN("construction by iterative copy") {
                    REQUIRE(victim.size() == EXPECTANCY.size());
                    REQUIRE(victim.capacity() == INITIAL_CAPACITY);
                    REQUIRE(std::equal(
                            EXPECTANCY.begin(), EXPECTANCY.end(),
                            victim.begin(), victim.end()
                    ));
                }
            }WHEN("move constructor") {

                auto counters = std::make_shared<token_counters>();

                small_vector<test_item, INITIAL_CAPACITY> source;

                init_items(source, INITIAL_SIZE, counters);

                small_vector<test_item, INITIAL_CAPACITY> victim = std::move(source);

                THEN("construction by move") {
                    REQUIRE(victim.size() == INITIAL_SIZE);

                    REQUIRE(counters->check_copies(0));
                    REQUIRE(counters->check_moves(INITIAL_SIZE));
                }
            }WHEN("initializer list constructor") {
                std::initializer_list<int> const EXPECTANCY = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

                small_vector<int, INITIAL_CAPACITY> victim = EXPECTANCY;

                THEN("construction by initializer list copy") {
                    REQUIRE(std::equal(
                            EXPECTANCY.begin(), EXPECTANCY.end(),
                            victim.begin(), victim.end()
                    ));
                }
            }
        }
    }

    SCENARIO("small_vector destruction", "[small_vector]") {
        GIVEN("a structure item type") {

            auto counters = std::make_shared<token_counters>();

            WHEN("items are directly contained") {

                {
                    small_vector<test_item, 10> victim;
                    victim.emplace_back(counters, 321);
                    // reset counter to cancel the destruction of the copied parameter
                    counters->reset();
                }

                THEN("items destructors should be invoked") {
                    REQUIRE(counters->check_dtors(1));
                }
            }WHEN("items are heap allocated") {
                auto item = std::make_unique<test_item>(counters);

                {
                    small_vector<test_item *, 10> victim;
                    victim.push_back(item.get());
                    // reset counter to cancel the destruction of the copied parameter
                    counters->reset();
                }

                THEN("items destructors should not be invoked") {
                    REQUIRE(counters->check_dtors(0));
                }
            }
        }
    }

    SCENARIO("small_vector attribution", "[small_vector]") {

        size_t const INITIAL_SIZE = 10;
        size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;

        auto counters = std::make_shared<token_counters>();

        auto make_item = [&](int value) {
            return test_item(counters, value);
        };

        small_vector<test_item, INITIAL_CAPACITY> victim;

        GIVEN("lvalue assigned vector") {
            WHEN("assigned vector lesser than capacity") {
                small_vector<test_item, INITIAL_CAPACITY> assigned;

                init_items(assigned, INITIAL_CAPACITY, counters);
                counters->reset();

                victim = assigned;

                THEN("assignment by copy") {
                    REQUIRE(victim.capacity() == assigned.capacity());
                    REQUIRE(victim.size() == assigned.size());

                    REQUIRE(counters->check_copies(assigned.size()));
                    REQUIRE(counters->check_moves(0));

                    REQUIRE(std::equal(
                            assigned.begin(), assigned.end(),
                            victim.begin(), victim.end()
                    ));
                }
            }WHEN("assigned vector greater than victim capacity") {
                size_t const INITIAL_ASSIGNED_SIZE = INITIAL_CAPACITY + INITIAL_SIZE;

                size_t const EXPECTED_CAPACITY = next_pow_2(
                        INITIAL_ASSIGNED_SIZE
                );

                small_vector<test_item, INITIAL_ASSIGNED_SIZE> assigned;

                init_items(assigned, INITIAL_ASSIGNED_SIZE, counters);
                counters->reset();

                victim = assigned;

                THEN("assignment by copy and capacity growth") {
                    REQUIRE(victim.capacity() == EXPECTED_CAPACITY);
                    REQUIRE(victim.size() == INITIAL_ASSIGNED_SIZE);

                    REQUIRE(counters->check_copies(INITIAL_ASSIGNED_SIZE));
                    REQUIRE(counters->check_moves(0));

                    REQUIRE(std::equal(
                            assigned.begin(), assigned.end(),
                            victim.begin(), victim.end()
                    ));
                }
            } WHEN ("assign self does nothing") {
                init_items(victim, INITIAL_SIZE, counters);
                counters->reset();

                decltype(victim) *ptr = nullptr;

                REQUIRE_NOTHROW(ptr = &(victim = victim));

                REQUIRE(ptr == &victim);

                REQUIRE(counters->check_copies(0));
                REQUIRE(counters->check_moves(0));
                REQUIRE(counters->check_dtors(0));

                REQUIRE (victim.capacity() == INITIAL_CAPACITY);
                REQUIRE (victim.size() == INITIAL_SIZE);
            }
        }GIVEN("rvalue vector") {
            WHEN("assigned vector lesser than capacity") {
                small_vector<test_item, INITIAL_CAPACITY> assigned;

                init_items(assigned, INITIAL_CAPACITY, counters);
                counters->reset();

                victim = std::move(assigned);

                THEN("assignment by move") {
                    REQUIRE(victim.capacity() == INITIAL_CAPACITY);
                    REQUIRE(victim.size() == INITIAL_CAPACITY);

                    REQUIRE(counters->check_copies(0));
                    REQUIRE(counters->check_moves(INITIAL_CAPACITY));
                }
            }WHEN("assigned vector greater than victim capacity") {
                size_t const INITIAL_ASSIGNED_SIZE = INITIAL_CAPACITY + INITIAL_SIZE;
                size_t const EXPECTED_CAPACITY = next_pow_2(INITIAL_ASSIGNED_SIZE);

                small_vector<test_item, INITIAL_ASSIGNED_SIZE> assigned;

                init_items(assigned, INITIAL_ASSIGNED_SIZE, counters);
                counters->reset();

                victim = std::move(assigned);

                THEN("assignment by move and capacity growth") {
                    REQUIRE(victim.capacity() == EXPECTED_CAPACITY);
                    REQUIRE(victim.size() == INITIAL_ASSIGNED_SIZE);

                    REQUIRE(counters->check_copies(0));
                    REQUIRE(counters->check_moves(INITIAL_ASSIGNED_SIZE));
                }
            }
        }GIVEN("initialize a list") {

            std::initializer_list<test_item> expectancy = {
                    make_item(1),
                    make_item(2),
                    make_item(3),
                    make_item(4),
                    make_item(5)
            };

            counters->reset();

            WHEN("on assignment") {

                victim = expectancy;

                THEN("copy assignment of initialized list items") {
                    REQUIRE(victim.size() == expectancy.size());

                    REQUIRE(counters->check_copies(expectancy.size()));
                    REQUIRE(counters->check_moves(0));

                    REQUIRE(std::equal(
                            expectancy.begin(), expectancy.end(),
                            victim.begin(), victim.end()
                    ));
                }
            }
        }
    }

    SCENARIO("small_vector assign", "[small_vector]") {

        size_t const INITIAL_SIZE = 10;
        size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;

        auto counters = std::make_shared<token_counters>();

        auto make_item = [&](int v) {
            return test_item(counters, v);
        };

        small_vector<test_item, INITIAL_CAPACITY> victim;

        GIVEN("assign operation call") {
            WHEN("count and default value") {
                test_item item = make_item(999);

                victim.assign(INITIAL_SIZE, item);

                THEN("should be assigned to n copies of the default value") {
                    REQUIRE(
                            std::count(victim.begin(), victim.end(), item) == INITIAL_SIZE
                    );
                }
            }WHEN("count and default value grows") {
                test_item item = make_item(999);

                size_t const ITEM_COUNT = INITIAL_CAPACITY + 2;
                size_t const EXPECTED_CAPACITY = 32;

                victim.assign(ITEM_COUNT, item);

                THEN("should be assigned to n copies of the default value and grows to next pow2") {
                    REQUIRE(std::count(victim.begin(), victim.end(), item) == ITEM_COUNT);
                    REQUIRE(victim.capacity() == EXPECTED_CAPACITY);
                }
            }WHEN("iterators") {
                small_vector<test_item, INITIAL_SIZE> expectancy;
                init_items(expectancy, INITIAL_SIZE, counters);

                counters->reset();

                victim.assign(expectancy.begin(), expectancy.end());

                THEN("should be assigned to copies of iterated values") {
                    REQUIRE(std::equal(
                            expectancy.begin(), expectancy.end(),
                            victim.begin(), victim.end()
                    ));

                    REQUIRE(counters->check_moves(0));
                    REQUIRE(counters->check_copies(INITIAL_SIZE));
                }
            }WHEN("initializer list") {

                std::initializer_list<test_item> expectancy = {
                        make_item(1),
                        make_item(2),
                        make_item(3),
                        make_item(4),
                        make_item(5)
                };

                counters->reset();

                victim.assign(expectancy);

                THEN("should be assigned to copies of initializer list values") {
                    REQUIRE(std::equal(
                            expectancy.begin(), expectancy.end(),
                            victim.begin(), victim.end()
                    ));

                    REQUIRE(counters->check_moves(0));
                    REQUIRE(counters->check_copies(expectancy.size()));
                }
            }
        }
    }

    SCENARIO("small_vector accessor operations", "[small_vector]") {

        size_t const INITIAL_SIZE = 10;
        size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;

        small_vector<int, INITIAL_CAPACITY> victim = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        GIVEN("operation at") {
            WHEN("within range") {
                THEN("should return proper value") {
                    REQUIRE(victim.at(5) == victim.data()[5]);
                }
            }WHEN("out of range") {
                THEN("should throw out of range exception") {
                    REQUIRE_THROWS_AS(victim.at(INITIAL_SIZE + INITIAL_SIZE), std::out_of_range);
                }
            }
        }GIVEN("operator indexer") {
            WHEN("within range") {
                THEN("should return proper value") {
                    REQUIRE(victim[5] == victim.data()[5]);
                }
            }
        }GIVEN("operation back") {
            WHEN("with elements") {
                THEN("should return last item") {
                    REQUIRE(victim.back() == victim.data()[9]);
                }
            }WHEN("const") {
                auto const &const_victim = victim;
                THEN("should return last item") {
                    REQUIRE(const_victim.back() == const_victim.data()[9]);
                }
            }
        }GIVEN("operation front") {
            WHEN("with elements") {
                THEN("should return first item") {
                    REQUIRE(victim.front() == victim.data()[0]);
                }
            }WHEN("const") {
                auto const &const_victim = victim;
                THEN("should return first item") {
                    REQUIRE(const_victim.front() == const_victim.data()[0]);
                }
            }
        }GIVEN("operation data") {
            WHEN("with elements") {
                THEN("should return pointer to data") {
                    REQUIRE(victim.data()[0] == victim.front());
                }
            }WHEN("const") {
                THEN("should return pointer to data") {
                    auto const &const_victim = victim;
                    REQUIRE(const_victim.data()[0] == const_victim.front());
                }
            }
        }
    }

    SCENARIO("small_vector accessor reference editing", "[small_vector]") {

        size_t const INITIAL_SIZE = 10;
        size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;

        small_vector<int, INITIAL_CAPACITY> victim = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        std::random_device rdev;
        std::default_random_engine re(rdev());

        std::uniform_int_distribution<int> value_dist(1000, 10000);

        GIVEN("operation at") {
            WHEN("within range") {
                auto change_value = value_dist(re);

                victim.at(5) = change_value;

                THEN("should have edited value") {
                    REQUIRE(victim[5] == change_value);
                }
            }
        }GIVEN("operator indexer") {
            WHEN("within range") {
                auto change_value = value_dist(re);

                victim[5] = change_value;

                THEN("should have edited value") {
                    REQUIRE(victim[5] == change_value);
                }
            }
        }GIVEN("operation back") {
            WHEN("with elements") {
                auto change_value = value_dist(re);

                victim.back() = change_value;

                THEN("should have edited value") {
                    REQUIRE(victim.back() == change_value);
                }
            }
        }GIVEN("operation front") {
            WHEN("with elements") {
                auto change_value = value_dist(re);

                victim.front() = change_value;

                THEN("should have edited value") {
                    REQUIRE(victim.front() == change_value);
                }
            }
        }
    }

    SCENARIO("small_vector state operations", "[small_vector]") {

        size_t const INITIAL_SIZE = 10;
        size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;

        small_vector<int, INITIAL_CAPACITY> victim = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        GIVEN("empty") {
            WHEN("is empty") {
                small_vector<int, INITIAL_CAPACITY> empty_victim;

                THEN("empty should be true") {
                    REQUIRE(empty_victim.empty());
                }
            }WHEN("is not empty") {
                THEN("empty should be false") {
                    REQUIRE(!victim.empty());
                }
            }
        }GIVEN("size") {
            WHEN("called") {
                THEN("should return the appropriate number of contained items") {
                    REQUIRE(victim.size() == INITIAL_SIZE);
                }
            }
        }GIVEN("max_size") {
            WHEN("called") {
                THEN("should return the expected maximum number of contained items") {
                    constexpr auto expected_max_size = std::numeric_limits<typename small_vector<int, INITIAL_CAPACITY>::size_type>::max();
                    REQUIRE(victim.max_size() == expected_max_size);
                }
            }
        }
    }

    SCENARIO("small_vector manage capacity", "[small_vector]") {

        size_t const INITIAL_SIZE = 10;
        size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;

        small_vector<int, INITIAL_CAPACITY> victim = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        GIVEN("reserve") {
            WHEN("called with larger capacity") {
                auto const new_capacity = INITIAL_CAPACITY * 2;

                victim.reserve(new_capacity);

                THEN("small_vector capacity should change to the nearest pow of 2") {
                    auto const expected_capacity = next_pow_2(new_capacity);
                    REQUIRE(victim.capacity() == expected_capacity);
                }
            }WHEN("called with smaller capacity") {
                auto const new_capacity = INITIAL_CAPACITY / 2;

                victim.reserve(new_capacity);

                THEN("small_vector capacity should have remained the same") {
                    REQUIRE(victim.capacity() == INITIAL_CAPACITY);
                }
            }
        }
    }

    SCENARIO("small_vector shrink_to_fit", "[small_vector]") {

        size_t const INITIAL_SIZE = 10;
        size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;

        small_vector<int, INITIAL_CAPACITY> victim = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        GIVEN("shrink_to_fit") {
            WHEN("called with 'large' small_vector") {
                auto const new_capacity = INITIAL_CAPACITY * 2;
                victim.reserve(new_capacity);

                THEN("small_vector should grow to 'non local'") {
                    REQUIRE(!victim.is_local());
                }

                victim.shrink_to_fit();
                THEN("small_vector capacity should shrink to data") {
                    REQUIRE(victim.capacity() == INITIAL_CAPACITY);
                }THEN("small_vector 'small' storage should be restored") {
                    REQUIRE(victim.is_local());
                }
            }WHEN("called with 'small' small_vector") {
                victim.shrink_to_fit();

                THEN("small_vector capacity should remain the same") {
                    REQUIRE(victim.capacity() == INITIAL_CAPACITY);
                }
            }
        }
    }

    SCENARIO("small_vector clear", "[small_vector]") {
        constexpr size_t INITIAL_SIZE = 10;
        constexpr size_t INITIAL_CAPACITY = INITIAL_SIZE * 2;

        GIVEN("a small_vector containing items") {
            small_vector<int, INITIAL_CAPACITY> victim = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

            WHEN("clear called") {
                victim.clear();

                THEN("items should be removed") {
                    REQUIRE(victim.empty());
                    REQUIRE(victim.capacity() == INITIAL_CAPACITY);
                }
            }
        }GIVEN("an empty small_vector") {
            small_vector<int, INITIAL_CAPACITY> victim;

            WHEN("clear called") {

                REQUIRE_NOTHROW (victim.clear());

                THEN("nothing should happen") {
                    REQUIRE(victim.empty());
                    REQUIRE(victim.capacity() == INITIAL_CAPACITY);
                }
            }
        }
    }

    SCENARIO("small_vector insert", "[small_vector]") {
        size_t const INITIAL_SIZE = 10;
        size_t const INITIAL_CAPACITY = INITIAL_SIZE * 2;
        size_t const expected_capacity = 32;
        size_t const item_value = 1234;

        auto counters = std::make_shared<token_counters>();

        auto make_item = [&](int v) {
            return test_item(counters, v);
        };

        std::random_device rdev;
        std::default_random_engine re(rdev());

        std::uniform_int_distribution<int> value_dist(1000, 10000);

        GIVEN("empty small_vector") {

            small_vector<test_item, INITIAL_CAPACITY> victim;

            WHEN("insert lvalue at begin") {
                auto item = make_item(item_value);

                counters->reset();

                victim.insert(victim.begin(), item);

                THEN("value should have been inserted in the first position") {
                    REQUIRE(victim.size() == 1);
                    REQUIRE(victim.back() == item);

                    REQUIRE(counters->check_copies(1));
                    REQUIRE(counters->check_moves(0));
                }
            }WHEN("insert rvalue at begin") {
                auto item = make_item(item_value);
                counters->reset();

                victim.insert(victim.begin(), std::move(item));

                THEN("value should have been inserted in the first position") {
                    REQUIRE(victim.size() == 1);
                    REQUIRE(victim.back().value() == item_value);

                    REQUIRE(counters->check_copies(0));
                    REQUIRE(counters->check_moves(1));
                }
            }WHEN("insert n lvalues at begin") {
                size_t const insert_count = 5;

                auto item = make_item(item_value);
                counters->reset();

                victim.insert(victim.begin(), insert_count, item);

                THEN("n values should have been inserted in the first position") {
                    REQUIRE(victim.size() == insert_count);

                    auto item_count = std::count(
                            victim.begin(), victim.end(),
                            item
                    );

                    REQUIRE(item_count == insert_count);
                    REQUIRE(counters->check_copies(insert_count));
                    REQUIRE(counters->check_moves(0));
                }
            }WHEN("insert lvalue at invalid position") {
                auto item = make_item(item_value);

                counters->reset();

                THEN("throw out_of_range") {
                    REQUIRE_THROWS_AS(victim.insert(victim.end() + 1, item), std::out_of_range);
                }
            }WHEN("insert rvalue at invalid position") {
                auto item = make_item(item_value);

                counters->reset();

                THEN("throw out_of_range") {
                    REQUIRE_THROWS_AS(victim.insert(victim.end() + 1, std::move(item)), std::out_of_range);
                }
            }WHEN("insert n lvalues at invalid position") {
                size_t const insert_count = 5;

                auto item = make_item(item_value);
                counters->reset();

                THEN("throw out_of_range") {
                    REQUIRE_THROWS_AS(victim.insert(victim.end() + 1, insert_count, item), std::out_of_range);
                }
            }WHEN("insert 0 lvalues at invalid position") {
                size_t const insert_count = 5;

                auto item = make_item(item_value);
                counters->reset();

                victim.insert(victim.end() + 1, 0, item);
                THEN("nothing should happen") {
                    REQUIRE(victim.empty());
                }
            }
        }GIVEN("filled small_vector") {

            small_vector<test_item, INITIAL_CAPACITY> victim;

            init_items(victim, INITIAL_CAPACITY, counters);
            counters->reset();

            WHEN("insert lvalue at begin") {
                auto item = make_item(item_value);

                counters->reset();

                victim.insert(victim.begin(), item);

                THEN("value should have been inserted in the first position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim.front() == item);

                    REQUIRE(counters->check_copies(1));
// TODO: optimize the moving around of things
// REQUIRE(counters.check_moves(INITIAL_CAPACITY));
                }
            }WHEN("insert lvalue at middle") {
                auto item = make_item(item_value);

                counters->reset();

                size_t offset = victim.size() / 2;
                victim.insert(victim.begin() + offset, item);

                THEN("value should have been inserted in the middle position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim[offset] == item);

                    REQUIRE(counters->check_copies(1));
// TODO: optimize the moving around of things
// REQUIRE(counters.check_moves(offset));
                }
            }WHEN("insert lvalue at end") {
                auto item = make_item(item_value);

                counters->reset();

                victim.insert(victim.end(), item);

                THEN("value should have been inserted in the last position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim.back() == item);

                    REQUIRE(counters->check_copies(1));
                    REQUIRE(counters->check_moves(INITIAL_CAPACITY));
                }
            }WHEN("insert lvalue at invalid position") {
                auto item = make_item(item_value);

                counters->reset();

                THEN("throw out_of_range") {
                    REQUIRE_THROWS_AS(victim.insert(victim.end() + 1, item), std::out_of_range);
                }
            }WHEN("insert rvalue at begin") {
                auto item = make_item(item_value);

                counters->reset();

                victim.insert(victim.begin(), std::move(item));

                THEN("value should have been inserted in the first position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim.front().value ()== item_value);

                    REQUIRE(counters->check_copies(0));
// TODO: optimize the moving around of things
// REQUIRE(counters.check_moves(??));
                }
            }WHEN("insert rvalue at middle") {
                auto item = make_item(item_value);

                counters->reset();

                size_t offset = victim.size() / 2;
                victim.insert(victim.begin() + offset, std::move(item));

                THEN("value should have been inserted in the middle position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim[offset].value() == item_value);

                    REQUIRE(counters->check_copies(0));
// TODO: optimize the moving around of things
// REQUIRE(counters.check_moves(???));
                }
            }WHEN("insert rvalue at end") {
                auto item = make_item(item_value);

                counters->reset();

                victim.insert(victim.end(), std::move(item));

                THEN("value should have been inserted in the last position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim.back().value() == item_value);

                    REQUIRE(counters->check_copies(0));
                    REQUIRE(counters->check_moves(INITIAL_CAPACITY + 1));
                }
            }WHEN("insert rvalue at invalid position") {
                auto item = make_item(item_value);

                counters->reset();

                THEN("throw out_of_range") {
                    REQUIRE_THROWS_AS(victim.insert(victim.end() + 1, std::move(item)), std::out_of_range);
                }
            }WHEN("insert n lvalues at begin") {
                size_t const insert_count = 5;

                auto item = make_item(item_value);
                counters->reset();

                victim.insert(victim.begin(), insert_count, item);

                THEN("n values should have been inserted in the first position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + insert_count);

                    auto item_count = std::count(
                            victim.begin(), victim.end(),
                            item
                    );

                    REQUIRE(item_count == insert_count);
                    REQUIRE(counters->check_copies(insert_count));
// TODO: optimize the moving around of things
// REQUIRE(counters.check_moves(???));
                }
            }WHEN("insert n lvalues at middle") {
                size_t const insert_count = 5;

                auto item = make_item(item_value);
                counters->reset();

                size_t offset = victim.size() / 2;
                victim.insert(victim.begin() + offset, insert_count, item);

                THEN("value should have been inserted in the middle position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + insert_count);

                    auto item_count = std::count(
                            victim.begin(), victim.end(),
                            item
                    );

                    REQUIRE(item_count == insert_count);

                    REQUIRE(counters->check_copies(insert_count));
// TODO: optimize the moving around of things
// REQUIRE(counters.check_moves(???));
                }
            }WHEN("insert n lvalues at end") {
                size_t const insert_count = 5;

                auto item = make_item(item_value);
                counters->reset();

                victim.insert(victim.end(), insert_count, item);

                THEN("value should have been inserted in the last position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + insert_count);

                    auto item_count = std::count(
                            victim.begin(), victim.end(),
                            item
                    );

                    REQUIRE(item_count == insert_count);

                    REQUIRE(counters->check_copies(insert_count));
                    REQUIRE(counters->check_moves(INITIAL_CAPACITY));
                }
            }WHEN("emplace at begin") {
                counters->reset();

                victim.emplace(victim.begin(), counters, item_value);

                THEN("value should have been inserted in the first position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim.front().value() == item_value);

                    REQUIRE(counters->check_ctors(1));
                    REQUIRE(counters->check_copies(0));
// TODO: optimize the moving around of things
// REQUIRE(counters.check_moves(INITIAL_CAPACITY));
                }
            }WHEN("emplace at middle") {
                counters->reset();

                size_t offset = victim.size() / 2;
                victim.emplace(victim.begin() + offset, counters, item_value);

                THEN("value should have been inserted in the middle position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim[offset].value() == item_value);

                    REQUIRE(counters->check_ctors(1));
                    REQUIRE(counters->check_copies(0));
// TODO: optimize the moving around of things
// REQUIRE(counters.check_moves(INITIAL_CAPACITY));
                }
            }WHEN("emplace at end") {
                counters->reset();

                victim.emplace(victim.end(), counters, item_value);

                THEN("value should have been inserted in the last position") {
                    REQUIRE(victim.capacity() == expected_capacity);
                    REQUIRE(victim.size() == INITIAL_CAPACITY + 1);
                    REQUIRE(victim.back().value() == item_value);

                    REQUIRE(counters->check_ctors(1));
                    REQUIRE(counters->check_copies(0));
                    REQUIRE(counters->check_moves(INITIAL_CAPACITY));
                }
            }WHEN("emplace at invalid position") {
                counters->reset();

                THEN("throw out_of_range") {
                    REQUIRE_THROWS_AS(victim.emplace(victim.end() + 1, counters, item_value), std::out_of_range);
                }
            }
        }
    }

    SCENARIO ("small_vector invalid operations", "[small_vector]") {
        GIVEN ("empty small_vector") {
            small_vector<int, 10> victim;
            small_vector<int, 10> const CONST_VICTIM;

            WHEN ("calling front") {
                REQUIRE_THROWS_AS (victim.front(), std::runtime_error);
            }WHEN ("calling front const") {
                REQUIRE_THROWS_AS (CONST_VICTIM.front(), std::runtime_error);
            }WHEN ("calling back") {
                REQUIRE_THROWS_AS (victim.back(), std::runtime_error);
            }WHEN ("calling back const") {
                REQUIRE_THROWS_AS (CONST_VICTIM.back(), std::runtime_error);
            }WHEN ("erasing") {
                REQUIRE_THROWS_AS (victim.erase(victim.begin()), std::out_of_range);
            }
        }GIVEN ("filled small_vector") {
            small_vector<int, 10> victim = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

            WHEN ("erasing") {
                REQUIRE_THROWS_AS (victim.erase(victim.begin() + 11, victim.begin() + 15), std::out_of_range);
                REQUIRE_THROWS_AS (victim.erase(victim.begin() + 5, victim.begin()), std::out_of_range);
                REQUIRE_THROWS_AS (victim.erase(victim.begin(), victim.begin() + 15), std::out_of_range);
            }
        }
    }

    SCENARIO ("small_vector erase", "[small_vector]") {
        const size_t CAPACITY = 10;

        GIVEN ("filled small_vector") {
            small_vector<int, CAPACITY> victim = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

            WHEN ("erasing at beginning") {
                small_vector<int, CAPACITY> const EXPECTED_ELEMENTS = {2, 3, 4, 5, 6, 7, 8, 9, 10};

                auto it = victim.erase(victim.begin());

                THEN ("first element should be removed") {
                    REQUIRE (std::equal(
                            EXPECTED_ELEMENTS.begin(),
                            EXPECTED_ELEMENTS.end(),
                            victim.begin(),
                            victim.end()));
                }
            }WHEN ("erasing at middle") {
                small_vector<int, CAPACITY> const EXPECTED_ELEMENTS = {1, 2, 3, 4, 6, 7, 8, 9, 10};

                auto it = victim.erase(victim.begin() + 4);

                THEN ("middle element should be removed") {
                    REQUIRE (std::equal(
                            EXPECTED_ELEMENTS.begin(),
                            EXPECTED_ELEMENTS.end(),
                            victim.begin(),
                            victim.end()));
                }
            }WHEN ("erasing at the end") {
                small_vector<int, CAPACITY> const EXPECTED_ELEMENTS = {1, 2, 3, 4, 5, 6, 7, 8, 9};

                auto it = victim.erase(victim.end() - 1);

                THEN ("last element should be removed") {
                    REQUIRE (std::equal(
                            EXPECTED_ELEMENTS.begin(),
                            EXPECTED_ELEMENTS.end(),
                            victim.begin(),
                            victim.end()));
                }
            }WHEN ("erasing several items at the middle") {
                small_vector<int, CAPACITY> const EXPECTED_ELEMENTS = {1, 2, 3, 4, 8, 9, 10};

                auto it = victim.erase(victim.begin() + 4, victim.begin() + 7);

                THEN ("middle element should be removed") {
                    REQUIRE (std::equal(
                            EXPECTED_ELEMENTS.begin(),
                            EXPECTED_ELEMENTS.end(),
                            victim.begin(),
                            victim.end()));
                }
            }
        }
    }

    SCENARIO ("small_vector swap", "[small_vector]") {
        const size_t CAPACITY = 10;

        small_vector<int, CAPACITY> const ORIGIN = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        small_vector<int, CAPACITY> const ORIGIN_B = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        small_vector<int, CAPACITY> const ORIGIN_C = {5, 4, 3, 2, 1};

        GIVEN ("empty small_vector") {
            small_vector<int, CAPACITY> victim_a;
            small_vector<int, CAPACITY> victim_b;

            WHEN ("swapping") {
                REQUIRE_NOTHROW (victim_a.swap(victim_b));
                REQUIRE (victim_a.empty());
                REQUIRE (victim_b.empty());
            }
        } GIVEN ("'large' small_vector") {
            size_t const DOUBLE_CAPACITY = CAPACITY * 2;

            auto counters = std::make_shared<token_counters>();

            small_vector<test_item, CAPACITY> victim_a;
            small_vector<test_item, CAPACITY> victim_b;

            init_items(victim_a, DOUBLE_CAPACITY, counters);
            init_items(victim_b, DOUBLE_CAPACITY, counters);

            WHEN ("swapping") {
                REQUIRE_NOTHROW (victim_a.swap(victim_b));

                REQUIRE (victim_a.size() == DOUBLE_CAPACITY);
                REQUIRE (victim_b.size() == DOUBLE_CAPACITY);
            }
        } GIVEN ("left hand filled small_vector") {
            small_vector<int, CAPACITY> victim_a = ORIGIN;
            small_vector<int, CAPACITY> victim_b;

            WHEN ("swapping") {
                REQUIRE_NOTHROW (victim_a.swap(victim_b));

                REQUIRE (victim_a.empty());
                REQUIRE (victim_b.size() == CAPACITY);

                REQUIRE (std::equal(
                        victim_b.begin(),
                        victim_b.end(),
                        ORIGIN.begin(),
                        ORIGIN.end()));
            }
        } GIVEN ("right hand filled small_vector") {
            small_vector<int, CAPACITY> victim_a;
            small_vector<int, CAPACITY> victim_b = ORIGIN;

            WHEN ("swapping") {
                REQUIRE_NOTHROW (victim_a.swap(victim_b));

                REQUIRE (victim_a.size() == CAPACITY);
                REQUIRE (victim_b.empty());

                REQUIRE (std::equal(
                        victim_a.begin(),
                        victim_a.end(),
                        ORIGIN.begin(),
                        ORIGIN.end()));
            }
        } GIVEN ("both filled small_vector") {
            small_vector<int, CAPACITY> victim_a = ORIGIN;
            small_vector<int, CAPACITY> victim_b = ORIGIN_B;

            WHEN ("swapping") {
                REQUIRE_NOTHROW (victim_a.swap(victim_b));

                REQUIRE (victim_a.size() == CAPACITY);
                REQUIRE (victim_b.size() == CAPACITY);

                REQUIRE (std::equal(
                        victim_b.begin(),
                        victim_b.end(),
                        ORIGIN.begin(),
                        ORIGIN.end()));

                REQUIRE (std::equal(
                        victim_a.begin(),
                        victim_a.end(),
                        ORIGIN_B.begin(),
                        ORIGIN_B.end()));
            }
        } GIVEN ("left hand diff sized small_vector") {
            small_vector<int, CAPACITY> victim_a = ORIGIN;
            small_vector<int, CAPACITY> victim_b = ORIGIN_C;

            WHEN ("swapping") {
                REQUIRE_NOTHROW (victim_a.swap(victim_b));

                REQUIRE (victim_a.size() == ORIGIN_C.size());
                REQUIRE (victim_b.size() == CAPACITY);

                REQUIRE (std::equal(
                        victim_b.begin(),
                        victim_b.end(),
                        ORIGIN.begin(),
                        ORIGIN.end()));

                REQUIRE (std::equal(
                        victim_a.begin(),
                        victim_a.end(),
                        ORIGIN_C.begin(),
                        ORIGIN_C.end()));
            }
        } GIVEN ("right hand diff sized small_vector") {
            small_vector<int, CAPACITY> victim_a = ORIGIN_C;
            small_vector<int, CAPACITY> victim_b = ORIGIN;

            WHEN ("swapping") {
                REQUIRE_NOTHROW (victim_a.swap(victim_b));

                REQUIRE (victim_a.size() == CAPACITY);
                REQUIRE (victim_b.size() == ORIGIN_C.size());

                REQUIRE (std::equal(
                        victim_b.begin(),
                        victim_b.end(),
                        ORIGIN_C.begin(),
                        ORIGIN_C.end()));

                REQUIRE (std::equal(
                        victim_a.begin(),
                        victim_a.end(),
                        ORIGIN.begin(),
                        ORIGIN.end()));
            }
        } GIVEN ("swap same small_vector") {
            small_vector<int, CAPACITY> victim = ORIGIN;

            WHEN ("swapping") {
                REQUIRE_NOTHROW (victim.swap(victim));
            }
        }
    }

}