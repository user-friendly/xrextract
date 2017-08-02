#!/usr/bin/env bash

if [ ! "$1" = "" ]; then
    SOURCE_DIR="$1"
else
    SOURCE_DIR="$(pwd)"
fi

if [ ! "$2" = "" ]; then
    BUILD_DIR="$2"
else
    BUILD_DIR="$SOURCE_DIR/bin"
fi

echo "Source directory is: $SOURCE_DIR"
echo "Build directory is: $BUILD_DIR"

if [ ! -d "$BUILD_DIR" ]; then
    echo "mkdir $BUILD_DIR"
    mkdir $BUILD_DIR
fi

echo "cd $BUILD_DIR"
cd $BUILD_DIR

# The configure script should use these environmental variables.
CC=CLANG
CXX=clang++
CPPFLAGS="-I$SOURCE_DIR/libs/include"

# Use the custom built Boost & SDL libraries.
# Comment out the following 3 lines to used the system
# provided libraries, if any.
WITH_BOOST="--with-boost=$SOURCE_DIR/libs"
WITH_BOOST_LIBDIR="--with-boost-libdir=$SOURCE_DIR/libs/lib"
LINKER_LDFLAGS=""
LINKER_LIBS="-Wl,-rpath=$SOURCE_DIR/libs/lib"

CONFIGURE_FLAGS=""

echo $SOURCE_DIR/configure \
    CPPFLAGS="$CPPFLAGS" \
    $WITH_BOOST $WITH_BOOST_LIBDIR --enable-debug \
    LIBS="$LINKER_LIBS"

$SOURCE_DIR/configure \
    CPPFLAGS="$CPPFLAGS" \
    $WITH_BOOST $WITH_BOOST_LIBDIR --enable-debug \
    LIBS="$LINKER_LIBS"
