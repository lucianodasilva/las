#include <catch2/catch_all.hpp>
#include <las/string.hpp>


namespace las::string::test {


    std::vector<std::string> to_string_vector(std::vector<std::string_view> const &source) {
        return {source.begin(), source.end()};
    }

    inline std::string to_string(std::string_view view) {
        return std::string(view);
    }

    TEST_CASE("String Split Into", "[string]") {
        std::string const TEST_STRING{"string_under_test"};

        std::vector<std::string> const EXPECTANCY{
                "string", "under", "test"
        };

        const std::string DELIMITER{"_"};
        const std::string OTHER_DELIMITER{"."};

        std::vector<std::string_view> out_container;

        SECTION("Split Into") {
            split_into(TEST_STRING, DELIMITER, out_container);

            REQUIRE_THAT (to_string_vector(out_container), Catch::Matchers::Equals(EXPECTANCY));
        }

        SECTION("Split Into, missing String") {
            split_into("", DELIMITER, out_container);
            REQUIRE(out_container.empty());
        }

        SECTION("Split Into, wrong delimiter") {
            split_into(TEST_STRING, OTHER_DELIMITER, out_container);

            REQUIRE(out_container.size() == 1);
            REQUIRE(to_string (out_container.at(0)) == TEST_STRING);
        }
    }

    TEST_CASE("String Split", "[string]") {
        std::string const TEST_STRING{"string_under_test"};

        std::vector<std::string> const EXPECTANCY{
                "string", "under", "test"
        };

        const std::string DELIMITER{"_"};

        SECTION("Split") {
            auto out_container = split(TEST_STRING, DELIMITER);

            REQUIRE_THAT (to_string_vector(out_container), Catch::Matchers::Equals(EXPECTANCY));
        }
    }

    TEST_CASE("String Trim", "[string]") {
        const std::string EXPECTANCY{"string_under_test"};
        const std::string EXPECTANCY_LEFT{" string_under_test"};
        const std::string EXPECTANCY_RIGHT{"string_under_test "};

        const std::string TEST_STRING_LEFT{" string_under_test"};
        const std::string TEST_STRING_RIGHT{"string_under_test "};
        const std::string TEST_STRING_BOTH{" string_under_test "};

        SECTION("Left Trim") {
            auto result = trim_left(TEST_STRING_LEFT);
            REQUIRE(to_string (result) == EXPECTANCY);
        }

        SECTION("Left Trim, ignore right") {
            auto result = trim_left(TEST_STRING_BOTH);
            REQUIRE(to_string (result) == EXPECTANCY_RIGHT);
        }

        SECTION("Right Trim") {
            auto result = trim_right(TEST_STRING_RIGHT);
            REQUIRE(to_string (result) == EXPECTANCY);
        }

        SECTION("Right Trim, ignore left") {
            auto result = trim_right(TEST_STRING_BOTH);
            REQUIRE(to_string (result) == EXPECTANCY_LEFT);
        }

        SECTION("Trim") {
            auto result = trim(TEST_STRING_BOTH);
            REQUIRE(to_string(result) == EXPECTANCY);
        }
    }

    TEST_CASE("String Trim with new line", "[string]") {
        const std::string EXPECTANCY{"string_under_test"};
        const std::string EXPECTANCY_LEFT{"\nstring_under_test"};
        const std::string EXPECTANCY_RIGHT{"string_under_test\n"};

        const std::string TEST_STRING_LEFT{"\nstring_under_test"};
        const std::string TEST_STRING_RIGHT{"string_under_test\n"};
        const std::string TEST_STRING_BOTH{"\nstring_under_test\n"};

        SECTION("Left Trim") {
            auto result = trim_left(TEST_STRING_LEFT);
            REQUIRE(to_string (result) == EXPECTANCY);
        }

        SECTION("Left Trim, ignore right") {
            auto result = trim_left(TEST_STRING_BOTH);
            REQUIRE(to_string (result) == EXPECTANCY_RIGHT);
        }

        SECTION("Right Trim") {
            auto result = trim_right(TEST_STRING_RIGHT);
            REQUIRE(to_string (result) == EXPECTANCY);
        }

        SECTION("Right Trim, ignore left") {
            auto result = trim_right(TEST_STRING_BOTH);
            REQUIRE(to_string (result) == EXPECTANCY_LEFT);
        }

        SECTION("Trim") {
            auto result = trim(TEST_STRING_BOTH);
            REQUIRE(to_string (result) == EXPECTANCY);
        }
    }

    TEST_CASE("String Trim with empty strings ", "[string]") {
        const std::string TEST_STRING{};

        SECTION("Left Trim") {
            auto output = trim_left(TEST_STRING);
            REQUIRE(output.empty());
        }

        SECTION("Right Trim") {
            auto output = trim_right(TEST_STRING);
            REQUIRE(output.empty());
        }

        SECTION("Trim") {
            auto output = trim(TEST_STRING);
            REQUIRE(output.empty());
        }
    }

    TEST_CASE("String Trim with single visible character", "[string]") {
        const std::string TEST_STRING{"F"};
        const std::string EXPECTANCY{"F"};

        SECTION("Trim Left") {
            auto output = trim_left(TEST_STRING);
            REQUIRE (to_string(output) == EXPECTANCY);
        }

        SECTION("Trim Right") {
            auto output = trim_right(TEST_STRING);
            REQUIRE (to_string(output) == EXPECTANCY);
        }

        SECTION("Trim") {
            auto output = trim(TEST_STRING);
            REQUIRE (to_string(output) == EXPECTANCY);
        }
    }

    TEST_CASE("String Trim with single non graphic character", "[string]") {
        const std::string TEST_STRING{" "};
        const std::string EXPECTANCY; // empty

        SECTION("Trim Left") {
            auto output = trim_left(TEST_STRING);
            REQUIRE (to_string(output) == EXPECTANCY);
        }

        SECTION("Trim Right") {
            auto output = trim_right(TEST_STRING);
            REQUIRE (to_string(output) == EXPECTANCY);
        }

        SECTION("Trim") {
            auto output = trim(TEST_STRING);
            REQUIRE (to_string(output) == EXPECTANCY);
        }
    }

    TEST_CASE("String Trim with single new line character", "[string]") {
        const std::string TEST_STRING{"\n"};
        const std::string EXPECTANCY; // empty

        SECTION("Trim Left") {
            auto output = trim_left(TEST_STRING);
            REQUIRE (to_string(output) == EXPECTANCY);
        }

        SECTION("Trim Right") {
            auto output = trim_right(TEST_STRING);
            REQUIRE (to_string(output) == EXPECTANCY);
        }

        SECTION("Trim") {
            auto output = trim(TEST_STRING);
            REQUIRE (to_string(output) == EXPECTANCY);
        }
    }

    TEST_CASE("String Trim Token", "[string]") {
        const std::string EXPECTANCY{"string_under_test"};
        const std::string EXPECTANCY_LEFT{".string_under_test"};
        const std::string EXPECTANCY_RIGHT{"string_under_test."};

        const std::string TEST_STRING_LEFT{".string_under_test"};
        const std::string TEST_STRING_RIGHT{"string_under_test."};
        const std::string TEST_STRING_BOTH{".string_under_test."};

        const char TRIM_TOKEN{'.'};

        SECTION("Left Trim") {
            auto result = trim_left(TEST_STRING_LEFT, TRIM_TOKEN);
            REQUIRE(to_string (result) == EXPECTANCY);
        }

        SECTION("Left Trim, ignore right") {
            auto result = trim_left(TEST_STRING_BOTH, TRIM_TOKEN);
            REQUIRE(to_string (result) == EXPECTANCY_RIGHT);
        }

        SECTION("Right Trim") {
            auto result = trim_right(TEST_STRING_RIGHT, TRIM_TOKEN);
            REQUIRE(to_string (result) == EXPECTANCY);
        }

        SECTION("Right Trim, ignore left") {
            auto result = trim_right(TEST_STRING_BOTH, TRIM_TOKEN);
            REQUIRE(to_string (result) == EXPECTANCY_LEFT);
        }

        SECTION("Trim") {
            auto result = trim(TEST_STRING_BOTH, TRIM_TOKEN);
            REQUIRE(to_string (result) == EXPECTANCY);
        }
    }

    TEST_CASE("String Trim with single visible token", "[string]") {
        const std::string TEST_STRING{"."};
        const std::string EXPECTANCY; // empty

        const char TRIM_TOKEN{'.'};

        SECTION("Trim Left") {
            auto output = trim_left(TEST_STRING, TRIM_TOKEN);
            REQUIRE (to_string(output) == EXPECTANCY);
        }

        SECTION("Trim Right") {
            auto output = trim_right(TEST_STRING, TRIM_TOKEN);
            REQUIRE (to_string(output) == EXPECTANCY);
        }

        SECTION("Trim") {
            auto output = trim(TEST_STRING, TRIM_TOKEN);
            REQUIRE (to_string (output) == EXPECTANCY);
        }
    }

    TEMPLATE_TEST_CASE ("As Number", "[string]", int, float, double) {
        std::string const VALID_INTEGER_STRING{"123"};
        std::string const VALID_FLOAT_STRING{"456.7"};
        std::string const INVALID_STRING{"not-a-number"};

        TestType const EXPECTANCY_INTEGER{123};

        TestType const EXPECTANCY_FLOAT = []() -> TestType {
            if constexpr (std::is_same_v<int, TestType>) {
                return 456;
            } else {
                return 456.7;
            }
        }();

        SECTION("With valid integer") {
            auto result = string::as_number<TestType>(VALID_INTEGER_STRING);

            REQUIRE(result.has_value());
            REQUIRE(result.value_or(0) == EXPECTANCY_INTEGER);
        }

        SECTION("With valid float") {
            auto result = string::as_number<TestType>(VALID_FLOAT_STRING);

            REQUIRE(result.has_value());
            REQUIRE(result.value_or(0) == EXPECTANCY_FLOAT);
        }

        SECTION ("With invalid string") {
            auto result = string::as_number<TestType>(INVALID_STRING);

            REQUIRE_FALSE (result.has_value());
        }
    }

    TEST_CASE ("Cat", "[string]") {

        std::string const EXPECTANCY_VALID{"String Under Test"};

        SECTION("With valid strings") {
            std::string result = cat("String ", "Under ", "Test");
            REQUIRE (to_string (result) == EXPECTANCY_VALID);
        }

        SECTION("Different string types as input") {
            const char * const CHAR{" "};
            const std::string_view STRING_VIEW{"String"};
            const std::string STRING{"Under"};
            char const ARRAY[] = {'T', 'e', 's', 't', '\0'};

            std::string result = cat(STRING_VIEW, CHAR, STRING, CHAR, ARRAY);

            REQUIRE (to_string (result) == EXPECTANCY_VALID);
        }
    }

    TEST_CASE ("Begins with", "[string]") {
        std::string const TEST_STRING{"string_under_test"};
        std::string const BEGIN_TOKEN{"string"};
        std::string const MIDDLE_TOKEN{"under"};
        std::string const NO_TOKEN{"INVALID"};
        std::string const LARGE_TOKEN{"string_larger_then_test_string"};

        SECTION("With valid string") {
            auto result = begins_with(TEST_STRING, BEGIN_TOKEN);
            REQUIRE (result == true);
        }

        SECTION("With middle sub string") {
            auto result = begins_with(TEST_STRING, MIDDLE_TOKEN);
            REQUIRE (result == false);
        }

        SECTION("With non contained token") {
            auto result = begins_with(TEST_STRING, NO_TOKEN);
            REQUIRE (result == false);
        }

        SECTION("With token larger than string") {
            auto result = begins_with(TEST_STRING, LARGE_TOKEN);
            REQUIRE (result == false);
        }
    }

    TEST_CASE ("Ends with", "[string]") {
        std::string const TEST_STRING{"string_under_test"};
        std::string const BEGIN_TOKEN{"test"};
        std::string const MIDDLE_TOKEN{"under"};
        std::string const NO_TOKEN{"INVALID"};
        std::string const LARGE_TOKEN{"string_larger_then_test_string"};

        SECTION("With valid string") {
            auto result = ends_with(TEST_STRING, BEGIN_TOKEN);
            REQUIRE (result == true);
        }

        SECTION("With middle sub string") {
            auto result = string::ends_with(TEST_STRING, MIDDLE_TOKEN);
            REQUIRE (result == false);
        }

        SECTION("With non contained token") {
            auto result = ends_with(TEST_STRING, NO_TOKEN);
            REQUIRE (result == false);
        }

        SECTION("With token larger than string") {
            auto result = ends_with(TEST_STRING, LARGE_TOKEN);
            REQUIRE (result == false);
        }
    }

    TEST_CASE ("contains", "[string]") {
        std::string const TEST_STRING{"string_under_test"};
        std::string const BEGIN_TOKEN{"string"};
        std::string const MIDDLE_TOKEN{"under"};
        std::string const END_TOKEN{"test"};
        std::string const NO_TOKEN{"INVALID"};
        std::string const LARGE_TOKEN{"string_larger_then_test_string"};

        SECTION("Starting token") {
            auto result = contains(TEST_STRING, BEGIN_TOKEN);
            REQUIRE (result == true);
        }

        SECTION("Mid Token") {
            auto result = contains(TEST_STRING, MIDDLE_TOKEN);
            REQUIRE (result == true);
        }

        SECTION("Ending Token") {
            auto result = contains(TEST_STRING, END_TOKEN);
            REQUIRE (result == true);
        }

        SECTION("With token larger than string") {
            auto result = contains(TEST_STRING, LARGE_TOKEN);
            REQUIRE (result == false);
        }

        SECTION("With empty token") {
            auto result = contains(TEST_STRING, "");
            REQUIRE (result == false);
        }

        SECTION("With not present token") {
            auto result = contains(TEST_STRING, NO_TOKEN);
            REQUIRE (result == false);
        }
    }

    TEST_CASE ("next line", "[string]") {
        std::string const TEST_STRING{"string_under_test"};

        const std::string EXPECTANCY_1_1{"string"};
        const std::string EXPECTANCY_1_2{"under_test"};
        const std::string EXPECTANCY_2_1{"under"};
        const std::string EXPECTANCY_2_2{"test"};

        const std::string DELIMITER{"_"};
        const std::string OTHER_DELIMITER{"."};

        SECTION("Valid new line ") {

            std::string_view view_line = TEST_STRING;
            std::string_view line;

            bool const RESULT = next_line(view_line, line, DELIMITER);

            REQUIRE(RESULT);
            REQUIRE (to_string(line) == EXPECTANCY_1_1);
            REQUIRE (to_string (view_line) == EXPECTANCY_1_2);
        }

        SECTION("Multiple new line") {
            std::string_view test_line = TEST_STRING;
            std::string_view line;

            bool const RESULT = next_line(test_line, line, DELIMITER);

            REQUIRE (RESULT);
            REQUIRE (to_string(line) == EXPECTANCY_1_1);
            REQUIRE (to_string(test_line) == EXPECTANCY_1_2);

            std::string_view new_line;

            bool const RESULT2 = next_line(test_line, new_line, DELIMITER);

            REQUIRE (RESULT2);
            REQUIRE (to_string(new_line) == EXPECTANCY_2_1);
            REQUIRE (to_string(test_line) == EXPECTANCY_2_2);
        }

        SECTION("Other delimiter") {
            std::string_view test_line = TEST_STRING;
            std::string_view line;

            bool const RESULT = next_line(test_line, line, OTHER_DELIMITER);

            REQUIRE_FALSE (RESULT);
        }
    }
}