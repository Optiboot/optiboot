# xTiny
export PACKS=/Downloads/Atmel.ATtiny_DFP.1.3.229

make -f Makefile.mega0 version

make -f Makefile.mega0 atmega4809 LED=D6 UARTTX=A0

make -f Makefile.mega0 attiny402 LED=A3 TIMEOUT=8 UARTTX=A1
make -f Makefile.mega0 attiny412 LED=A3 TIMEOUT=8 UARTTX=A6

make -f Makefile.mega0 attiny804 LED=A3 TIMEOUT=8 UARTTX=A1
make -f Makefile.mega0 attiny814 LED=A3 TIMEOUT=8 UARTTX=B2
make -f Makefile.mega0 attiny406 LED=A3 TIMEOUT=8 UARTTX=A1
make -f Makefile.mega0 attiny816 LED=A3 TIMEOUT=8 UARTTX=B2
make -f Makefile.mega0 attiny1617 LED=A3 TIMEOUT=8 UARTTX=A1
make -f Makefile.mega0 attiny3217 LED=A3 TIMEOUT=8 UARTTX=B2

make -f Makefile.mega0 drazzy402
make -f Makefile.mega0 drazzy814
make -f Makefile.mega0 drazzy3216
make -f Makefile.mega0 drazzy1607
make -f Makefile.mega0 curiosity1607
make -f Makefile.mega0 xplained416

# Mega0
export PACKS=/Downloads/Atmel.ATmega_DFP.1.3.300

make -f Makefile.mega0 version

make -f Makefile.mega0 atmega4809 LED=D6 UARTTX=A0
make -f Makefile.mega0 atmega809 LED=D6 UARTTX=A4
make -f Makefile.mega0 atmega1609 LED=D6 UARTTX=C0
make -f Makefile.mega0 atmega3209 LED=D6 UARTTX=C4
make -f Makefile.mega0 atmega4808 LED=D6 UARTTX=F0
make -f Makefile.mega0 atmega4809 LED=D6 UARTTX=F4
make -f Makefile.mega0 atmega4809 LED=D6 UARTTX=B0
make -f Makefile.mega0 atmega4809 LED=D6 UARTTX=B4

make -f Makefile.mega0 xplained4809
make -f Makefile.mega0 freeduino4809
make -f Makefile.mega0 freeduino4809chip
make -f Makefile.mega0 curiosity4809
make -f Makefile.mega0 xplained4809


