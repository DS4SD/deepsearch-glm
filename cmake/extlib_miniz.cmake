
message(STATUS "entering in extlib_miniz.cmake")

set(ext_name "miniz")

if(USE_SYSTEM_DEPS)
    find_package(PkgConfig)
    pkg_check_modules(libminiz REQUIRED IMPORTED_TARGET miniz)
    add_library(${ext_name} ALIAS PkgConfig::libminiz)

else()
    include(ExternalProject)
    include(CMakeParseArguments)

    set(MINIZ_URL https://github.com/richgel999/miniz.git)
    set(MINIZ_TAG 3.0.2)

    ExternalProject_Add(extlib_miniz

        PREFIX extlib_miniz

        GIT_REPOSITORY ${MINIZ_URL}
        GIT_TAG ${MINIZ_TAG}

        UPDATE_COMMAND ""

        BUILD_ALWAYS OFF
        INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH}
            -DCMAKE_INSTALL_LIBDIR=lib
            -DCMAKE_C_FLAGS=${CMAKE_LIB_FLAGS}
            -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE

        BUILD_IN_SOURCE ON
        LOG_DOWNLOAD ON
        LOG_BUILD ON
        LOG_OUTPUT_ON_FAILURE ON
    )

    add_library(${ext_name} STATIC IMPORTED)
    add_dependencies(${ext_name} extlib_miniz)
    set_target_properties(${ext_name} PROPERTIES
        IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}miniz${CMAKE_STATIC_LIBRARY_SUFFIX}
        INTERFACE_INCLUDE_DIRECTORIES ${EXTERNALS_PREFIX_PATH}/include
    )
endif()
