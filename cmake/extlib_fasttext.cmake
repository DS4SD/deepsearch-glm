
message(STATUS "entering in extlib_fasttext.cmake")

set(ext_name "fasttext")

if(USE_SYSTEM_DEPS)
    find_package(PkgConfig)
    pkg_check_modules(libfasttext_pic REQUIRED IMPORTED_TARGET fasttext)
    add_library(${ext_name} ALIAS PkgConfig::libfasttext_pic)
    
else()
    include(ExternalProject)
    include(CMakeParseArguments)

    set(FASTTEXT_URL https://github.com/PeterStaar-IBM/fastText.git)
    set(FASTTEXT_TAG 9d5b2a2b364f49ed2707ff3be48a0f1ba6d86022)

    ExternalProject_Add(extlib_fasttext

        PREFIX extlib_fasttext

        GIT_REPOSITORY ${FASTTEXT_URL}
        GIT_TAG ${FASTTEXT_TAG}

        UPDATE_COMMAND ""

        BUILD_ALWAYS OFF
        INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

        CMAKE_ARGS \\
            -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} \\
            -DCMAKE_CXX_FLAGS=${CMAKE_LIB_FLAGS} \\
            -DCMAKE_INSTALL_LIBDIR=${EXTERNALS_PREFIX_PATH}/lib \\
            -DCMAKE_INSTALL_BINDIR=${EXTERNALS_PREFIX_PATH}/bin \\
            -DCMAKE_INSTALL_INCLUDEDIR=${EXTERNALS_PREFIX_PATH}/include

        BUILD_IN_SOURCE ON
        LOG_DOWNLOAD ON
        LOG_BUILD ON
    )

    add_library(${ext_name} STATIC IMPORTED)
    add_dependencies(${ext_name} extlib_fasttext)
    set_target_properties(${ext_name} PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libfasttext_pic.a INTERFACE_INCLUDE_DIRECTORIES ${EXTERNALS_PREFIX_PATH}/include
    )
endif()
