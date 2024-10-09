
message(STATUS "entering in extlib_cxxopts.cmake")

set(ext_name "cxxopts")

if(USE_SYSTEM_DEPS)
    find_package(PkgConfig)
    pkg_check_modules(libcxxopts REQUIRED IMPORTED_TARGET cxxopts)
    add_library(${ext_name} ALIAS PkgConfig::libcxxopts)
    
else()
    include(ExternalProject)
    include(CMakeParseArguments)

    set(CXXOPTS_URL https://github.com/jarro2783/cxxopts.git)
    set(CXXOPTS_TAG v3.2.0)

    ExternalProject_Add(extlib_cxxopts

        PREFIX extlib_cxxopts

        UPDATE_COMMAND ""
        GIT_REPOSITORY ${CXXOPTS_URL}
        GIT_TAG ${CXXOPTS_TAG}

        BUILD_ALWAYS OFF

        INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

        CMAKE_ARGS \\
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \\
        -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH}

        BUILD_IN_SOURCE ON
        LOG_DOWNLOAD ON
    )

    add_library(${ext_name} INTERFACE)
    add_dependencies(${ext_name} extlib_cxxopts)
    set_target_properties(${ext_name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${EXTERNALS_PREFIX_PATH}/include
    )
endif()
