
# project source files
set (las_test_include_files
        include/las/test/las-test.h
        include/las/test/mock.h
        include/las/test/token.hpp)

set (las_test_source_files
        src/mock.cpp)

# project build description
add_library (las-test
        ${las_test_include_files}
        ${las_test_source_files})

target_link_libraries(
        las-test PUBLIC
        las)

target_include_directories(
        las-test PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)

target_compile_features (
        las-test PUBLIC cxx_std_17)

# project export
vcpkg_manifest_version(las_version)

set_target_properties(las-test PROPERTIES
        VERSION ${las_version}
        PUBLIC_HEADER "${las_test_include_files}")

target_include_directories(
        las-test PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

write_basic_package_version_file(
        lasTestConfigVersion.cmake
        VERSION ${las_version}
        COMPATIBILITY ExactVersion)

install(TARGETS las-test EXPORT las
        COMPONENT test)

install(EXPORT las
        NAMESPACE las::
        DESTINATION lib/cmake/las
        FILE lasConfig.cmake
        COMPONENT test)

if (BUILD_TESTS)
    find_package  (Catch2 CONFIG REQUIRED)

    add_executable (las-test-unit
            test/token.cpp)

    target_link_libraries(
            las-test-unit PUBLIC
            las-test
            Catch2::Catch2
            Catch2::Catch2WithMain)
endif()