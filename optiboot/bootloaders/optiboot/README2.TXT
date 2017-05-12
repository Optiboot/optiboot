Changes to the project have been made in order to build a bootloader for use in the Receiver Development Platform. 
The bootloader for this project is compatible with AVRDude run with the following parameters:

C:\> avrdude -c arduino -p m328p -b 9600 

Other Properties:

 o Capable of bootloading after either an external (Reset pin) interrupt, or after a power-on reset.
 o Only supports a baud rate of 9600
 o Compiles under AtmelStudio 7 by importing the following files into a project:
     - boot.h
	 - defs.h
	 - optiboot.c
	 - pin_defs.h
	 - stk500.h
 o Loads at byte address (.text=) 0x7800 (word address 0x3C00), for compatibility with an ATmega328p 
   processor with fuse settings as follows: 
		BODLEVEL = 2V7
		RSTDISBL = [ ]
		DWEN = [ ]
		SPIEN = [X]
		WDTON = [ ]
		EESAVE = [ ]
		BOOTSZ = 1024W_3C00
		BOOTRST = [X]
		CKDIV8 = [ ]
		CKOUT = [ ]
		SUT_CKSEL = INTRCOSC_8MHZ_6CK_14CK_0MS

		EXTENDED = 0xFD (valid)
		HIGH = 0xDA (valid)
		LOW = 0xC2 (valid)
 o Compiles to 736 bytes at optimization level -O1
 o Can be safely debugged in AtmelStudio with optimiation level -O0
