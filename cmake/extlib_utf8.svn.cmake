cmake_minimum_required(VERSION 3.5)

message(STATUS "entering in extlib_utf8.cmake")

include(ExternalProject)
include(CMakeParseArguments)

set(UTF8_URL svn://svn.code.sf.net/p/utfcpp/code/)

ExternalProject_Add(extlib_utf8
    PREFIX extlib_utf8

    SVN_REPOSITORY ${UTF8_URL}
    INSTALL_DIR ${EXTERNALS_PREFIX_PATH}

    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""

    BUILD_COMMAND ""
    BUILD_ALWAYS OFF

    INSTALL_COMMAND ""

    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/v2_0/source ${EXTERNALS_PREFIX_PATH}/include/utf8

    LOG_DOWNLOAD ON
    LOG_BUILD ON
    )

add_library(utf8 INTERFACE)
add_custom_target(install_extlib_utf8 DEPENDS extlib_utf8)
add_dependencies(utf8 install_extlib_utf8)