cmake_minimum_required(VERSION 3.16)

set(THIS_TARGET_NAME mdo)
project(${THIS_TARGET_NAME})

#
# adding targets
#
add_subdirectory(benchmarks)
add_subdirectory(fixparser)
add_subdirectory(app)