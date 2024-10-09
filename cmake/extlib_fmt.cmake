
message(STATUS "entering in extlib_fmt.cmake")

set(ext_name "fmt")

if(USE_SYSTEM_DEPS)
    find_package(PkgConfig)
    pkg_check_modules(libfmt REQUIRED IMPORTED_TARGET fmt)
    add_library(${ext_name} ALIAS PkgConfig::libfmt)
    
else()
    include(ExternalProject)
    include(CMakeParseArguments)

    set(FMT_URL https://github.com/fmtlib/fmt.git)
    set(FMT_TAG 10.2.1)

    message(STATUS "extlib_fmt cxx-flags: " ${CMAKE_CXX_FLAGS})

    ExternalProject_Add(extlib_fmt

        PREFIX extlib_fmt

        GIT_REPOSITORY ${FMT_URL}
        GIT_TAG ${FMT_TAG}

        UPDATE_COMMAND ""

        BUILD_ALWAYS OFF
        INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

        CMAKE_ARGS \\
            -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} \\
            -DCMAKE_CXX_FLAGS=${CMAKE_LIB_FLAGS} \\
            -DFMT_LIB_DIR=lib \\
            -DFMT_TEST=OFF \\
            -DFMT_DOC=OFF \\
            -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE

        BUILD_IN_SOURCE ON
        LOG_DOWNLOAD ON
        LOG_BUILD ON
    )

    add_library(${ext_name} STATIC IMPORTED)
    add_dependencies(${ext_name} extlib_fmt)
    set_target_properties(${ext_name} PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libfmt.a INTERFACE_INCLUDE_DIRECTORIES ${EXTERNALS_PREFIX_PATH}/include
    )
endif()
