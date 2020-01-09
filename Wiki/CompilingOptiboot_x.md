# Introduction #

Optiboot_x, the bootloader for Mega0, Tiny0, and Tiny1 architecture AVR chips, uses a slightly different build procedure than Optiboot for the older AVR chips.  The major difference is that you must provide the name of the makefile in the make command, and may require that you specify the location of the appropriate PACKS directory.
~~~
export PACKS=<directory>
make -f Makefile.mega0 <target> <options>
~~~


# Details #

Optiboot is designed to be compiled with the same version of avr-gcc that is distributed with the Arduino MegaAVR Core. (Currently an Arduino-compiled avr-gcc 7.3), or the latest CLI tools from Microchip (a version of avr-gcc 5.4 (yes, older than the Arduino compiler))

In all cases, Optiboot compiles directly from the command line using "make", rather than from within any IDE.  It is currently designed to used compilers installed in one of three different places:

  * Local Development Environment - you have WinAVR (on Windows), CrossPack (on Mac), or an avr-gcc package (linux) installed on your system, with appropriate development tools somewhere in your path.
  * You have Arduino installed, and are trying to compile Optiboot from its home directory within the Arduino directory structure (hardware/arduino/bootloaders/optiboot/)  The Arduino app includes a pretty complete set of compiler tools, and these can be used to compile optiboot without installing a separate toolchain. (as of Version 1.5 of the Arduino IDE, a much smaller set of tools is included, and this method doesn't work any more.)
  * You have downloaded the Arduino Source code, which also includes (binary) copies of avr-gcc toolchains, and a source directory containing the Optiboot source code.

WARNING: some targets don't compile with "make" 3.81 or 3.82 due to bug in it.  See https://github.com/Optiboot/optiboot/issues/106


# Compile Options #

The Optiboot build procedure has been set up to allow a large set of customization options.  The general format of a build command is:

~~~~
make help
~~~~
The "make help" command should output a list of possible options and targets, probably more complete and up-to-date than this documentation.

~~~~
make -f Makefile.mega0 <target>  <options>
~~~~
Where \<target\> is one of the named chips or boards implemented as normal targets in the makefile (ie "drazzy402") (the order may be reversed.) The implemented \<options\> include:

 * **TIMEOUT=n** -- Sets the bootloader timeout to n seconds (n=1, 2, 4, or 8)  Longer timeouts are useful on boards/chips that don't implement auto-reset, or that need power-cycled to reset.
  * **BAUD\_RATE=nnnnn**  --  Use an alternate bitrate (default: 115200)<br>
  * **UARTTX=<portPin>**  -- use the designated pin for UART TX.  This implies a particular UART and the RX pin as well.  The pins that can be used are chip dependent.
  * **LED=\<portPin\>**  --  Like "LED=B5" or "LED=L5" - which LED to flash.
  * **LED\_START\_FLASHES=n**  -- number of flashes to emit when the bootloader executes (default 3)  Setting this to 0 omits the LED-flashing code and saves some space.
  * **LED\_START\_ON**  -- Turn on the LED when the bootloader starts.  A smaller alternative to LED\_START\_FLASHES.
  * **LED\_DATA\_FLASH=1** -- flash the LED in the data receive function as well as at bootloader startup.
  * **BIGBOOT=1** -- include extra features that cause the bootloader to grow to between 512 and 1024 bytes.  Not implemented.
  * **NO\_APP\_SPM=1** -- Omit the code tat allows applications to have Optiboot execute an SPM instruction (for writing to flash.)
  * **AVR\_FREQ=nnnnnn**  --  Not applicable.
  * **SINGLESPEED=1**  -- Not applicable.
  * **SUPPORT\_EEPROM=1** -- Not applicable.  EEPROM is always supported.
  * **SOFT_UART=1**  --  Not applicable.  All chips have UARTS


For example:

~~~~
make -f Makefile.mega0 UARTTX=A1 LED=A7 atmega4809
make -f Makefile.mega0 curiosity4809
~~~~
Note that many of the board-level targets are implemented using a recursive invocation of make using this options.  For exmaple, the "drazzy402" target ends up being:

~~~~
 make -f  Makefile.mega0 optiboot_attiny402.hex UARTTX=A1 TIMEOUT=8 LED=A7
~~~~

# Mega-0 Differences #
The mega-0 and tiny-0/1 architecture is significantly different (and "nicer") than previous ATmega/ATtiny chips, especially when it comes to implementing a bootloader.  Here are some of the more relevant changes:

 1. The bootloader starts at 0x0, and the Application start after the bootloader. <br>This means that the bootloader binary is the same for chips in the same family that have different flash memory size (eg ATmega1609 and ATmega4809.)
 2. Commonality of binaries.<br>These chips have much better cross-chip compatibility than previous AVR generations.  Common peripherals are identical and at the same addresses.  Combined with (1), this means that the same bootloader binary should run on any of the 14, 20, and 24pin tiny0 or tiny1 chips, if the same UARTTX ports are in use.
 3. Chosing a UART.<br>In addition to having multiple UARTs, the mega-0 has a pin multiplexing ability that can redirect the functionality to two different sets of pins for each UART.  Specifying the port and pin of the TX signal uniquely identfies both the UART and pinmux settings.
 4. Clock speed.<br>Mega-0 chips normally boot to an internal clock that is accurate and fast enough for uart communications, regardless of whether "normal operation" of that board uses a different clock rate.  Optiboot runs off of this internal clock after figuring out the rate
 1. New NVM structure.<br>Flash and EEPROM are treated very similarly, so that support for both fits easily in the 512byte bootloader.
