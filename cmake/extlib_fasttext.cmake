cmake_minimum_required(VERSION 3.5)

message(STATUS "entering in extlib_fasttext.cmake")

include(ExternalProject)
include(CMakeParseArguments)

set(FASTTEXT_URL https://github.com/PeterStaar-IBM/fastText.git)
set(FASTTEXT_TAG 9d5b2a2b364f49ed2707ff3be48a0f1ba6d86022)

set(CXX_FLAGS "${ENV_ARCHFLAGS} -O3")

ExternalProject_Add(extlib_fasttext
    PREFIX extlib_fasttext

    GIT_REPOSITORY ${FASTTEXT_URL}
    GIT_TAG ${FASTTEXT_TAG}

    UPDATE_COMMAND ""

    CMAKE_ARGS \\
        -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} \\
        -DCMAKE_CXX_FLAGS=${CMAKE_LIB_FLAGS} \\
        -DCMAKE_INSTALL_LIBDIR=${EXTERNALS_PREFIX_PATH}/lib \\
        -DCMAKE_INSTALL_BINDIR=${EXTERNALS_PREFIX_PATH}/bin \\
        -DCMAKE_INSTALL_INCLUDEDIR=${EXTERNALS_PREFIX_PATH}/include

    INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

    BUILD_IN_SOURCE ON
    LOG_DOWNLOAD ON
    LOG_BUILD ON)

add_library(fasttext STATIC IMPORTED)
set_target_properties(fasttext PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libfasttext_pic.a)
add_dependencies(fasttext extlib_fasttext)
