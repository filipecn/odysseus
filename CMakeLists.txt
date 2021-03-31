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
        odysseus/containers/object_pool.h
        odysseus/debug/debug.h
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
if (BUILD_TESTS OR BUILD_ALL)
    add_subdirectory(tests)
endif (BUILD_TESTS OR BUILD_ALL)

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
    # pip3 install sphinx
    # pip3 install breathe
    # pip3 install sphinx_rtd_theme
    # check if Doxygen is installed
    find_package(Doxygen)
    find_package(Sphinx)
    if (DOXYGEN_FOUND AND SPHINX_FOUND)
        set(DOXYGEN_INPUT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/odysseus)
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen)
        set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        # create doxygen output dir
        file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")
        # note the option ALL which allows to build the docs together with the application
        add_custom_target(doc_doxygen ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM)


        set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/docs/source)
        set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/docs/sphinx)

        add_custom_target(Sphinx ALL
                COMMAND ${SPHINX_EXECUTABLE} -b html
                # Tell Breathe where to find the Doxygen output
                -Dbreathe_projects.Odysseus=${DOXYGEN_OUTPUT_DIR}/xml
                ${SPHINX_SOURCE} ${SPHINX_BUILD}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating documentation with Sphinx")

    else (DOXYGEN_FOUND AND SPHINX_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND AND SPHINX_FOUND)
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