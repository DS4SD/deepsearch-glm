cmake_minimum_required(VERSION 3.12..3.26)

project(andromeda VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_VERBOSE_MAKEFILE off)

message(STATUS "cmake osx-deployment: " ${CMAKE_OSX_DEPLOYMENT_TARGET})

# minimum macosx, ignored on other platforms
if(APPLE)
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        message(STATUS "cmake is not set")            
    elseif(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS 10.15)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version" FORCE)
    endif()
endif()

#set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)
message(STATUS "cmake system-version: " ${CMAKE_SYSTEM_VERSION})
message(STATUS "cmake osx-deployment: " ${CMAKE_OSX_DEPLOYMENT_TARGET})

# ************************
# ***     paths        ***
# ************************

if(NOT DEFINED TOPLEVEL_PREFIX_PATH)
    set(TOPLEVEL_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${TOPLEVEL_PREFIX_PATH}/install_dir)
endif()

if(NOT DEFINED EXTERNALS_PREFIX_PATH)
    set(EXTERNALS_PREFIX_PATH "${TOPLEVEL_PREFIX_PATH}/externals" CACHE INTERNAL "")
endif()

if(NOT DEFINED RESOURCES_PREFIX_PATH)
    set(RESOURCES_PREFIX_PATH "${TOPLEVEL_PREFIX_PATH}/resources" CACHE INTERNAL "")
endif()

if(NOT "${TOPLEVEL_PREFIX_PATH}/cmake" IN_LIST CMAKE_MODULE_PATH)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${TOPLEVEL_PREFIX_PATH}/cmake")
endif()

set(ENV_ARCH $ENV{CIBW_ARCHS})
message(STATUS "cibw arch: " ${ENV_ARCH})

set(CMAKE_OSX_ARCHITECTURES $ENV{CIBW_ARCHS})
#message(STATUS "cibw arch: " ${ENV_ARCH})

set(ENV_ARCHFLAGS $ENV{ARCHFLAGS})
message(STATUS "arch flags: " ${ENV_ARCHFLAGS})

message(STATUS "      top path: " ${TOPLEVEL_PREFIX_PATH})
message(STATUS "      lib path: " ${EXTERNALS_PREFIX_PATH})
message(STATUS "  install path: " ${CMAKE_INSTALL_PREFIX})
message(STATUS "    cmake path: " ${CMAKE_MODULE_PATH})
message(STATUS "  cmake system: " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "cmake osx arch: " ${CMAKE_OSX_ARCHITECTURES})

# ************************
# ***  cxx-flags       ***
# ************************

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -Ofast -DROOT_PATH='\"${TOPLEVEL_PREFIX_PATH}\"' ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -O3 -DROOT_PATH='\"${TOPLEVEL_PREFIX_PATH}\"' ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -O2 -DROOT_PATH='\"${TOPLEVEL_PREFIX_PATH}\"' ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -O1 -DROOT_PATH='\"${TOPLEVEL_PREFIX_PATH}\"' ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -O0 -DROOT_PATH='\"${TOPLEVEL_PREFIX_PATH}\"' ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -g3 -DROOT_PATH='\"${TOPLEVEL_PREFIX_PATH}\"' -arch arm64")

#set(CMAKE_LIB_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -O3 -DROOT_PATH='\"${TOPLEVEL_PREFIX_PATH}\"' ${ENV_ARCHFLAGS}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -O3 -DROOT_PATH='\"${TOPLEVEL_PREFIX_PATH}\"' ${ENV_ARCHFLAGS}")

if (WIN32)
    add_definitions(-DPCRE2_STATIC)
    add_definitions(-DNOMINMAX)
    add_definitions(-DROOT_PATH="\\\"${TOPLEVEL_PREFIX_PATH}\\\"")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -O3 ${ENV_ARCHFLAGS}")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -O3 -DROOT_PATH='\"${TOPLEVEL_PREFIX_PATH}\"' ${ENV_ARCHFLAGS}")
endif()

message(STATUS "cxx-compiler: " ${CMAKE_CXX_COMPILER})
message(STATUS "cxx-flags   : " ${CMAKE_CXX_FLAGS})
message(STATUS "cxx-standard: " ${CMAKE_CXX_STANDARD})

# ************************
# *** dependencies     ***
# ************************

option(USE_SYSTEM_DEPS OFF "If enabled, the build will find and link to system dependencies, otherwise they are sourced from the original repos and compiled on the fly.")

if(USE_SYSTEM_DEPS)
    set(DEPENDENCIES fmt json loguru cxxopts pcre2 fasttext utf8 sentencepiece)
else()
    if(NOT EXISTS ${EXTERNALS_PREFIX_PATH})
        file(MAKE_DIRECTORY ${EXTERNALS_PREFIX_PATH})
        file(MAKE_DIRECTORY ${EXTERNALS_PREFIX_PATH}/bin)
        file(MAKE_DIRECTORY ${EXTERNALS_PREFIX_PATH}/lib)
        file(MAKE_DIRECTORY ${EXTERNALS_PREFIX_PATH}/include)
        file(MAKE_DIRECTORY ${EXTERNALS_PREFIX_PATH}/resources)
    endif()
    set(DEPENDENCIES fmt json loguru cxxopts pcre2 fasttext utf8 sentencepiece sentencepiece_train)
endif()

# include dependencies
include(cmake/extlib_fmt.cmake)
include(cmake/extlib_json.cmake)
include(cmake/extlib_loguru.cmake)
include(cmake/extlib_cxxopts.cmake)
include(cmake/extlib_pcre2.cmake)
include(cmake/extlib_fasttext.cmake)
include(cmake/extlib_utf8.git.cmake)
include(cmake/extlib_sentencepiece.cmake)

# ************************
# *** libraries        ***
# ************************

# define LIB_LINK and OS_DEPENDENCIES
include(cmake/os_opts.cmake)
list(APPEND DEPENDENCIES ${OS_DEPENDENCIES})

# ***********************
# ***  includes       ***
# ***********************

# define subdirlist utility
include(cmake/subdirlist.cmake)
include_directories(${TOPLEVEL_PREFIX_PATH}/src)

subdirlist(SUBDIRS ${TOPLEVEL_PREFIX_PATH}/src)
message(STATUS "subdirs: " ${SUBDIRS})
include_directories(${SUBDIRS})

# **********************
# ***  Executables   ***
# **********************

add_executable(nlp.exe "${TOPLEVEL_PREFIX_PATH}/app/nlp.cpp")
add_executable(glm.exe "${TOPLEVEL_PREFIX_PATH}/app/glm.cpp")

set_property(TARGET nlp.exe PROPERTY CXX_STANDARD 20)
set_property(TARGET glm.exe PROPERTY CXX_STANDARD 20)

add_dependencies(nlp.exe ${DEPENDENCIES})
target_include_directories(nlp.exe INTERFACE ${DEPENDENCIES})
target_link_libraries(nlp.exe ${DEPENDENCIES} ${LIB_LINK})

add_dependencies(glm.exe ${DEPENDENCIES})
target_include_directories(glm.exe INTERFACE ${DEPENDENCIES})
target_link_libraries(glm.exe ${DEPENDENCIES} ${LIB_LINK})

# **********************
# ***  Libraries     ***
# **********************

add_library(nlp STATIC "${TOPLEVEL_PREFIX_PATH}/app/nlp.cpp")
add_dependencies(nlp ${DEPENDENCIES})
target_include_directories(nlp INTERFACE ${DEPENDENCIES})
set_target_properties(nlp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(nlp ${DEPENDENCIES} ${LIB_LINK})

add_library(glm STATIC "${TOPLEVEL_PREFIX_PATH}/app/glm.cpp")
add_dependencies(glm ${DEPENDENCIES})
target_include_directories(glm INTERFACE ${DEPENDENCIES})
set_target_properties(glm PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(glm ${DEPENDENCIES} ${LIB_LINK})

# ***************************
# ***  Python-binding     ***
# ***************************

# https://pybind11.readthedocs.io/en/stable/compiling.html#configuration-variables
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(andromeda_structs "${TOPLEVEL_PREFIX_PATH}/app/pybind_structs.cpp")
pybind11_add_module(andromeda_nlp "${TOPLEVEL_PREFIX_PATH}/app/pybind_nlp.cpp")
pybind11_add_module(andromeda_glm "${TOPLEVEL_PREFIX_PATH}/app/pybind_glm.cpp")

add_dependencies(andromeda_structs nlp)
target_include_directories(andromeda_structs INTERFACE ${DEPENDENCIES})
target_compile_definitions(andromeda_structs PRIVATE VERSION_INFO=${CMAKE_PROJECT_VERSION})
target_link_libraries(andromeda_structs PRIVATE nlp)

add_dependencies(andromeda_nlp nlp)
target_include_directories(andromeda_nlp INTERFACE ${DEPENDENCIES})
target_compile_definitions(andromeda_nlp PRIVATE VERSION_INFO=${CMAKE_PROJECT_VERSION})
target_link_libraries(andromeda_nlp PRIVATE nlp)

add_dependencies(andromeda_glm glm)
target_include_directories(andromeda_glm INTERFACE ${DEPENDENCIES})
target_compile_definitions(andromeda_glm PRIVATE VERSION_INFO=${CMAKE_PROJECT_VERSION})
target_link_libraries(andromeda_glm PRIVATE glm)

# *****************
# ***  Install  ***
# *****************

# add the install targets

install(TARGETS andromeda_structs DESTINATION "${TOPLEVEL_PREFIX_PATH}/deepsearch_glm")
install(TARGETS andromeda_nlp DESTINATION "${TOPLEVEL_PREFIX_PATH}/deepsearch_glm")
install(TARGETS andromeda_glm DESTINATION "${TOPLEVEL_PREFIX_PATH}/deepsearch_glm")

# *****************
# ***  Testing  ***
# *****************
