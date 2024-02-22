cmake_minimum_required(VERSION 3.5)

message(STATUS "entering in extlib_sentencepiece.cmake")

include(ExternalProject)
include(CMakeParseArguments)

set(SENTENCEPIECE_URL https://github.com/google/sentencepiece.git)
set(SENTENCEPIECE_TAG v0.1.99)

message(STATUS "arch flags in spm: " ${ENV_ARCHFLAGS})

ExternalProject_Add(extlib_sentencepiece
    PREFIX extlib_sentencepiece

    GIT_REPOSITORY ${SENTENCEPIECE_URL}
    GIT_TAG ${SENTENCEPIECE_TAG}

    UPDATE_COMMAND ""
    CMAKE_ARGS \\
    -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} \\
    -DCMAKE_INSTALL_LIBDIR=lib \\
    -DCMAKE_CXX_FLAGS=${ENV_ARCHFLAGS} \\
    -DSPM_BUILD_TEST=OFF \\
    -DSPM_COVERAGE=OFF \\
    -DSPM_ENABLE_NFKC_COMPILE=OFF \\
    -DSPM_ENABLE_SHARED=OFF \\	   
    -DSPM_ENABLE_TCMALLOC=OFF \\
    -DSPM_ENABLE_TENSORFLOW_SHARED=OFF \\
    -DSPM_NO_THREADLOCAL=OFF \\
    -DSPM_TCMALLOC_STATIC=OFF \\
    -DSPM_USE_BUILTIN_PROTOBUF=ON \\
    -DSPM_USE_EXTERNAL_ABSL=OFF \\
    -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE

    INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

    BUILD_IN_SOURCE ON
    LOG_DOWNLOAD ON
    LOG_BUILD ON)

add_library(sentencepiece STATIC IMPORTED)
set_target_properties(sentencepiece PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libsentencepiece.a)
add_dependencies(sentencepiece extlib_sentencepiece)

add_library(sentencepiece_train STATIC IMPORTED)
set_target_properties(sentencepiece_train PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libsentencepiece_train.a)
add_dependencies(sentencepiece_train extlib_sentencepiece)
