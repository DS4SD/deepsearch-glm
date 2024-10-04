
message(STATUS "entering in extlib_loguru.cmake")

set(ext_name "loguru")

if(USE_SYSTEM_DEPS)
    find_package(PkgConfig)
    pkg_check_modules(libloguru REQUIRED IMPORTED_TARGET loguru)

    add_library(${ext_name} ALIAS PkgConfig::libloguru)
else()
    include(ExternalProject)
    include(CMakeParseArguments)

    set(LOGURU_URL https://github.com/emilk/loguru)
    set(LOGURU_TAG v2.1.0)

    set(LOGURU_INCLUDE_DIR ${EXTERNALS_PREFIX_PATH}/include/loguru)

    # Ensure the directory for headers exists
    execute_process(COMMAND mkdir -p ${LOGURU_INCLUDE_DIR})

    ExternalProject_Add(extlib_loguru

        PREFIX extlib_loguru

        GIT_REPOSITORY ${LOGURU_URL}
        GIT_TAG ${LOGURU_TAG}

        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        BUILD_ALWAYS OFF

        INSTALL_DIR ${EXTERNALS_PREFIX_PATH}
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/loguru.hpp ${LOGURU_INCLUDE_DIR} &&
                        ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/loguru.cpp ${LOGURU_INCLUDE_DIR}

        LOG_DOWNLOAD ON
        LOG_BUILD ON
    )

    add_library(${ext_name} INTERFACE)
    add_dependencies(${ext_name} extlib_loguru)
    set_target_properties(${ext_name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LOGURU_INCLUDE_DIR}
    )
endif()
