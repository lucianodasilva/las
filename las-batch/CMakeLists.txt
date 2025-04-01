if(BUILD_BATCH)

    find_package (cxxopts REQUIRED)

    add_executable(las-batch
            src/main.cpp
            src/options.cpp
            src/options.hpp
            src/report.cpp
            src/report.hpp
            src/runner_linux.cpp
            src/runner.hpp)

    target_link_libraries(las-batch PRIVATE las::las cxxopts::cxxopts)

endif()