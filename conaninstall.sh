#!/bin/bash

set -x

SOURCE_DIR=$1
BUILD_DIR=$2

if [ $? -gt 0 ]
then
  echo "Expected argument determining the build directory to install conan libraries"
  exit 1
fi

VERSION=$( conan --version )
IFS=. read -r VERSION <<< $VERSION
MAJOR=$( echo $VERSION | awk '{print $3}' )

if [ $MAJOR -gt 1  ]; then
  conan install $SOURCE_DIR --output-folder $BUILD_DIR
else
  conan install $SOURCE_DIR --install-folder $BUILD_DIR --build missing
fi
