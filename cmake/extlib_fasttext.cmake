
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
    # The old pin (9d5b2a2b, 2023-10-17) predates two fixes that already exist
    # on this fork's master:
    #   f8b084a9  "#include <cstdint>" in args.cc -- gcc 13+ no longer pulls it
    #             in transitively, so args.cc fails on the manylinux_2_28 image
    #             (gcc 14). This is what broke every py3.11+ linux wheel, while
    #             py3.10 kept working because cibuildwheel <3 uses manylinux2014
    #             (gcc 10).
    #   9e4f8199  install rules now honour CMAKE_INSTALL_* (see CMAKE_ARGS note)
    #   6879700e  do not force "-pthread -std=c++11" under MSVC, which cl.exe
    #             rejects -- required for the win_arm64 build
    #   040db187  do not link pthread under MSVC either; there is no pthread.lib,
    #             and fasttext-bin failing to link (LNK1181) fails the whole
    #             build even though every library target links fine
    set(FASTTEXT_TAG 040db1874f833cf5ad4857411fdccf9932b18e99)

    # Force-include <cstdint> as belt-and-braces (see CMAKE_ARGS below). The
    # spelling is compiler specific: cl.exe has no -include.
    if(MSVC)
        set(FASTTEXT_EXTRA_CXX_FLAGS "/FIcstdint")
    else()
        set(FASTTEXT_EXTRA_CXX_FLAGS "-include cstdint")
    endif()

    ExternalProject_Add(extlib_fasttext

        PREFIX extlib_fasttext

        GIT_REPOSITORY ${FASTTEXT_URL}
        GIT_TAG ${FASTTEXT_TAG}

        UPDATE_COMMAND ""

        BUILD_ALWAYS OFF
        INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

        # NOTE: deliberately no `\\` line continuations here. CMake expands `\\`
        # to a literal backslash which then escapes the list separator, so every
        # argument came out as ";-DCMAKE_INSTALL_LIBDIR=..." and was silently
        # ignored. That was harmless while fastText hardcoded `DESTINATION lib`,
        # but master includes GNUInstallDirs and installs to CMAKE_INSTALL_LIBDIR
        # -- which defaults to lib64 on the RHEL-based manylinux image, i.e. not
        # where IMPORTED_LOCATION below looks for libfasttext_pic.a.
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH}
            # belt-and-braces: the args.cc fix above covers the one translation
            # unit that is known to break, this covers any other header that
            # relied on a transitive <cstdint>
            "-DCMAKE_CXX_FLAGS=${CMAKE_LIB_FLAGS} ${FASTTEXT_EXTRA_CXX_FLAGS}"
            -DCMAKE_INSTALL_LIBDIR=${EXTERNALS_PREFIX_PATH}/lib
            -DCMAKE_INSTALL_BINDIR=${EXTERNALS_PREFIX_PATH}/bin
            -DCMAKE_INSTALL_INCLUDEDIR=${EXTERNALS_PREFIX_PATH}/include

        BUILD_IN_SOURCE ON
        LOG_DOWNLOAD ON
        # LOG_BUILD is deliberately off: with it on, a failure is replayed
        # through LOG_OUTPUT_ON_FAILURE truncated ("...skipping to end..."),
        # which hid why the MSVC/ARM64 build returns 1 even though
        # fasttext_pic.lib links fine. Stream it instead.
        LOG_OUTPUT_ON_FAILURE ON
    )

    add_library(${ext_name} STATIC IMPORTED)
    add_dependencies(${ext_name} extlib_fasttext)
    set_target_properties(${ext_name} PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libfasttext_pic.a INTERFACE_INCLUDE_DIRECTORIES ${EXTERNALS_PREFIX_PATH}/include
    )
endif()
