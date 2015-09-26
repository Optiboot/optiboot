#%/bin/bash

# Build a "release" .zip file for Optiboot bootloader
# Run from the build directory

# Uncomment if you want a clean builds of specific files
# make clean
# make atmega328
# make atmega168
# make atmega8

rm -Rf /tmp/optiboot-release

#
# Create the 3rd-party hardware extension directory structure
# Note that the structure under "packages" (handed by boards manager)
# is different than the structure under "sketchbook/hardware" would be.

TOP=/tmp/optiboot-release/Optiboot/
#
# Bootloaders directory
mkdir -p $TOP/bootloaders/optiboot
# cores, variants, libraries
# mkdir -p $TOP/variants/
mkdir -p $TOP/libraries/
# Less common: firmware, system.
# mkdir -p $TOP/firmwares/
# mkdir -p $TOP/system/

#
# Copy files from whereever into the release director 
cp ../../boards-1.6.txt $TOP/boards.txt
#cp -R ../../examples $TOP/libraries/
#cp -R ../../variants $TOP/
#cp -R ../../system $TOP/

#
# Create platform.tx, because it contains the "group" name for the boards menu
echo name=Optiboot $1 > $TOP/platform.txt
echo version=$1 >> $TOP/platform.txt

#
# Create a README file.
echo This is an Optiboot version $1 \"Binary\" Release. > $TOP/README.TXT
echo >> $TOP/README.TXT
echo For Source code see http://github.com/Optiboot/optiboot>> $TOP/README.TXT

#
# Copy over our "binaries."
cp *.hex $TOP/bootloaders/optiboot

#
# Copy examples ?  (examples for a boards.txt don't seem possible?)


# files we'd specifical exclude, if we weren't doing only .hex files.
#rm /tmp/optiboot-release/Optiboot/avr/bootloaders/optiboot/*.lst
#rm /tmp/optiboot-release/Optiboot/avr/bootloaders/optiboot/*~
#rm /tmp/optiboot-release/Optiboot/avr/bootloaders/optiboot/#*

#
# zip everything up.
pushd /tmp/optiboot-release
zip -r Optiboot.zip Optiboot
HASH=`openssl dgst -sha256 Optiboot.zip | sed -e 's/.* //'`
SIZE=`stat -f %z Optiboot.zip`
popd
sed -e "s/#.*//" -e "s/%HASH%/$HASH/" -e "s/%VERSION%/$1/" -e "s/%SIZE%/$SIZE/" ../../package_optiboot_optiboot-additional_index.json.TEMPLATE > /tmp/optiboot-release/package_optiboot_optiboot-additional_index.json

#
# This leaves the .zip and the .json file in /tmp/optiboot-release
# where it can be copied to a suitable network location.

