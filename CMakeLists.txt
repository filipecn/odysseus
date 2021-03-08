# avoid policy warning message
if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)
# main project
project(ODYSSEUS)
cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)
# path variables
set(ROOT_PROJECT_PATH "${PROJECT_SOURCE_DIR}")
set(BUILD_ROOT ${CMAKE_CURRENT_BINARY_DIR})
##########################################
##               options                ##
##########################################
option(BUILD_ALL "build all libraries" OFF)
option(BUILD_TESTS "build library unit tests" OFF)
option(BUILD_EXAMPLES "build library examples" OFF)
option(BUILD_SHARED "build shared library" OFF)
option(BUILD_DOCS "build library documentation" OFF)
set(INSTALL_PATH ${BUILD_ROOT} CACHE STRING "include and lib folders path")
# cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
# check for specific machine/compiler options.
include(ConfigCompilerFlags)
##########################################
##             dependencies             ##
##########################################
# include external libraries path
include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext")
include(catch2)
include(ponos)
include(circe)
##########################################
##               source                ##
##########################################
set(ODYSSEUS_HEADERS
        odysseus/debug/assert.h
        odysseus/memory/double_stack_allocator.h
        odysseus/memory/mem.h
        odysseus/memory/pool_allocator.h
        odysseus/memory/stack_allocator.h
        )
file(GLOB ODYSSEUS_SOURCES
        odysseus/memory/*.cpp
        )
add_library(odysseus STATIC
        ${ODYSSEUS_SOURCES}
        ${ODYSSEUS_HEADERS}
        )
set_target_properties(odysseus PROPERTIES
        OUTPUT_NAME "odysseus"
        FOLDER "ODYSSEUS"
        )
target_include_directories(odysseus PUBLIC
        ${ODYSSEUS_SOURCE_DIR}
        ${PONOS_INCLUDES}
        ${CIRCE_INCLUDES}
        )
target_link_libraries(odysseus PUBLIC
        ${PONOS_LIBRARIES}
        ${CIRCE_LIBRARIES}
        )

add_dependencies(odysseus circe ponos)

##########################################
##              install                ##
##########################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/odysseus
        DESTINATION ${INSTALL_PATH}/include
        FILES_MATCHING REGEX "(.*\\.[inl|h])")

install(TARGETS odysseus
        LIBRARY DESTINATION ${INSTALL_PATH}/lib
        ARCHIVE DESTINATION ${INSTALL_PATH}//lib
        )
##########################################
##                tests                 ##
##########################################
if(BUILD_TESTS OR BUILD_ALL)
    add_subdirectory(tests)
endif(BUILD_TESTS OR BUILD_ALL)

##########################################
##              examples                ##
##########################################
if (BUILD_EXAMPLES OR BUILD_ALL)
    add_subdirectory(examples)
endif (BUILD_EXAMPLES OR BUILD_ALL)
#########################################
##           documentation              ##
##########################################
if (BUILD_DOCS)
    # TODO
endif (BUILD_DOCS)
##########################################
##         output variables             ##
##########################################
get_directory_property(ODYSSEUS_HAS_PARENT PARENT_DIRECTORY)
if (ODYSSEUS_HAS_PARENT)
    set(ODYSSEUS_INCLUDES ${PONOS_INCLUDES} ${CIRCE_INCLUDES} PARENT_SCOPE)
    set(ODYSSEUS_EXTRA_LIBS PARENT_SCOPE)
endif (ODYSSEUS_HAS_PARENT)
#