#!/bin/bash

set -x

cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$VCPKG_CMAKE_FILE
