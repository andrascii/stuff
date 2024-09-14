#!/bin/bash

set -x

cmake -B build-relwithdeb -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=$VCPKG_CMAKE_FILE
