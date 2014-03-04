CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
#CMAKE_POLICY(VERSION 2.8.12)

PROJECT(fusepp)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules/")

OPTION (FUSEPP_STATIC "Build the static version of the library (WARNING: LGPL is tricky w.r.t. statically linked libs, see the README)" OFF)
OPTION (FUSEPP_BUILD_SAMPLES "Build the samples" ON)
OPTION (FUSEPP_BUILD_TESTS "Build the tests" ON)

INCLUDE_DIRECTORIES (${fusepp_SOURCE_DIR}/include )

add_definitions("-std=c++11")

ADD_SUBDIRECTORY(src)
