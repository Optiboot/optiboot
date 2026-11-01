# Optiboot Mini 328P integration, October 2026.
# Included via Makefile.custom; original Optiboot Makefile and C targets remain untouched.
# Fixed configuration: ATmega328P, 16 MHz, 115200 baud, 512-byte boot area at 0x7E00.

HELPTEXT += "target mini328       - ATmega328P Optiboot Mini + EEPROM/STK500v1 (512 bytes)\n"

.PHONY: mini328 mini328_check
mini328:
	python3 build_mini328.py

mini328_check:
	python3 build_mini328.py --verify-reference
