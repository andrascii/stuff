cmake_minimum_required(VERSION 3.16)

include(make_static_library_sanitized_target)

set(THIS_TARGET_NAME fixparser)
project(${THIS_TARGET_NAME})

#
# deps
#
list(APPEND DEPS hffix::hffix)
list(APPEND DEPS fmt::fmt)

#
# collecting sources and headers
#
file(GLOB_RECURSE SOURCES_LIST "*.cpp")
file(GLOB_RECURSE HEADERS_LIST "*.h")

#
# creating target
#
add_library(${THIS_TARGET_NAME} STATIC ${HEADERS_LIST} ${SOURCES_LIST})

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
  TARGET_NAME ${THIS_TARGET_NAME}
  SOURCES ${SOURCES_LIST}
  HEADERS ${HEADERS_LIST}
  DEPS ${DEPS}
)