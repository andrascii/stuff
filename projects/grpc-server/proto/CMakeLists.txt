cmake_minimum_required(VERSION 3.16)

include(make_executable_sanitized_target)

set(THIS_TARGET_NAME proto)
project(${THIS_TARGET_NAME})

#
# deps
#
list(APPEND DEPS gRPC::grpc)
list(APPEND DEPS gRPC::grpc++)
list(APPEND DEPS protobuf::libprotobuf)

#
# collecting proto files
#
file(GLOB_RECURSE PROTO_FILES "*.proto")

#
# adding include directories to created target
#
include_directories(${ADDITIONAL_INCLUDE_DIRECTORIES})

#
# creating target
#
add_library(${THIS_TARGET_NAME} STATIC ${PROTO_FILES})

#
# linking this target with other targets
#
target_link_libraries(${THIS_TARGET_NAME} PUBLIC ${DEPS})

#
# adding generated protobuf/grpc files
#
target_include_directories(${THIS_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#
# getting grpc C++ plugin location
#
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

protobuf_generate(
  TARGET ${THIS_TARGET_NAME}
  LANGUAGE CPP
)

protobuf_generate(
  TARGET ${THIS_TARGET_NAME}
  LANGUAGE GRPC
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
  PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)

#
# creating sanitized version of this target to check UB
#
#make_executable_sanitized_target(
#  TARGET_NAME ${THIS_TARGET_NAME}-sanitized
#  SOURCES ${SOURCES_LIST}
#  HEADERS ${HEADERS_LIST}
#  DEPS ${DEPS}
#  PRECOMPILED_HEADER ${PRECOMPILED_HEADER}
#)
