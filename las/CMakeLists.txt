
# project source files
set(las_include_files
        include/las/las.h
        include/las/bits.hpp
        include/las/debug.hpp
        include/las/details.hpp
        include/las/dispatcher.hpp
        include/las/event.hpp
        include/las/flag.hpp
        include/las/ip_lock.hpp
        include/las/job.h
        include/las/locked_value.hpp
        include/las/ring_buffer.hpp
        include/las/scope_guards.hpp
        include/las/small_vector.hpp
        include/las/string.hpp
        include/las/system.hpp
        include/las/traits.hpp
        include/las/view.hpp)

set(las_source_files
        src/las.cpp
        src/dispatcher.cpp
        src/event.cpp
        src/ip_lock.cpp
        src/job.cpp)

# project build description
add_library(las
        ${las_include_files}
        ${las_source_files})

target_include_directories(
        las PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)

target_compile_features(
        las PUBLIC cxx_std_17)

# project export
# read version from vcpkg.json
vcpkg_manifest_version(las_version)

set_target_properties(las PROPERTIES
        VERSION ${las_version}
        PUBLIC_HEADER "${las_include_files}")

target_include_directories(
        las PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

write_basic_package_version_file(
        lasConfigVersion.cmake
        VERSION ${las_version}
        COMPATIBILITY ExactVersion)

install(TARGETS las EXPORT las
        COMPONENT core)

install(EXPORT las
        NAMESPACE las::
        DESTINATION share/las
        FILE lasConfig.cmake)

# project tests
if (BUILD_TESTS)
    find_package(Catch2 CONFIG REQUIRED)

    add_executable(las-unit
            test/byte_swap.cpp
            test/dispatcher.cpp
            test/ring_buffer.cpp
            test/scope_guards.cpp
            test/small_vector.cpp
            test/string.tools.cpp
            test/view.tools.cpp
            test/view.cpp)

    target_link_libraries(
            las-unit PUBLIC
            las
            las-test
            Catch2::Catch2
            Catch2::Catch2WithMain)

    if (MSVC)
        target_link_options(
                las-unit PUBLIC
                "/ignore:4099")
    endif ()

endif ()