#!/bin/bash
make clean
#
#  buildable platforms of somewhat questionable support level
make lilypad $*
make pro8 $*
make pro16 $*
make pro20 $*
make atmega328_pro8 $*
make sanguino $*
make mega1280 $*
make luminet $*
make diecimila $*
make bobuino $*
make wildfirev2 $*
make atmega1284 $*
make atmega32 $*
make atmega88 $*
make atmega168p $*

#
# Atmel development board targets
make xplained168pb $*
make xplained328p $*
make xplained328pb $*

#
# The "big three" standard bootloaders.
# These need to be built AFTER the platforms, or they'll get renamed
make atmega8 $*
make atmega168 $*
make atmega328 $*
