cmake_minimum_required(VERSION 3.16)

include(make_coverage_target)
include(make_executable_sanitized_target)

set(THIS_TARGET_NAME mdo-tests)
project(${THIS_TARGET_NAME})

#
# precompiled header
#
set(PRECOMPILED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h")

#
# deps include directories
#
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../message-driven-objects")

#
# deps
#
list(APPEND DEPS ${Boost_LIBRARIES})
list(APPEND DEPS spdlog::spdlog)
list(APPEND DEPS GTest::gtest)
list(APPEND DEPS GTest::gtest_main)
list(APPEND DEPS tl::expected)
list(APPEND DEPS message-driven-objects)

#
# collecting sources and headers
#
file(GLOB_RECURSE SOURCES_LIST "*.cpp")
file(GLOB_RECURSE HEADERS_LIST "*.h")

#
# adding include directories to created target
#
include_directories(${ADDITIONAL_INCLUDE_DIRECTORIES})

#
# creating target
#
add_executable(${THIS_TARGET_NAME} ${HEADERS_LIST} ${SOURCES_LIST})

#
# adding sources property to target
#
set_property(
  TARGET ${THIS_TARGET_NAME}
  PROPERTY SOURCES_PROPERTY ${HEADERS_LIST} ${SOURCES_LIST}
)

#
# adding precompiled header
#
target_precompile_headers(${THIS_TARGET_NAME} PRIVATE ${PRECOMPILED_HEADER})

#
# linking this target with other targets
#
target_link_libraries(${THIS_TARGET_NAME} PUBLIC ${DEPS})

#
# creating new tests target with clang's coverage flags
# to test code coverage
#
make_coverage_target(
  TARGET_NAME ${THIS_TARGET_NAME}
)

#
# creating sanitized version of this target to check UB
#
make_executable_sanitized_target(
  TARGET_NAME ${THIS_TARGET_NAME}
  SOURCES ${SOURCES_LIST}
  HEADERS ${HEADERS_LIST}
  DEPS ${DEPS}
  PRECOMPILED_HEADER ${PRECOMPILED_HEADER}
)

#
# register test cases added by gtest (to start them - run 'test' target)
#
gtest_discover_tests(${THIS_TARGET_NAME})