#!/bin/bash
set -e

##################################################################################################
## Modified to support AVR-USB-MCUs by Virtual Java                                             ##
## https://github.com/Virtual-Java/optiboot                                                     ##
##                                                                                              ##
## Created by MCUdude for compiling Optiboot flash from source                                  ##
## https://github.com/MCUdude/optiboot_flash                                                    ##
##                                                                                              ##
## Execute ./makeall to build all variants of this bootloader.                                  ##
## Run $ chmod +x if this file isn't executable.                                                ##
##                                                                                              ##
## This is the file contains all all make procedures for all microcontrollers. You can modify   ##
## the mcu_and_params array if you want to add, remove or edit any of the settings. You can     ##
## also modify the clock_and_baud array if you need other F_CPUs or baudrates.                  ##
##                                                                                              ##
## The table below shows the available precompiled bootloaders for the corresponding            ##
## clock frequencies and baud rates.                                                            ##
##                                                                                              ##
## |             | 1000000 | 500000 | 250000 | 230400 | 115200 | 57600 | 38400 | 19200 | 9600 | ##
## |-------------|---------|--------|--------|--------|--------|-------|-------|-------|------| ##
## | 20 MHz      |         |  X     |  X     |        |  X     |       |       |  X    |      | ##
## | 18.4320 MHz |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | ##
## | 16 MHz      |  X      |  X     |  X     |        |  X     |       |  X    |  X    |  X   | ##
## | 14.7456 MHz |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | ## 
## | 12 MHz      |         |  X     |  X     |        |        |  X    |       |  X    |  X   | ##
## | 11.0592 MHz |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | ## 
## | 8 MHz       |  X      |  X     |  X     |        |  X     |  X    |  X    |  X    |  X   | ##
## | 7.3728 MHz  |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | ## 
## | 4 MHz       |         |  X     |  X     |        |        |       |       |  X    |  X   | ##
## | 3.6864 MHz  |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | ## 
## | 2 MHz       |         |        |  X     |        |        |       |       |  X    |  X   | ##
## | 1.8432 MHz  |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | ## 
## | 1 MHz       |         |        |        |        |        |       |       |       |  X   | ## 
##                                                                                              ##
##################################################################################################

declare -a mcu_and_params=(
"atmega8u2"   "LED=D5 LED_START_FLASHES=2 UART=1"
"atmega16u2"  "LED=D5 LED_START_FLASHES=2 UART=1"
"atmega32u2"  "LED=D5 LED_START_FLASHES=2 UART=1"
"atmega16u4"  "LED=C7 LED_START_FLASHES=0 UART=1" # disable blinking the LED to make the bootloader to fit in 512 Bytes Bootsection
"atmega32u4"  "LED=C7 LED_START_FLASHES=0 UART=1" # disable blinking the LED to make the bootloader to fit in 512 Bytes Bootsection
"atmega32u6"  "LED=C6 LED_START_FLASHES=2 UART=1" # for some reason it is not necessary to disable blinking the LED for this device
"at90usb646"  "LED=C6 LED_START_FLASHES=2 UART=1 BIGBOOT=1" # enable bigboot since at90usbXYZS devices have minimal bootsection of 1048 Bytes
"at90usb647"  "LED=C6 LED_START_FLASHES=2 UART=1 BIGBOOT=1" # enable bigboot since at90usbXYZS devices have minimal bootsection of 1048 Bytes
"at90usb1286"  "LED=C6 LED_START_FLASHES=2 UART=1 BIGBOOT=1" # enable bigboot since at90usbXYZS devices have minimal bootsection of 1048 Bytes
"at90usb1287"  "LED=C6 LED_START_FLASHES=2 UART=1 BIGBOOT=1" # enable bigboot since at90usbXYZS devices have minimal bootsection of 1048 Bytes
)

declare -a clock_and_baud=(
# "20000000L" "500000"
"20000000L" "250000"
"20000000L" "115200"
# "20000000L" "19200"
# "18432000L" "230400"
# "18432000L" "115200"
# "18432000L" "57600"
# "18432000L" "38400"
# "18432000L" "19200"
# "18432000L" "9600"
# "16000000L" "1000000"
# "16000000L" "500000"
"16000000L" "250000"
"16000000L" "115200"
# "16000000L" "38400"
# "16000000L" "19200"
# "16000000L" "9600"
# "14745600L" "230400"
# "14745600L" "115200"
# "14745600L" "57600"
# "14745600L" "38400"
# "14745600L" "19200"
# "14745600L" "9600"
# "12000000L" "500000"
# "12000000L" "250000"
#"12000000L" "57600"
# "12000000L" "19200"
# "12000000L" "9600"
# "11059200L" "230400"
# "11059200L" "115200"
# "11059200L" "57600"
# "11059200L" "38400"
# "11059200L" "19200"
# "11059200L" "9600"
# "8000000L" "1000000" 
# "8000000L" "500000" 
"8000000L" "250000" 
"8000000L" "115200" 
"8000000L" "57600"
# "8000000L" "38400"
# "8000000L" "19200"
# "8000000L" "9600"
# "7372800L" "230400"
# "7372800L" "115200"
# "7372800L" "57600"
# "7372800L" "38400"
# "7372800L" "19200"
# "7372800L" "9600"
# "4000000L" "500000"
# "4000000L" "250000"
# "4000000L" "19200"
# "4000000L" "9600"
# "3686400L" "230400"
# "3686400L" "115200"
# "3686400L" "57600"
# "3686400L" "38400"
# "3686400L" "19200"
# "3686400L" "9600"
# "2000000L" "250000"
# "2000000L" "19200"
# "2000000L" "9600"
# "1843200L" "230400"
# "1843200L" "115200"
# "1843200L" "57600"
# "1843200L" "38400"
# "1843200L" "19200"
# "1843200L" "9600"
# "1000000L" "9600"
)


# Start out by deleting all previous generated files
echo -e "\x1B[7m\n\nDeleting all previous generated files \x1B[0m"
#make clean_all
make clean

# Loop through all MCUs and parameters
for((i=0; i < ${#mcu_and_params[@]}; i+=2));
do
  # Loop through all clock speeds and baud rated
  for((j=0; j < ${#clock_and_baud[@]}; j+=2));
  do
     # Count how many builds that have been ran
     COUNTER=$(($COUNTER + 1))
     echo -e "\x1B[7m\n\n$COUNTER of $((${#mcu_and_params[@]} * ${#clock_and_baud[@]} / 4))"
     # Print out current build info
     echo -e " ${mcu_and_params[$i]} AVR_FREQ=${clock_and_baud[$j]} BAUD_RATE=${clock_and_baud[$j+1]} ${mcu_and_params[$i+1]} \x1B[0m"
     # Build
     make ${mcu_and_params[$i]} AVR_FREQ=${clock_and_baud[$j]} BAUD_RATE=${clock_and_baud[$j+1]} ${mcu_and_params[$i+1]} $*
  done
done

# Finish up by removing all *.lst files
echo -e "\x1B[7m\n\nDeleting all generated *.lst files \x1B[0m"
make clean_asm
