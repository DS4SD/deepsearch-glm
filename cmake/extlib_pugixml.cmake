
message(STATUS "entering in extlib_pugixml.cmake")

set(ext_name "pugixml")

if(USE_SYSTEM_DEPS)
    find_package(PkgConfig)
    pkg_check_modules(libpugixml REQUIRED IMPORTED_TARGET pugixml)
    add_library(${ext_name} ALIAS PkgConfig::libpugixml)

else()
    include(ExternalProject)
    include(CMakeParseArguments)

    set(PUGIXML_URL https://github.com/zeux/pugixml.git)
    set(PUGIXML_TAG v1.15)

    ExternalProject_Add(extlib_pugixml

        PREFIX extlib_pugixml

        GIT_REPOSITORY ${PUGIXML_URL}
        GIT_TAG ${PUGIXML_TAG}

        UPDATE_COMMAND ""

        BUILD_ALWAYS OFF
        INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH}
            -DCMAKE_INSTALL_LIBDIR=lib
            -DCMAKE_CXX_FLAGS=${CMAKE_LIB_FLAGS}
            -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE

        BUILD_IN_SOURCE ON
        LOG_DOWNLOAD ON
        LOG_BUILD ON
        LOG_OUTPUT_ON_FAILURE ON
    )

    add_library(${ext_name} STATIC IMPORTED)
    add_dependencies(${ext_name} extlib_pugixml)
    set_target_properties(${ext_name} PROPERTIES
        IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}pugixml${CMAKE_STATIC_LIBRARY_SUFFIX}
        INTERFACE_INCLUDE_DIRECTORIES ${EXTERNALS_PREFIX_PATH}/include
    )
endif()
