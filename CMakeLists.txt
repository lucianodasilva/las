cmake_minimum_required(VERSION 3.24)

project (las CXX)

option(LAS_TEST_TOOLS "Compile testing tools library"   ON)
option(LAS_UNIT_TEST "Compile unit tests"               ON)

if (LAS_UNIT_TEST)
    set (LAS_TEST_TOOLS ON)
endif()

add_subdirectory(las)

if (LAS_TEST_TOOLS)
    add_subdirectory(las-test)
endif()