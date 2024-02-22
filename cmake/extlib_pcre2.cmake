cmake_minimum_required(VERSION 3.5)

message(STATUS "entering in extlib_pcre2.cmake")

include(ExternalProject)
include(CMakeParseArguments)

set(PCRE2_URL https://github.com/PCRE2Project/pcre2.git)
#set(PCRE2_TAG pcre2-10.40)

ExternalProject_Add(extlib_pcre2
    PREFIX extlib_pcre2

    GIT_REPOSITORY ${PCRE2_URL}
    #GIT_TAG ${PCRE2_TAG}

    UPDATE_COMMAND ""
    
    CMAKE_ARGS \\
    -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} \\
    -DCMAKE_INSTALL_LIBDIR=${EXTERNALS_PREFIX_PATH}/lib \\
    -DCMAKE_CXX_FLAGS="-O3 ${ENV_ARCHFLAGS}" \\
    -DBUILD_SHARED_LIBS=OFF \\
    -DBUILD_STATIC_LIBS=ON \\
    -DPCRE2_STATIC_PIC=ON \\
    -DPCRE2_SHOW_REPORT=OFF

    BUILD_ALWAYS OFF

    INSTALL_DIR ${EXTERNALS_PREFIX_PATH}
    #LIBRARY_DIR ${EXTERNALS_PREFIX_PATH}/lib

    BUILD_IN_SOURCE ON
    LOG_DOWNLOAD ON
    LOG_BUILD ON)

add_library(pcre2 STATIC IMPORTED)
set_target_properties(pcre2 PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libpcre2-8.a)
#set_target_properties(pcre2 PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib64/libpcre2-8.a)
add_dependencies(pcre2 extlib_pcre2)
