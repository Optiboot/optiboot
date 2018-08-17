# common functions to get avr-gcc tools
#

MAKE_PACKAGE=make_4.1-6_amd64.deb
WGET_FLAGS="--retry-connrefused --tries=3 --timeout=60 --continue"


# download and unpack package
function download_arduino()
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

function download_make4()
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

function download_avr_toolchain()
{
    cd $LOCAL_TOOLS_DIR

    # check if tools are already in place
    if [ -d avr8-gnu-toolchain-linux_x86_64 ]; then
	echo "AVR 8-bit Toolchain already downloaded and extracted, skipping"
	return
    fi

    echo "Downloading AVR 8-bit Toolchain"
    
    # download package
    wget $WGET_FLAGS --content-disposition "https://www.microchip.com/mymicrochip/filehandler.aspx?ddocname=en605750"
    if [ $? -ne 0 ]; then
	echo "ERROR: Can't download AVR 8-bit Toolchain"
	rm avr8-gnu-toolchain-*-linux.any.x86_64.tar.gz*
	exit 1
    fi
    
    # unpack
    tar xf avr8-gnu-toolchain-*-linux.any.x86_64.tar.gz
    
    # clean up
    rm avr8-gnu-toolchain-*-linux.any.x86_64.tar.gz*
}
