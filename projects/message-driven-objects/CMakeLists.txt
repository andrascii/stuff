cmake_minimum_required(VERSION 3.16)

set(THIS_TARGET_NAME mdo)
project(${THIS_TARGET_NAME})

#
# adding targets
#
add_subdirectory(message-driven-objects)
add_subdirectory(benchmarks)
add_subdirectory(tests)
add_subdirectory(app)