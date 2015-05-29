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
mkdir /tmp/optiboot-release
mkdir /tmp/optiboot-release/Optiboot
mkdir /tmp/optiboot-release/Optiboot/avr
mkdir /tmp/optiboot-release/Optiboot/avr/bootloaders
mkdir /tmp/optiboot-release/Optiboot/avr/bootloaders/optiboot
cp ../../boards-1.6.txt /tmp/optiboot-release/Optiboot/avr/boards.txt
#
# Create platform.tx, because it contains the "group" name for the boards menu
echo name=\[Optiboot $1\] > /tmp/optiboot-release/Optiboot/avr/platform.txt
echo version=$1 >> /tmp/optiboot-release/Optiboot/avr/platform.txt

#
# Create a README file.
echo This is an Optiboot version $1 \"Binary\" Release. > /tmp/optiboot-release/Optiboot/README.TXT
echo >> /tmp/optiboot-release/Optiboot/README.TXT
echo For Source code see http://github.com/Optiboot/optiboot >> /tmp/optiboot-release/Optiboot/README.TXT

#
# Copy over our "binaries."
cp *.hex /tmp/optiboot-release/Optiboot/avr/bootloaders/optiboot

# files we'd specifical exclude, if we weren't doing only .hex files.
#rm /tmp/optiboot-release/Optiboot/avr/bootloaders/optiboot/*.lst
#rm /tmp/optiboot-release/Optiboot/avr/bootloaders/optiboot/*~
#rm /tmp/optiboot-release/Optiboot/avr/bootloaders/optiboot/#*

#
# zip everything up.
pushd /tmp/optiboot-release
zip -r Optiboot.zip Optiboot
popd

