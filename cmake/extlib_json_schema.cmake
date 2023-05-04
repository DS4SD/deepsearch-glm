cmake_minimum_required(VERSION 3.5)

message(STATUS "entering in extlib_json_schema.cmake")

include(ExternalProject)
include(CMakeParseArguments)

set(JSON_SCHEMA_URL https://github.com/pboettch/json-schema-validator.git)
set(JSON_SCHEMA_TAG 2.1.0)

#message(STATUS "json-schema-url: " "${JSON_SCHEMA_URL}")
#message(STATUS "json-schema-tag: " "${JSON_SCHEMA_TAG}")

ExternalProject_Add(extlib_json_schema

    PREFIX extlib_json_schema

    GIT_REPOSITORY ${JSON_SCHEMA_URL}
    GIT_TAG ${JSON_SCHEMA_TAG}

    BUILD_ALWAYS OFF
    UPDATE_COMMAND ""

    INSTALL_DIR ${JSON_SCHEMA_PREFIX_INSTALL_DIR}

    CMAKE_CACHE_ARGS
        -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
	-DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
    CMAKE_ARGS
	-DCMAKE_PREFIX_PATH=${EXTERNALS_PREFIX_PATH}	        
        -DCMAKE_INSTALL_PREFIX=${EXTERNALS_PREFIX_PATH} 
        -DBUILD_SHARED_LIBS=OFF
	-DBUILD_EXAMPLES=OFF
	-DBUILD_TESTS=OFF
	-Dnlohmann_json_DIR=${EXTERNALS_PREFIX_PATH}

    BUILD_IN_SOURCE ON

    LOG_DOWNLOAD ON
    LOG_BUILD ON)

add_library(json_schema STATIC IMPORTED)
set_target_properties(json_schema PROPERTIES IMPORTED_LOCATION ${EXTERNALS_PREFIX_PATH}/lib/libnlohmann_json_schema_validator.a)

add_dependencies(extlib_json_schema extlib_json)
add_dependencies(json_schema extlib_json_schema)
