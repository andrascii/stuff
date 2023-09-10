cmake_minimum_required(VERSION 3.16)

#if (WIN32)
#  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/conaninstall.bat ${CMAKE_BINARY_DIR})
#else()
#  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/conaninstall.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
#endif()

#
# by default generates project to build release package
#
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(USE_CLANG "Enables using clang as a compiler" ON)
option(BUILD_TESTS "Enables building tests project" OFF)

#
# by default on UNIX systems we are using clang compiler
#
if (UNIX AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" AND USE_CLANG)
  set(CMAKE_C_COMPILER "clang")
  set(CMAKE_CXX_COMPILER "clang++")
endif()

project("stuff" LANGUAGES C CXX)

#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()

#
# adding another one path to find .cmake files
#
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# adding settings for cl/clang compiler (depends on which compiler actully used)
#
include(clang_options)
include(msvc_options)

#
# function to register source files for reformatting by clang-format
#
include(add_clang_format)

message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")

include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(
  CPPKAFKA_INCLUDES
  NAMES cppkafka.h
  HINTS ${CURRENT_INSTALLED_DIR}
  PATH_SUFFIXES cppkafka
)

message(STATUS "CPPKAFKA DEBUG PATH: ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug")

find_library(
  CPPKAFKA_LIBRARY_DEBUG
  NAMES cppkafka
  PATH_SUFFIXES lib
  PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug"
  NO_DEFAULT_PATH REQUIRED
)

find_library(
  CPPKAFKA_LIBRARY_RELEASE
  NAMES cppkafka
  PATH_SUFFIXES lib
  PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}"
  NO_DEFAULT_PATH REQUIRED
)

select_library_configurations(CPPKAFKA)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(hffix CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)
find_package(RdKafka CONFIG REQUIRED)

#
# C++ compiler settings
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(PROJECTS_DIR projects)
file(GLOB CHILDREN ${PROJECTS_DIR}/*)

foreach(CHILD ${CHILDREN})
  if (IS_DIRECTORY ${CHILD} AND EXISTS ${CHILD}/CMakeLists.txt)
    message(STATUS "${CHILD} is a directory and it contains CMakeLists.txt so it'll be added as a target")
    get_filename_component(TARGET_NAME ${CHILD} NAME)
    list(APPEND TARGETS ${TARGET_NAME})
  endif()
endforeach()

if (NOT TARGETS)
  message(FATAL "Not found targets")
endif()

foreach(TARGET ${TARGETS})
  add_subdirectory(${PROJECTS_DIR}/${TARGET})
endforeach()

#
# collecting sources to register them for reformat by clang-format utility
#
foreach(MY_TARGET ${TARGETS})
  get_property(SOURCES TARGET ${MY_TARGET} PROPERTY SOURCES_PROPERTY)
  list(APPEND REFORMAT_SOURCES ${SOURCES})
endforeach()

#
# creating target 'reformat' to trigger calling clang-format utility for each source/header file in the project
#
add_clang_format(
  TARGET_NAME reformat
  SOURCES_LIST ${REFORMAT_SOURCES}
)
