# Introduction #

Optiboot implements a small subset of the  "STK500 communications protocol" (version 1) defined by Atmel in their [Application Note AVR061](http://www.atmel.com/Images/doc2525.pdf)  In order to conserve code space, many of the protocol commands are ignored and receive "lies" rather than correct response codes, leading to the possibility that the bootloader may not operate correctly with all host-side software.  It is known to work with versions of "avrdude" supporting the "-c arduino" programmer type.

Optiboot is designed to reside in the bootloader section, and chip fuses should be set appropriately for the size of optiboot (normally 256 words) (BOOTSZx) and to reset to the bootloader start address (BOOTRST)  Note that BOOTSZx values are chip-dependent, and some of the chips suppored by Optiboot have a 512word minimum bootloader size.)

## Basic Operation ##
  1. On reset, Optiboot starts and reads the reset reason from MCUSR.  For any cause other than "external reset", the application is started immediately.  Otherwise, optiboot attempts to download new application software:
  1. The "start LED" is flashed to indicate that optiboot is running. (which pin is used, and how many times it flashes, is configurable.)
  1. The (configurable) UART is configured and the WDT is enabled with a 1s timeout.
  1. Optiboot attempts to read commands from the (configurable) serial port.  Valid characters will cause the WDT to be reset, and the application flash area to be programmed (hopefully.)
  1. With no valid UART traffic, or after programming, the WDT is allowed to expire, causing the chip to reset.
  1. Since the WDT reset is NOT an 'external reset', the application is started as in (1) - the AVR at the time the application is run has all IO registers except MCUSR and SP in their pristine, reset, state.

# Details #
## Implemented Commands ##
The following core commands are "supported"; they actually do something useful WRT loading code via the bootloader:

|Cmd Name | Value  | Description |
|:--------|-------|:------------|
|STK\_LOAD\_ADDRESS|0x55,'U'| Note: 16bit word address, 128kb flash max.|
|STK\_PROG\_PAGE|0x64,'d'|  Flash only|
|STK\_READ\_PAGE|0x74,'t'|  Flash only|
|STK\_READ\_SIGN|0x75,'u'|  Reads compiled-in signature.|
|STK\_LEAVE\_PROGMODE|0x51,'Q'| Starts application code via WDT reset.|
|STK\_GET\_PARAMETER|0x41,'A'| Supports "minor SW version" and "major SW version"  Returns 3 for all other parameters.|

The following commands will have their arguments correctly read, and produce in a "success" response, but they don't actually do anything:

|Cmd Name | Value |
|:------------- |:-------- |
|STK\_UNIVERSAL|0x56, 'V'| |
|STK\_SET\_DEVICE|0x42, 'B'| |
|STK\_SET\_DEVICE\_EXT|0x45, 'E'| |

All other commands will result in a success response if they are immediate followed by CRC\_EOP (0x20, ' ') (ie they are ignored), or a WDT reset (start applicatin) otherwise.

## Space Saving Techniques ##
In order to save space, Optiboot utilizes a couple of techniques that may be of general interest:

  * The Vector table and normal startup code are omitted by using linker options "-nostdlib -nostartfiles"
  * This requires that main() be declared with attribute OS\_main and manually placed in the .init9 section.
  * and thus main() has to initialize the known-zero register (r1) and SP on chips that don't automatically set SP to end-of-ram.
  * inlining of functions is carefully controlled.