#include <catch2/catch_all.hpp>
#include <las/view.hpp>
#include <array>


namespace las::test {

    std::vector<std::string> to_string_vector(std::vector<std::string_view> const &source) {
        return {source.begin(), source.end()};
    }

    inline std::string to_string(std::string_view view) {
        return std::string(view);
    }

    char const TEST_ARRAY[] = {'v', 'i', 'e', 'w', '_', 'u', 'n', 'd', 'e', 'r', '_', 't', 'e', 's', 't'};
    char const BEGIN_ARRAY[] = {'v', 'i', 'e', 'w'};
    char const MIDDLE_ARRAY[] = {'u', 'n', 'd', 'e', 'r'};
    char const END_ARRAY[] = {'t', 'e', 's', 't'};
    char const MISSING_ARRAY[] = {'I', 'N', 'V', 'A', 'L', 'I', 'D'};
    char const LARGE_ARRAY[] = {'v', 'i', 'e', 'w', '_', 'l', 'a', 'r', 'g', 'e', 'r', '_', 't', 'h', 'a', 'n', '_',
                                't', 'e', 's', 't', '_', 'v', 'i', 'e', 'w'};

    TEST_CASE ("Begins with", "[view]") {
        auto const TEST_VIEW = las::view(TEST_ARRAY);
        auto const BEGIN_VIEW = las::view(BEGIN_ARRAY);
        auto const MIDDLE_VIEW = las::view(MIDDLE_ARRAY);
        auto const MISSING_VIEW = las::view(MISSING_ARRAY);
        auto const LARGE_VIEW = las::view(LARGE_ARRAY);

        SECTION("With valid sequence") {
            auto result = begins_with(TEST_VIEW, BEGIN_VIEW);
            REQUIRE (result == true);
        }

        SECTION("With middle sub sequence") {
            auto result = begins_with(TEST_VIEW, MIDDLE_VIEW);
            REQUIRE (result == false);
        }

        SECTION("With non contained sequence") {
            auto result = begins_with(TEST_VIEW, MISSING_VIEW);
            REQUIRE (result == false);
        }

        SECTION("With sequence larger than view") {
            auto result = begins_with(TEST_VIEW, LARGE_VIEW);
            REQUIRE (result == false);
        }
    }

    TEST_CASE ("Ends with", "[view]") {
        auto const TEST_VIEW = las::view(TEST_ARRAY);
        auto const END_VIEW = las::view(END_ARRAY);
        auto const MIDDLE_VIEW = las::view(MIDDLE_ARRAY);
        auto const MISSING_VIEW = las::view(MISSING_ARRAY);
        auto const LARGE_VIEW = las::view(LARGE_ARRAY);

        SECTION("With valid view") {
            auto result = ends_with(TEST_VIEW, END_VIEW);
            REQUIRE (result == true);
        }

        SECTION("With middle sub sequence") {
            auto result = ends_with(TEST_VIEW, MIDDLE_VIEW);
            REQUIRE (result == false);
        }

        SECTION("With non contained sequence") {
            auto result = ends_with(TEST_VIEW, MISSING_VIEW);
            REQUIRE (result == false);
        }

        SECTION("With sequence larger than view") {
            auto result = ends_with(TEST_VIEW, LARGE_VIEW);
            REQUIRE (result == false);
        }
    }

    TEST_CASE ("contains", "[view]") {
        auto const TEST_VIEW = las::view(TEST_ARRAY);
        auto const BEGIN_VIEW = las::view(BEGIN_ARRAY);
        auto const MIDDLE_VIEW = las::view(MIDDLE_ARRAY);
        auto const END_VIEW = las::view(END_ARRAY);
        auto const MISSING_VIEW = las::view(MISSING_ARRAY);
        auto const LARGE_VIEW = las::view(LARGE_ARRAY);

        SECTION("Starting sequence") {
            auto result = contains(TEST_VIEW, BEGIN_VIEW);
            REQUIRE (result == true);
        }

        SECTION("Mid sequence") {
            auto result = contains(TEST_VIEW, MIDDLE_VIEW);
            REQUIRE (result == true);
        }

        SECTION("Ending sequence") {
            auto result = contains(TEST_VIEW, END_VIEW);
            REQUIRE (result == true);
        }

        SECTION("With sequence larger than view") {
            auto result = contains(TEST_VIEW, LARGE_VIEW);
            REQUIRE (result == false);
        }

        SECTION("With empty sequence") {
            auto result = contains(TEST_VIEW, decltype(TEST_VIEW)());
            REQUIRE (result == false);
        }

        SECTION("With not present sequence") {
            auto result = contains(TEST_VIEW, MISSING_VIEW);
            REQUIRE (result == false);
        }
    }
}