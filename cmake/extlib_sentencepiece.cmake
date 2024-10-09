
message(STATUS "entering in extlib_sentencepiece.cmake")

set(ext_name "sentencepiece")

if(USE_SYSTEM_DEPS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(sentencepiece REQUIRED IMPORTED_TARGET sentencepiece)
    add_library(${ext_name} ALIAS PkgConfig::sentencepiece)

else()
    include(CMakeParseArguments)

    set(SENTENCEPIECE_URL https://github.com/google/sentencepiece.git)
    set(SENTENCEPIECE_TAG v0.2.0)

    message(STATUS "extlib_sentencepiece cxx-flags: " ${CMAKE_CXX_FLAGS})

    ExternalProject_Add(extlib_sentencepiece
        PREFIX extlib_sentencepiece

        GIT_REPOSITORY ${SENTENCEPIECE_URL}
        GIT_TAG ${SENTENCEPIECE_TAG}

        UPDATE_COMMAND ""
        CMAKE_ARGS \\
            -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} \\
            -DCMAKE_INSTALL_LIBDIR=lib \\
            -DCMAKE_CXX_FLAGS=${CMAKE_LIB_FLAGS} \\
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
        LOG_BUILD ON
    )

    add_library(${ext_name} STATIC IMPORTED)
    set_target_properties(${ext_name} PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libsentencepiece.a)
    add_dependencies(${ext_name} extlib_sentencepiece)

    add_library(${ext_name}_train STATIC IMPORTED)
    set_target_properties(${ext_name}_train PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libsentencepiece_train.a)
    add_dependencies(${ext_name}_train extlib_sentencepiece)
endif()
