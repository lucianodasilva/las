#include <catch2/catch_all.hpp>
#include <las/bits.hpp>

namespace las::test {

    TEMPLATE_TEST_CASE_SIG(
            "unsigned integers", "[byte_swap]",
            ((typename type_t, type_t TEST_VALUE, type_t TEST_EXPECTATION), type_t, TEST_VALUE, TEST_EXPECTATION),
                           (uint8_t, 123, 123),
                           (uint16_t, 0xF77F, 0x7FF7),
                           (uint32_t, 0x15F77F51, 0x517FF715),
                           (uint64_t, 0x216415F77F514612, 0x1246517FF7156421))
    {
        REQUIRE(las::byte_swap(TEST_VALUE) == TEST_EXPECTATION);
    }

    TEMPLATE_TEST_CASE_SIG(
            "positive signed integers", "[byte_swap]",
            ((typename type_t, type_t TEST_VALUE, type_t TEST_EXPECTATION), type_t, TEST_VALUE, TEST_EXPECTATION),
            (int8_t, 123, 123),
            (int16_t, 27445, 13675),
            (int32_t, 1498941268, 1409767513),
            (int64_t, 2764555521326020405, 3852683738373446950))
    {
        REQUIRE(las::byte_swap(TEST_VALUE) == TEST_EXPECTATION);
    }

    TEMPLATE_TEST_CASE_SIG(
            "negative signed integers", "[byte_swap]",
            ((typename type_t, type_t TEST_VALUE, type_t TEST_EXPECTATION), type_t, TEST_VALUE, TEST_EXPECTATION),
            (int8_t, -123, -123),
            (int16_t, -27445, -13420),
            (int32_t, -1498941268, -1392990298),
            (int64_t, -2764555521326020405, -3780626144335519015))
    {
        REQUIRE(las::byte_swap(TEST_VALUE) == TEST_EXPECTATION);
    }
}