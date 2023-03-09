cmake_minimum_required(VERSION 3.24)

project (las CXX)

option(LAS_BUILD_TEST "Compile unit tests" ON)

set(las_include_files
        include/las.h
        include/las/bits.hpp
        include/las/debug.hpp
        include/las/details.hpp
        include/las/dispatcher.hpp
        include/las/event.hpp
        include/las/flag.hpp
        include/las/ip_lock.hpp
        include/las/locked_value.hpp
        include/las/ring_buffer.hpp
        include/las/scope_guards.hpp
        include/las/string.hpp
        include/las/system.hpp
        include/las/traits.hpp)

set(las_source_files
        src/las.cpp
        src/dispatcher.cpp
        src/event.cpp
        src/ip_lock.cpp)

add_library (las
        ${las_include_files}
        ${las_source_files})

target_include_directories(
        las PUBLIC
        include/)

target_compile_features (
        las PUBLIC cxx_std_17)

if (LAS_BUILD_TEST)

    find_package (Catch2 CONFIG REQUIRED)

    add_executable (las-unit
            test/ring_buffer.cpp
            test/string.cpp)

    target_link_libraries(
            las-unit PUBLIC
            las
            Catch2::Catch2
            Catch2::Catch2WithMain)
endif()