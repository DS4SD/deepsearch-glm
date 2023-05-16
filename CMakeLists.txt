cmake_minimum_required(VERSION 3.5.0)

project(andromeda VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_VERBOSE_MAKEFILE off)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON )
endif()

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

message(STATUS "    top path: " ${TOPLEVEL_PREFIX_PATH})
message(STATUS "    lib path: " ${EXTERNALS_PREFIX_PATH})
message(STATUS "install path: " ${CMAKE_INSTALL_PREFIX})
message(STATUS "  cmake path: " ${CMAKE_MODULE_PATH})
message(STATUS " python path: " ${})

# ************************
# ***  cxx-flags       ***
# ************************

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g3")

message(STATUS "cxx-compiler: " ${CMAKE_CXX_COMPILER})
message(STATUS "cxx-flags   : " ${CMAKE_CXX_FLAGS})
message(STATUS "cxx-standard: " ${CMAKE_CXX_STANDARD})

# ************************
# *** libraries        ***
# ************************

if(NOT EXISTS ${EXTERNALS_PREFIX_PATH})
  execute_process(COMMAND mkdir ${EXTERNALS_PREFIX_PATH})
  execute_process(COMMAND mkdir ${EXTERNALS_PREFIX_PATH}/lib)
  execute_process(COMMAND mkdir ${EXTERNALS_PREFIX_PATH}/bin)
  execute_process(COMMAND mkdir ${EXTERNALS_PREFIX_PATH}/include)
  execute_process(COMMAND mkdir ${EXTERNALS_PREFIX_PATH}/resources)
endif()

#find_package(OpenSSL REQUIRED)

include(cmake/subdirlist.cmake)

include(cmake/extlib_json.cmake)
#include(cmake/extlib_json_schema.cmake)
include(cmake/extlib_loguru.cmake)
include(cmake/extlib_cxxopts.cmake)
include(cmake/extlib_pcre2.cmake)
include(cmake/extlib_fasttext.cmake)
#include(cmake/extlib_utf8.svn.cmake)
include(cmake/extlib_utf8.git.cmake)
include(cmake/extlib_pybind11.cmake)
#include(cmake/extlib_andromeda_pos.cmake)

include(cmake/os_opts.cmake)

# ***********************
# ***  includes       ***
# ***********************

include_directories(${EXTERNALS_PREFIX_PATH}/include)

subdirlist(EXTERNALS_INCLUDES ${EXTERNALS_PREFIX_PATH}/include)	
include_directories(${EXTERNALS_INCLUDES})

message(STATUS "subdirs: " ${SUBDIRS})

include_directories(${TOPLEVEL_PREFIX_PATH}/src)

subdirlist(SUBDIRS ${TOPLEVEL_PREFIX_PATH}/src)
include_directories(${SUBDIRS})

# **********************
# ***  Executables   ***
# **********************

add_executable(nlp.exe "${TOPLEVEL_PREFIX_PATH}/app/nlp.cpp")
add_executable(glm.exe "${TOPLEVEL_PREFIX_PATH}/app/glm.cpp")

set_property(TARGET nlp.exe PROPERTY CXX_STANDARD 20)
set_property(TARGET glm.exe PROPERTY CXX_STANDARD 20)

target_link_libraries(nlp.exe ${LIB_LINK})
target_link_libraries(glm.exe ${LIB_LINK})

# **********************
# ***  Libraries     ***
# **********************

add_library(nlp STATIC "${TOPLEVEL_PREFIX_PATH}/app/nlp.cpp")
add_library(glm STATIC "${TOPLEVEL_PREFIX_PATH}/app/glm.cpp")

set_target_properties(nlp PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(glm PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(nlp ${LIB_LINK})
target_link_libraries(glm ${LIB_LINK})

# ***************************
# ***  Python-binding     ***
# ***************************

# inspiration from
# 1. https://github.com/pybind/cmake_example/blob/master/CMakeLists.txt`
# 2. https://blog.devgenius.io/calling-python-and-c-code-using-pybind-99ab7fefa685
# 3. https://github.com/pybind/cmake_example/issues/11

include(FetchContent)
FetchContent_Declare(pybind11
  FETCHCONTENT_BASE_DIR extlib_pybind11 
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.10.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(pybind11)

pybind11_add_module(andromeda_nlp "${TOPLEVEL_PREFIX_PATH}/app/pybind_nlp.cpp")
pybind11_add_module(andromeda_glm "${TOPLEVEL_PREFIX_PATH}/app/pybind_glm.cpp")

target_compile_definitions(andromeda_nlp PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
target_compile_definitions(andromeda_glm PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})

target_link_libraries(andromeda_nlp PRIVATE nlp)
target_link_libraries(andromeda_glm PRIVATE glm)
