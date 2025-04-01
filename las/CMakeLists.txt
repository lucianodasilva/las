
#region project source files
set(las_include_files
        include/las/atomic_details.hpp
        include/las/las.h
        include/las/barrier.hpp
        include/las/bits.hpp
        include/las/config.hpp
        include/las/debug.hpp
        include/las/debug_print.hpp
        include/las/details.hpp
        include/las/dispatcher.hpp
        include/las/event.hpp
        include/las/flag.hpp
        include/las/ip_lock.hpp
        include/las/job.hpp
        include/las/locked_value.hpp
        include/las/ring_buffer.hpp
        include/las/scope_guards.hpp
        include/las/small_vector.hpp
        include/las/spin_mutex.hpp
        include/las/static_ring_buffer.hpp
        include/las/static_storage.hpp
        include/las/string.hpp
        include/las/system.hpp
        include/las/traits.hpp
        include/las/view.hpp)

set(las_source_files
        src/las.cpp
        src/barrier.cpp
        src/dispatcher.cpp
        src/event.cpp
        src/ip_lock.cpp
        src/job.cpp
        src/system.cpp)
#endregion

#region project build description
add_library(las
        ${las_include_files}
        ${las_source_files})

target_include_directories(
        las PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)

target_compile_features(
        las PUBLIC cxx_std_17)

add_library(las::las ALIAS las)
#endregion

#region project export
set_target_properties(las PROPERTIES
        VERSION ${LAS_VERSION})

target_include_directories(
        las PUBLIC
        $<INSTALL_INTERFACE:include/>)

install(TARGETS las EXPORT LAS_INSTALL_TARGETS)
install(DIRECTORY include/ DESTINATION include/)
#endregion

#region project tests
if (BUILD_TESTS)
    find_package(Catch2 CONFIG REQUIRED)

    add_executable(las-unit
            test/byte_swap.cpp
            test/dispatcher.cpp
            test/static_ring_buffer.cpp
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
#endregion