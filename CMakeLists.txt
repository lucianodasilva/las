cmake_minimum_required(VERSION 3.24)

include(cmake/tools.cmake)
include(CMakePackageConfigHelpers)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "test")
endif ()

project(las CXX)

add_subdirectory(las)
add_subdirectory(las-test)