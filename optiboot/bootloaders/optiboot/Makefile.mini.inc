# Additional Optiboot Mini targets. Safe to include from upstream Makefile.custom.
# The main Optiboot Makefile, original optiboot.c, and other targets remain unchanged.

# Follow explicit settings in the calling project or on the command line.
# Upstream's file-local MCU_TARGET defaults to atmega168 even for an unspecific make;
# do not inherit that default accidentally. Prefer MINI_MCU, then MCU, then an
# explicitly exported/command-line MCU_TARGET, and finally Arduino Uno/328P.
ifneq ($(strip $(MINI_MCU)),)
MINI_SELECTED_MCU := $(MINI_MCU)
else ifneq ($(strip $(MCU)),)
MINI_SELECTED_MCU := $(MCU)
else ifneq ($(filter command\ line environment environment\ override,$(origin MCU_TARGET)),)
MINI_SELECTED_MCU := $(MCU_TARGET)
else
MINI_SELECTED_MCU := atmega328p
endif

# Explicit F_CPU wins. Tiny84/85 default to 8 MHz actual CPU clock even when
# upstream Makefile defines an AVR_FREQ for unrelated Arduino boards.
MINI_SOFT_CHIP := $(filter attiny84 attiny85,$(MINI_SELECTED_MCU))
MINI_SELECTED_FREQ := $(if $(F_CPU),$(F_CPU),$(if $(MINI_SOFT_CHIP),8000000,$(if $(AVR_FREQ),$(patsubst %L,%,$(AVR_FREQ)),16000000)))
MINI_SELECTED_BAUD := $(if $(MINI_BAUD_RATE),$(MINI_BAUD_RATE),$(if $(MINI_SOFT_CHIP),auto,$(if $(BAUD_RATE),$(BAUD_RATE),auto)))
MINI_SELECTED_CLOCK := $(if $(MINI_CLOCK_SOURCE),$(MINI_CLOCK_SOURCE),$(if $(CLOCK_SOURCE),$(CLOCK_SOURCE),$(if $(MINI_SOFT_CHIP),internal_rc,external_crystal)))
MINI_SELECTED_BOOT := $(if $(MINI_BOOT_BYTES),$(MINI_BOOT_BYTES),auto)
MINI_EXTRA_FLAGS := $(if $(filter 1 yes true,$(MINI_ALLOW_OVERCLOCK)),--allow-overclock,)
MINI_EXTRA_FLAGS += $(if $(filter 1 yes true,$(MINI_ALLOW_BAUD_ERROR)),--allow-baud-error,)
MINI_EXTRA_FLAGS += $(if $(filter 1 yes true,$(MINI_OLD_AVRDUDE_EEPROM)),--legacy-eeprom-byte-address,)
MINI_PYTHON ?= python3

HELPTEXT += "target mini - Auto-select classic AVR from MINI_MCU/MCU/explicit MCU_TARGET (default ATmega328P)\n"

.PHONY: mini mini_check mini_list mini_test
mini:
	$(MINI_PYTHON) build_mini.py --mcu "$(MINI_SELECTED_MCU)" --f-cpu "$(MINI_SELECTED_FREQ)" --baud "$(MINI_SELECTED_BAUD)" --clock-source "$(MINI_SELECTED_CLOCK)" --boot-bytes "$(MINI_SELECTED_BOOT)" $(MINI_EXTRA_FLAGS)

mini_check: mini
	$(MINI_PYTHON) -m unittest discover -s mini_tests -v

mini_list:
	$(MINI_PYTHON) build_mini.py --list-chips

mini_test:
	$(MINI_PYTHON) -m unittest discover -s mini_tests -v
