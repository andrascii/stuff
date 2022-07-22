#!/bin/bash

SOURCE_DIR=$1
BUILD_DIR=$2

if [ $? -gt 0 ]
then
  echo "Expected argument determining the build directory to install conan libraries"
  exit 1
fi

conan install $SOURCE_DIR --install-folder $BUILD_DIR --build missing
