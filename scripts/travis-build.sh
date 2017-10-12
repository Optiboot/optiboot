#!/usr/bin/env bash

LOCAL_TOOLS_DIR=$HOME/avr-tools
MAKE_PACKAGE=make_4.1-6_amd64.deb
WGET_FLAGS="--retry-connrefused --tries=3 --timeout=60 --continue"


if [ -z "$TRAVIS_BUILD_DIR" ]; then
    echo "This script should be run by Travis-CI environment"
    echo "If you want to simulate Travis build, please set TRAVIS_BUILD_DIR"
    echo "envirinment variable to directory where your code lives"
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


# oownload and unpack package
function download_and_unpack()
{
    cd $LOCAL_TOOLS_DIR

    # check if tools are already in place
    if [ -d arduino-$1/hardware/tools/avr ]; then
	echo "Arduino version $1 already downloaded and extracted, skipping"
	return
    fi

    echo "Downloading Arduino version $1"
    
    # default package extension
    local arduExt="tar.xz"
    
    # for packages in version <1.6 extension is .tgz
    local regex="1\.[05]"
    if [[ "$1" =~ $regex ]]; then arduExt="tgz"; fi
    
    # download package
    wget $WGET_FLAGS "http://downloads.arduino.cc/arduino-$1-linux64.$arduExt"
    if [ $? -ne 0 ]; then
	echo "ERROR: Can't download Arduino"
	rm arduino-$1-linux64.$arduExt*
	exit 1
    fi
    
    # try to check md5sum, but Arduino provide only checksums for version 1.6 and greater
    wget $WGET_FLAGS https://downloads.arduino.cc/arduino-$1.md5sum.txt
    if [ $? -eq -0 ]; then
	cat arduino-$1.md5sum.txt|grep "linux64"|md5sum -c
	if [ $? -ne 0 ]; then
	    echo "ERROR: md5sum for downloaded Arduino doesn't match"
	    rm arduino-$1.md5sum.txt*
	    exit 1
	fi
	rm arduino-$1.md5sum.txt*
    fi
    
    # extract only avr-gcc
    tar xf arduino-$1-linux64.$arduExt --wildcards '*/hardware/tools/avr/'

    # clean up
    rm arduino-$1-linux64.$arduExt*
}

function get_make4()
{
    cd $LOCAL_TOOLS_DIR
    
    # check for existence
    if [ -x usr/bin/make ]; then
	echo "Make already in place, skipping"
	return
    fi
    
    # download
    wget http://archive.ubuntu.com/ubuntu/pool/main/m/make-dfsg/$MAKE_PACKAGE
    if [ $? -ne 0 ]; then
	echo "ERROR: Can't download make4"
	exit 1
    fi
    
    # unpack
    dpkg-deb -x $MAKE_PACKAGE $LOCAL_TOOLS_DIR

    # clean up
    rm ${MAKE_PACKAGE}*
}

# make directory for tools
mkdir -p $LOCAL_TOOLS_DIR

# get new make as Optiboot requires version >4.0
get_make4

# download specific Arduino version
download_and_unpack $1

# set search path
PATH=$LOCAL_TOOLS_DIR/usr/bin:$PATH:$LOCAL_TOOLS_DIR/arduino-$1/hardware/tools/avr/bin

cd $TRAVIS_BUILD_DIR/optiboot/bootloaders/optiboot

make --version
make clean
make $2
