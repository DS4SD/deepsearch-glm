cmake_minimum_required(VERSION 3.5)

message(STATUS "entering in extlib_fmt.cmake")

include(ExternalProject)
include(CMakeParseArguments)

set(FMT_URL https://github.com/fmtlib/fmt.git)
set(FMT_TAG 10.2.1)

ExternalProject_Add(extlib_fmt
    PREFIX extlib_fmt

    GIT_REPOSITORY ${FMT_URL}
    GIT_TAG ${FMT_TAG}

    UPDATE_COMMAND ""
    CMAKE_ARGS \\
    -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} \\
    -DCMAKE_CXX_FLAGS=${ENV_ARCHFLAGS} \\
    -DFMT_LIB_DIR=lib \\
    -DFMT_TEST=OFF \\
    -DFMT_DOC=OFF \\
    -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE

    #-DCMAKE_CXX_FLAGS=-O3 \\
    #-DFMT_INSTALL=OFF 

    INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

    BUILD_IN_SOURCE ON
    LOG_DOWNLOAD ON
    LOG_BUILD ON)

add_library(fmt STATIC IMPORTED)
set_target_properties(fmt PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libfmt.a)
add_dependencies(fmt extlib_fmt)
