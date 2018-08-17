#!/usr/bin/env bash

LOCAL_TOOLS_DIR=$HOME/avr-tools

if [ -z "$TRAVIS_BUILD_DIR" ]; then
    echo "This script should be run by Travis-CI environment"
    echo "If you want to simulate Travis build, please set TRAVIS_BUILD_DIR"
    echo "environment variable to directory where your code lives"
    exit 1
fi

if [ -z "$1" ]; then
    echo "Arduino version required"
    exit 1
fi

if [ -z "$2" ]; then
    echo "Target required"
    exit 1
fi


# Include functions to download stuff
. $TRAVIS_BUILD_DIR/scripts/travis-download.inc.sh

# Make directory for tools
mkdir -p $LOCAL_TOOLS_DIR

# get new make as Optiboot requires version >4.0
download_make4

# download specific tools version
if [ "$1" = "microchip" ]; then
    download_avr_toolchain

    # set search path
    PATH=$LOCAL_TOOLS_DIR/usr/bin:$PATH:$LOCAL_TOOLS_DIR/avr8-gnu-toolchain-linux_x86_64/bin
    avr-gcc --version
else
    download_arduino $1

    # set search path
    PATH=$LOCAL_TOOLS_DIR/usr/bin:$PATH:$LOCAL_TOOLS_DIR/arduino-$1/hardware/tools/avr/bin
fi

cd $TRAVIS_BUILD_DIR/optiboot/bootloaders/optiboot

make --version
make clean
shift
make $@
