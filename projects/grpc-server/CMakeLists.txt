cmake_minimum_required(VERSION 3.16)

set(THIS_TARGET_NAME grpc-example)
project(${THIS_TARGET_NAME})

#
# adding targets
#
add_subdirectory(proto)
add_subdirectory(grpc-server)
add_subdirectory(grpc-client)