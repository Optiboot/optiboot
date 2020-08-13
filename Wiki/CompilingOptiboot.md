# Introduction #

You can compile Optiboot for platforms that do not have pre-existing .hex files, or you can make modifications to the source code and/or Makefile in order to implement "custom" versions of Optiboot.

This will require some relatively deep knowledge of avr-gcc, Atmel AVR hardware and processor details, Makefiles, and the command line environment.  We HAVE tried to take steps to allow Optiboot to be compiled without installing a full AVR development suite.


# Details #

Optiboot is designed to be compiled with the same version of avr-gcc that is distributed with the Arduino environment: 4.3.2.  This is actually quite an old version of gcc; some effort has been made to allow the compile to procede with new versions of the compiler.  However, since Optiboot MUST compile to a total size of less than 512 bytes, it is very sensitive to changes in the way the compiler does optimizations, or the way it processes optimization options on the compile command.

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
make <platform>  <options>
~~~~
Where <platform> is one of the named chips or boards implemented as normal targets in the makefile (ie "atmega328".) (the order may be reversed.) The implemented <options> include:

  * **AVR\_FREQ=nnnnnn**  --  Use CPU frequency as specified (default: target dependent, but usually 16000000L)

  * **BAUD\_RATE=nnnnn**  --  Use an alternate bitrate (default: usually 115200)<br>
  * **SINGLESPEED=1**  -- Operate the UART in single-speed mode.
  * **UART=n**  -- user UARTn instead of UART0.
  * **SOFT_UART=1**  --  use a bit-banged Software Uart.  Required for chips without a HW UART.
  * **LED=\<portpin\>**  --  Like "LED=B5" or "LED=L5" - which LED to flash.
  * **LED\_START\_FLASHES=n**  -- number of flashes to emit when the bootloader executes (default 3)  Setting this to 0 omits the LED-flashing code and saves some space.
  * **LED\_START\_ON**  -- Turn on the LED when the bootloader starts.  A smaller alternative to LED\_START\_FLASHES.
  * **LED\_DATA\_FLASH=1 ** -- flash the LED in the data receive function as well as at bootloader startup.
  * **BIGBOOT=1** -- include extra features that cause the bootloader to grow to between 512 and 1024 bytes.
  * **SUPPORT\_EEPROM=1** -- try to include EEPROM read/write support, without other BIGBOOT features.
  * **NO\_APP\_SPM=1** -- Omit the code tat allows applications to have Optiboot execute an SPM instruction (for writing to flash.)

For example:

~~~~
make UART=1 LED=A7 AVR_FREQ=20000000L atmega1284
~~~~
Note that many of the board-level targets are implemented using a recursive invocation of make using this options.  For exmaple, the "pro20" target ends up being:

~~~~
 make atmega168 AVR_FREQ=20000000L LED_START_FLASHES=3
~~~~
