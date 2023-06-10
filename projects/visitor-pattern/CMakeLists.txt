cmake_minimum_required(VERSION 3.16)

include(make_static_library_sanitized_target)

set(THIS_TARGET_NAME visitor-pattern)
project(${THIS_TARGET_NAME})

#
# deps include directories
#
list(APPEND ADDITIONAL_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")

#
# deps
#
list(APPEND DEPS ${Boost_LIBRARIES})

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
#target_precompile_headers(${THIS_TARGET_NAME} PRIVATE ${PRECOMPILED_HEADER})

#
# linking this target with other targets
#
target_link_libraries(${THIS_TARGET_NAME} PUBLIC ${DEPS})

#
# creating sanitized version of this target to check UB
#
make_static_library_sanitized_target(
  TARGET_NAME ${THIS_TARGET_NAME}-sanitized
  SOURCES ${SOURCES_LIST}
  HEADERS ${HEADERS_LIST}
  DEPS ${DEPS}
  #PRECOMPILED_HEADER ${PRECOMPILED_HEADER}
)
