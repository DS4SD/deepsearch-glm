cmake_minimum_required(VERSION 3.5)

message(STATUS "entering in extlib_fasttext.cmake")

include(ExternalProject)
include(CMakeParseArguments)

set(FASTTEXT_URL https://github.com/facebookresearch/fastText.git)
#set(FASTTEXT_TAG v0.9.2)

ExternalProject_Add(extlib_fasttext
    PREFIX extlib_fasttext

    GIT_REPOSITORY ${FASTTEXT_URL}
    #GIT_TAG ${FASTTEXT_TAG}

    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH}
    #CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} -DBUILD_SHARED_LIBS=OFF

    #BUILD_ALWAYS ON

    INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

    BUILD_IN_SOURCE ON
    LOG_DOWNLOAD ON
    LOG_BUILD ON)

add_library(fasttext STATIC IMPORTED)
set_target_properties(fasttext PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libfasttext.a)
add_dependencies(fasttext extlib_fasttext)
