#!/usr/bin/env bash

CURRENT_DIR="$(pwd)"
BOOST_ROOT_DIR="$1"
INSTALL_DIR="$2"

if [ "$0" == "help" ] || [ "$0" == "-help" ] || [ "$0" == "--help" ] || [ "$0" == "-h" ]; then
    echo "Usage: boost-custom-build.sh BOOST_ROOT_DIR [INSTALL_DIR]"
    exit 0
fi



if [ -z "$BOOST_ROOT_DIR" ]; then
    echo "error: boost root directory is required"
    exit 1
fi

BOOST_ROOT_DIR="$(readlink -fn $BOOST_ROOT_DIR)"

if [ ! -x "$BOOST_ROOT_DIR/b2" ]; then
    echo "error: '$BOOST_ROOT_DIR/b2' does not exist or is not executable"
    exit 1
fi

if [ ! -z "$INSTALL_DIR" ] && [ ! -d "$INSTALL_DIR" ]; then
    echo "error: installation directory '$INSTALL_DIR' does not exist"
    exit 1
fi

if [ -z "$INSTALL_DIR" ]; then
    INSTALL_DIR="$(pwd)/libs"
fi

INSTALL_DIR="$(readlink -fn $INSTALL_DIR)"

B2_OPTIONS="--build-type=minimal --with-system --with-program_options --with-filesystem"
B2_PROPERTIES="toolset=clang variant=release link=shared runtime-link=shared threading=multi"
B2_TARGETS="install"

B2_OPTS="--prefix=$INSTALL_DIR dll-path=$INSTALL_DIR/lib $B2_OPTIONS $B2_PROPERTIES $B2_TARGETS"

echo -e " \n \
     Boost root dir: $BOOST_ROOT_DIR \n \
     B2 executable:  $BOOST_ROOT_DIR/b2 \n \
     Install dir:    $INSTALL_DIR \n \
     B2 options:     $B2_OPTS \n \
"

COMMAND_TO_RUN="$BOOST_ROOT_DIR/b2 $B2_OPTS"

echo "Run command: $COMMAND_TO_RUN"
select yn in "Yes" "No"; do
    case $yn in
        "Yes" ) break ;;
        "No" ) echo "Too scared?"; exit 0 ;;
    esac
done

echo "Change dir to $BOOST_ROOT_DIR"

cd $BOOST_ROOT_DIR

$COMMAND_TO_RUN

echo "Change dir back to $CURRENT_DIR"

cd $CURRENT_DIR
