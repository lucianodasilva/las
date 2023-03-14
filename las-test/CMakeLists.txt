set (las_test_include_files
        include/las-test.h
        include/las/test/mock.h
        include/las/test/token.hpp)

set (las_test_source_files
        src/mock.cpp)

add_library (las-test
        ${las_test_include_files}
        ${las_test_source_files})

target_link_libraries(
        las-test PUBLIC
        las)

target_include_directories(
        las-test PUBLIC
        include/)

target_compile_features (
        las-test PUBLIC cxx_std_17)

set_target_properties(las-test PROPERTIES
        PUBLIC_HEADER "${las_test_include_files}")

install(TARGETS las-test
        PUBLIC_HEADER DESTINATION include/las/test)

if (LAS_UNIT_TEST)

    find_package (Catch2 CONFIG REQUIRED)

    add_executable (las-test-unit
            test/token.cpp)

    target_link_libraries(
            las-test-unit PUBLIC
            las-test
            Catch2::Catch2
            Catch2::Catch2WithMain)
endif()