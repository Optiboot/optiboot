# Installing optiboot #

There are two aspects to "installing optiboot."  The first problem, which is discussed here, involves getting the optiboot firmware into chips, whether the chips have an older version of a bootloader, or are completely blank.

The second problem is configuring the Arduino IDE to support the optiboot-loaded chips.  This is easy if you're loading up ATmega328x chips, since any 328 chip with optiboot is essentially an Arduino Uno, and you can use the existing Uno configuration.  It is more difficult (and not yet documented) if you're adding a new chip, or putting optiboot on a chip that normally uses a different bootloader.  This is (will be) described at [AddingOptibootChipsToIde](https://github.com/Optiboot/optiboot/blob/master/Wiki/AddingOptibootChipsToIde.md).

Much information about burning optiboot into ATmega chips for use in Arduino can be found in the Arduino forums, especially in [this thread](http://arduino.cc/forum/index.php/topic,64105.0.html)

There are about three main methods of installing optiboot on an otherwise blank AVR chip.

## Installing using the Arduino IDE ##
The Optiboot project here includes ONLY the bootloader, not any support for a chip or chip
options in the "core" Arduino code.   It also includes bootloaders for many chips, of which only a few are likely to be of interest to any one person.  This means that while it is *possible* to "install" Optiboot in the IDE, this is no longer the preferred way to do thing.

 In recent years, since the introduction of installable
3rd party hardware in Arduino 1.6.4, open-source developers have been busy creating
additional "boards" for the IDE that can be installed very easily.  Many of these cores
use Optiboot, or a modified version of Optiboot, as their bootloader, and the preferred
method of installing "XXX processor with Optiboot bootloader" is to ignore this repository
entirely, find a core for that processor, and use the board manager to install it.

Here are some of the well-known and well-used "cores" that use Optiboot:

* https://github.com/MCUdude/MiniCore  (ATmega328, ATmega168, ATmega88, ATmega48 and ATmega8)
* https://github.com/MCUdude/MightyCore  (ATmega8535, ATmega16, ATmega32, ATmega164, ATmega324, ATmega644 and ATmega1284)
* https://github.com/MCUdude/MegaCore  (ATmega64, ATmega128, ATmega640, ATmega1280, ATmega1281, ATmega2560 and ATmega2561)
* https://github.com/SpenceKonde/ATTinyCore  (ATtiny 841, 828, 1634, 87, 167)


If you are using a chip that is "supported" by the Arduino team, or by some other person who has provided files, you can install optiboot (or for that matter, any other bootloader) directly from the Arduino IDE.  This usually has the following steps:

  1. Install the files as directed, usually (for Arduino 1.0+) in a subdirectory of your personal sketch's ../hardware/ directory.  Note that the formats of various files (notably boards.txt) have changed with different versions of the IDE, and you'll need to make sure that the files you have been provided match the version of the IDE you are using.
  1. Connect a device programmer to the ISP connector of the target board, or via wires to an avr chip with appropriate support hardware in a protoboard, as per the instructions associated with that programmer.  It will need to be a programmer of a type supported by the Arduino IDE in the tools/programmer Menu.  You can use a 2nd Arduino with the ArduinoISP sketch as a programmer.  The details are beyond the scope of this document, but are often discussed in the Arduino forums.
  1. Running the Arduino IDE, select the tools/board of the target chip, and the tools/programmer of your programmer, and if necessary the tools/serial port of the programmer.
  1. Select tools/Burn Bootloader

## Installing using a specialized bootloader loader sketch ##
There are a couple of Arduino sketches that have been written to make it easier to reprogram Optiboot (or other SW) into other Arduinos, especially in bulk.  One example is WestfW's [OptiLoader](https://github.com/WestfW/OptiLoader).  Similar programs are available from [Adafruit](https://github.com/adafruit/Standalone-Arduino-AVR-ISP-programmer) and [Nick Gammon](http://www.gammon.com.au/forum/?id=11635).  These typically contain a pre-loaded copy of some version of the bootloader for several different chips (OptiLoader supports ATmega8, ATmega168, ATmega328P, and ATmega328.)  It's very fast and easy IF you have one of the supported targets:

  1. Set up an existing Arduino with an Arduino to ISP connector and load the optiLoader sketch using the standard Arduino IDE.
  1. For each target board, connect the ISP connector and hit the reset button on the optiLoader Arduino, or use the "G" command in the Serial Monitor.  Programming the bootloader takes about 5 seconds, and reports status and information to the Arduino serial port.

## Installing using the optiboot repository Makefile ##
We might assume that you're here at the optiboot code repository because you want to use optiboot on a chip that is NOT supported by another party, but is supposed to be supported by the optiboot in general.  (An example might be the ATmega1280, which normally runs a different bootloader.)

  1. Compile the appropriate binary target (ie "mega1280"), as described on the CompilingOptiboot page.
  1. Connect a device programmer to your target as appropriate.
  1. Edit the Makefile to make the ISPxxx variables correct for your programmer.  Make sure that there is a "target\_isp" make target defined, or create it using the existing targets as examples.
  1. Use a command like "make mega1280\_isp" to burn the bootloader.
  1. You can also use avrdude at the command line, manually.  Burning the bootloader is usually a two-step process:
    * "chip erase" the target and set the fuses and lockbits to allow writing to the bootloader section of the device
    * burn the bootloader at its defined address, and reset the lockbits to prevent accidental overwriting of the bootloader section.