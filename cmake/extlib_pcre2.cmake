
message(STATUS "entering in extlib_pcre2.cmake")

set(ext_name "pcre2")

if(USE_SYSTEM_DEPS)
    find_package(PkgConfig)
    pkg_check_modules(libpcre2-8 REQUIRED IMPORTED_TARGET libpcre2-8)
    add_library(${ext_name} ALIAS PkgConfig::libpcre2-8)
    
else()
    include(ExternalProject)
    include(CMakeParseArguments)

    set(PCRE2_URL https://github.com/PCRE2Project/pcre2.git)
    set(PCRE2_TAG pcre2-10.44)

    ExternalProject_Add(extlib_pcre2

        PREFIX extlib_pcre2

        GIT_REPOSITORY ${PCRE2_URL}
        GIT_TAG ${PCRE2_TAG}

        UPDATE_COMMAND ""

        CMAKE_ARGS \\
            -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} \\
            -DCMAKE_INSTALL_LIBDIR=${EXTERNALS_PREFIX_PATH}/lib \\
            -DCMAKE_OSX_ARCHITECTURES=${ENV_ARCH} \\
            -DBUILD_SHARED_LIBS=OFF \\
            -DBUILD_STATIC_LIBS=ON \\
            -DPCRE2_STATIC_PIC=ON \\
            -DPCRE2_SHOW_REPORT=OFF

        BUILD_ALWAYS OFF

        INSTALL_DIR ${EXTERNALS_PREFIX_PATH}
        BUILD_IN_SOURCE ON
        LOG_DOWNLOAD ON
        LOG_BUILD ON
        LOG_OUTPUT_ON_FAILURE ON
    )

    add_library(${ext_name} STATIC IMPORTED)
    add_dependencies(${ext_name} extlib_pcre2)
    # The static library basename is not just a prefix/suffix swap: pcre2's own
    # CMakeLists appends "-static" under MSVC so the static lib does not collide
    # with the DLL import library. Observed on the runner: pcre2-8-static.lib
    # vs libpcre2-8.a everywhere else.
    if(MSVC)
        set(PCRE2_STATIC_NAME pcre2-8-static)
    else()
        set(PCRE2_STATIC_NAME pcre2-8)
    endif()

    set_target_properties(${ext_name} PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${PCRE2_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX} INTERFACE_INCLUDE_DIRECTORIES ${EXTERNALS_PREFIX_PATH}/include
    )
endif()
