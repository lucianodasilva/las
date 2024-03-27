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

add_library (las
        ${las_include_files}
        ${las_source_files})

target_include_directories(
        las PUBLIC
        include/)

target_compile_features (
        las PUBLIC cxx_std_17)

set_target_properties(las PROPERTIES
        PUBLIC_HEADER "${las_include_files}")

install(TARGETS las
        PUBLIC_HEADER DESTINATION include)

if (BUILD_TESTS)
    find_package  (Catch2 CONFIG REQUIRED)

    add_executable (las-unit
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

    if(MSVC)
        target_link_options(
                las-unit PUBLIC 
                "/ignore:4099")
    endif()

endif()