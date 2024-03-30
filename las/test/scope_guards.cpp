#include <catch2/catch_all.hpp>
#include <las/las.h>

namespace las::test {

    TEST_CASE ("scope_exit exit", "[scope_exit]") {
        bool token = false;

        {
            auto guard = scope_exit ([&]{
                token = true;
            });
        }

        REQUIRE (token == true);
    }

    TEST_CASE ("scope_exit exit dismissed", "[scope_exit]") {
        bool token = false;

        {
            auto guard = scope_exit ([&]{
                token = true;
            });

            guard.dismiss ();
        }

        REQUIRE (token == false);
    }

    TEST_CASE ("scope_success exit", "[scope_success]") {
        bool token = false;

        {
            auto guard = scope_success ([&]{
                token = true;
            });
        }

        REQUIRE (token == true);
    }

    TEST_CASE ("scope_success exit dismiss", "[scope_success]") {
        bool token = false;

        {
            auto guard = scope_success ([&]{
                token = true;
            });

            guard.dismiss();
        }

        REQUIRE (token == false);
    }

    TEST_CASE ("scope_success exit failed", "[scope_success]") {
        bool token = false;

        try {
            auto guard = scope_success ([&]{
                token = true;
            });

            throw std::exception ();
        } catch (...) {}

        REQUIRE (token == false);
    }

    TEST_CASE ("scope_fail exit", "[scope_fail]") {
        bool token = false;

        {
            auto guard = scope_fail ([&]{
                token = true;
            });
        }

        REQUIRE (token == false);
    }

    TEST_CASE ("scope_fail exit failed", "[scope_fail]") {
        bool token = false;

        try {
            auto guard = scope_fail ([&]{
                token = true;
            });

            throw std::exception ();
        } catch (...) {}

        REQUIRE (token == true);
    }

    TEST_CASE ("scope_fail exit failed dismiss", "[scope_fail]") {
        bool token = false;

        try {
            auto guard = scope_fail ([&]{
                token = true;
            });

            guard.dismiss ();

            throw std::exception ();
        } catch (...) {}

        REQUIRE (token == false);
    }

}