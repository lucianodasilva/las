
#region project source files
set(las_test_include_files
        include/las/test/test.h
        include/las/test/concurrent_stress_tester.hpp
        include/las/test/mock.hpp
        include/las/test/random.hpp
        include/las/test/token.hpp)

set(las_test_source_files
        src/concurrent_stress_tester.cpp
        src/mock.cpp
        src/random.cpp)
#endregion

#region project build description
add_library(las-test
        ${las_test_include_files}
        ${las_test_source_files})

target_link_libraries(
        las-test PUBLIC
        las
        $<$<PLATFORM_ID:Windows>:Synchronization>)

target_include_directories(
        las-test PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)

target_compile_features(
        las-test PUBLIC cxx_std_17)

add_library(las::test ALIAS las-test)
#endregion

#region project export
set_target_properties(las-test PROPERTIES
        VERSION ${LAS_VERSION}
        EXPORT_NAME test)

target_include_directories(
        las-test PUBLIC
        $<INSTALL_INTERFACE:include/>)

install(TARGETS las-test EXPORT LAS_INSTALL_TARGETS)
install(DIRECTORY include/ DESTINATION include/)
#endregion

#region project tests
if (BUILD_TESTS)
    find_package(Catch2 CONFIG REQUIRED)

    add_executable(las-test-unit
            test/token.cpp)

    target_link_libraries(
            las-test-unit PUBLIC
            las-test
            Catch2::Catch2
            Catch2::Catch2WithMain)
endif ()
#endregion