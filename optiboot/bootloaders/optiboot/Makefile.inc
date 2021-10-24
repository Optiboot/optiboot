# Makefile that contains common definitions of variables


# use bash if installed on system else use the shell the script is called with
# to remove error message /bin/sh: 1: arithmetic expression: expecting primary: ""
# when invoking $(shell printf "0x%x" $$(($(1)))) or $(shell printf "0x%x" $$(($(1))))
ifeq ($(shell test -s /bin/bash && echo -n yes), yes)
SHELL=/bin/bash
endif


# function to calculate hex addresses from size given as decimal
#CALC_ADDRESS_IN_HEX = $(printf '%s=sh' "0x%x" $$(($(1))))
#CALC_ADDRESS_IN_HEX = $(shell printf "0x%x" $$(($(1))))
CALC_ADDRESS_IN_HEX = $(shell printf "0x%06x" $$(($(1))))

# preset variables with zero to avoid error messages
BOOT_SECTION_SIZE_KIB  ?= 0
FLASH_SIZE_KIB         ?= 0

# functions to convert KibIByte (KIB) to Byte (B)
BOOT_SECTION_SIZE_B  = ($(BOOT_SECTION_SIZE_KIB) * 1024)
FLASH_SIZE_B         = ($(FLASH_SIZE_KIB) * 1024)

# formulas to calculate addresses of memory sections
BOOT_SEC_OFFSET      = $(call CALC_ADDRESS_IN_HEX, ($(FLASH_SIZE_B)) - ($(BOOT_SECTION_SIZE_B)) )
TEXT_SEC_OFFSET      = $(BOOT_SEC_OFFSET)
VERSION_SEC_OFFSET   = $(call CALC_ADDRESS_IN_HEX, ($(FLASH_SIZE_B)) )
FLASHEND             = $(call CALC_ADDRESS_IN_HEX, ($(FLASH_SIZE_B) - 1) )

