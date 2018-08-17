#!/usr/bin/env bash
#
# Download avr-gcc from Arduino and Microchip
# Get also make 4 required to compile Optiboot
#

# path where tools are extracted
LOCAL_TOOLS_DIR=$HOME/avr-tools


# check if we are running by Travis-CI
if [ -z "$TRAVIS_BUILD_DIR" ]; then
    echo "This script should be run by Travis-CI environment"
    echo "If you want to simulate Travis build, please set TRAVIS_BUILD_DIR"
    echo "environment variable to directory where your code lives"
    exit 1
fi

# include functions to download stuff
. $TRAVIS_BUILD_DIR/scripts/travis-download.inc.sh

# make directory for tools
mkdir -p $LOCAL_TOOLS_DIR

# get new make as Optiboot requires version >4.0
download_make4

# download Arduino versions
for version in $(cat $TRAVIS_BUILD_DIR/docs/arduino-gcc-versions.md |grep -i "| yes |"|cut -f 2 -d '|'); do
    download_arduino $version
done

# download Microchip's AVR8 Toolchain
download_avr_toolchain
