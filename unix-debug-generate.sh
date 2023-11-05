#!/bin/bash

set -x

cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$VCPKG_CMAKE_FILE
