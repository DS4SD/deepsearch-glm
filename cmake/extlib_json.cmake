
message(STATUS "entering in extlib_json.cmake")

set(ext_name "json")

if(USE_SYSTEM_DEPS)
    find_package(PkgConfig)
    pkg_check_modules(libjson REQUIRED IMPORTED_TARGET nlohmann_json)
    add_library(${ext_name} ALIAS PkgConfig::libjson)
    
else()
    include(ExternalProject)
    include(CMakeParseArguments)

    set(JSON_URL https://github.com/nlohmann/json.git)
    set(JSON_TAG v3.11.3)

    ExternalProject_Add(extlib_json

        PREFIX extlib_json

        GIT_REPOSITORY ${JSON_URL}
        GIT_TAG ${JSON_TAG}

        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        BUILD_ALWAYS OFF

        INSTALL_DIR ${EXTERNALS_PREFIX_PATH}
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include/ ${EXTERNALS_PREFIX_PATH}/include/

        LOG_DOWNLOAD ON
        LOG_BUILD ON
    )

    add_library(${ext_name} INTERFACE)
    add_dependencies(${ext_name} extlib_json)
    set_target_properties(${ext_name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${EXTERNALS_PREFIX_PATH}/include
    )
endif()
