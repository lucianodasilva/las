#include <catch2/catch_all.hpp>

#include <las/test/token.hpp>

namespace las::test {

    SCENARIO("Token Counters", "[token_counters]") {

        token_counters victim{};

        GIVEN ("newly instanced token_counters") {

            THEN("all counters should be zero") {
                REQUIRE (victim.check_copies(0));
                REQUIRE (victim.check_moves(0));
                REQUIRE (victim.check_ctors(0));
                REQUIRE (victim.check_dtors(0));
            }

            WHEN("inc_copy is called") {
                victim.inc_copy();
                THEN ("copy counter should be incremented, but no other counter should be modified") {
                    REQUIRE (victim.check_copies(1));
                    REQUIRE (victim.check_moves(0));
                    REQUIRE (victim.check_ctors(0));
                    REQUIRE (victim.check_dtors(0));
                }
            }

            WHEN("inc_move is called") {
                victim.inc_move();
                THEN ("move counter should be incremented, but no other counter should be modified") {
                    REQUIRE (victim.check_copies(0));
                    REQUIRE (victim.check_moves(1));
                    REQUIRE (victim.check_ctors(0));
                    REQUIRE (victim.check_dtors(0));
                }
            }

            WHEN("inc_ctor is called") {
                victim.inc_ctor();
                THEN ("ctor counter should be incremented, but no other counter should be modified") {
                    REQUIRE (victim.check_copies(0));
                    REQUIRE (victim.check_moves(0));
                    REQUIRE (victim.check_ctors(1));
                    REQUIRE (victim.check_dtors(0));
                }
            }

            WHEN("inc_dtor is called") {
                victim.inc_dtor();
                THEN ("dctor counter should be incremented, but no other counter should be modified") {
                    REQUIRE (victim.check_copies(0));
                    REQUIRE (victim.check_moves(0));
                    REQUIRE (victim.check_ctors(0));
                    REQUIRE (victim.check_dtors(1));
                }
            }
        }

    }

    SCENARIO ("Token constructors", "[token]") {

        GIVEN ("A token instanced without parameters") {
            token<int> const VICTIM;

            THEN ("'empty' value and counter instances should be initialized") {
                auto const &counters = VICTIM.counters();

                REQUIRE (counters.check_copies(0));
                REQUIRE (counters.check_moves(0));
                REQUIRE (counters.check_ctors(1)); // obviously the victim was constructed once
                REQUIRE (counters.check_dtors(0));

                REQUIRE (VICTIM.value() == 0);
            }
        }

        GIVEN ("a token instanced with value construction parameter") {
            int const TEST_VALUE = 1324;
            token<int> const VICTIM(TEST_VALUE);

            THEN ("value should be initialized with parameter value") {
                auto const &counters = VICTIM.counters();

                REQUIRE (counters.check_copies(0));
                REQUIRE (counters.check_moves(0));
                REQUIRE (counters.check_ctors(1));
                REQUIRE (counters.check_dtors(0));

                REQUIRE (VICTIM.value() == TEST_VALUE);
            }
        }

        GIVEN ("a token instanced with shared counter") {
            auto counters = std::make_shared<token_counters>();
            token<int> const VICTIM(counters);

            THEN ("counter instances should be set to shared counter") {

                REQUIRE (counters->check_copies(0));
                REQUIRE (counters->check_moves(0));
                REQUIRE (counters->check_ctors(1)); // obviously the victim was constructed once
                REQUIRE (counters->check_dtors(0));

                REQUIRE (VICTIM.value() == 0);
            }
        }

        GIVEN ("a token instanced with shared counter and value") {
            int const TEST_VALUE = 1324;
            auto counters = std::make_shared<token_counters>();
            token<int> const VICTIM(counters, TEST_VALUE);

            THEN ("counter instances should be set to shared counter, value should be initialized to constructor value") {

                REQUIRE (counters->check_copies(0));
                REQUIRE (counters->check_moves(0));
                REQUIRE (counters->check_ctors(1)); // obviously the victim was constructed once
                REQUIRE (counters->check_dtors(0));

                REQUIRE (VICTIM.value() == TEST_VALUE);
            }
        }

    }

    SCENARIO ("Token operators", "[token]") {
        int const TEST_VALUE = 1324;
        int const ALTER_VALUE = 6576;

        auto counters = std::make_shared<token_counters>();

        token<int> victim (counters, TEST_VALUE);

        WHEN ("moving into a new variable") {
            token<int> other_victim (std::move (victim));

            THEN ("victims counters and value should not be the same") {
                auto * victim_counters = &victim.counters ();

                REQUIRE_FALSE(victim_counters == counters.get());
                REQUIRE_FALSE (victim.value () == TEST_VALUE);

                REQUIRE(victim_counters->check_moves(1));
            }
            AND_THEN("other victim should have the moved data") {
                auto * victim_counters = &other_victim.counters ();

                REQUIRE(victim_counters == counters.get());
                REQUIRE(other_victim.value () == TEST_VALUE);
                REQUIRE(victim_counters->check_moves(1));
            }
        }

        WHEN ("moving into a existing variable") {
            auto other_counters = std::make_shared<token_counters>();
            token<int> other_victim (other_counters, ALTER_VALUE);

            other_victim = std::move (victim);

            THEN ("victims and other_victims contents should swap") {
                auto * victim_counters = &victim.counters ();

                REQUIRE (victim_counters == other_counters.get());
                REQUIRE (victim.value () == ALTER_VALUE);
                REQUIRE (victim_counters->check_moves(1));

                auto * other_victim_counters = &other_victim.counters ();

                REQUIRE(other_victim_counters == counters.get());
                REQUIRE(other_victim.value () == TEST_VALUE);
                REQUIRE(other_victim_counters->check_moves(1));
            }
        }

        WHEN ("copying into a new variable") {
            token<int> other_victim (victim);

            THEN ("other_victim should contain a copied value and shared counter instance") {
                auto * other_victim_counters = &other_victim.counters ();

                REQUIRE (other_victim_counters == counters.get());

                REQUIRE (counters->check_copies(1));
                REQUIRE (counters->check_moves(0));
                REQUIRE (counters->check_ctors(1));
                REQUIRE (counters->check_dtors(0));

                REQUIRE (other_victim.value () == TEST_VALUE);
            }
        }

        WHEN ("assigning") {
            token<int> other_victim;

            other_victim = victim;

            THEN ("other_victim should contain a copied value and shared counter instance") {
                auto * other_victim_counters = &other_victim.counters ();

                REQUIRE (other_victim_counters == counters.get());

                REQUIRE (counters->check_copies(1));
                REQUIRE (counters->check_moves(0));
                REQUIRE (counters->check_ctors(1));
                REQUIRE (counters->check_dtors(0));

                REQUIRE (other_victim.value () == TEST_VALUE);
            }
        }

        WHEN ("assigning to itself") {
            victim = victim;

            THEN ("nothing should happen") {
                REQUIRE (counters->check_copies(0));
                REQUIRE (counters->check_moves(0));
                REQUIRE (counters->check_ctors(1));
                REQUIRE (counters->check_dtors(0));

                REQUIRE (victim.value () == TEST_VALUE);
            }
        }

    }

}