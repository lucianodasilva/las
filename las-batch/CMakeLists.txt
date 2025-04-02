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

    #region project export
    set_target_properties(las-batch PROPERTIES
            VERSION ${LAS_VERSION}
            EXPORT_NAME batch)

    install (
            TARGETS las-batch
            EXPORT LAS_INSTALL_TARGETS
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    #endregion

endif()