cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0054 NEW)

include(cmake/tools.cmake)
include(CMakePackageConfigHelpers)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "test")
endif ()

vcpkg_manifest_version(LAS_VERSION)

project(las CXX)

add_subdirectory(las)
add_subdirectory(las-test)

#region export targets
write_basic_package_version_file(
        lasConfigVersion.cmake
        VERSION ${LAS_VERSION}
        COMPATIBILITY ExactVersion)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/lasConfigVersion.cmake
        DESTINATION share/las)

install(EXPORT LAS_INSTALL_TARGETS
        NAMESPACE las::
        DESTINATION share/las
        FILE lasConfig.cmake)