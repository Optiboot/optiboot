# Optiboot Mini 328P — optional assembly build target

The files in this directory are installed into an existing checkout of the [official Optiboot project](https://github.com/Optiboot/optiboot). They add an independent `mini328` build target without replacing or editing `optiboot.c`, the main `Makefile`, `stk500.h`, `boot_opt.h`, or build targets for other AVR microcontrollers.

## Build and verify

Run from `optiboot/bootloaders/optiboot/`:

```sh
make mini328
make mini328_check
```

The build uses `build_mini328.py` to assemble `optiboot_mini328.S` and emit a 512-byte BIN plus an Intel HEX file with absolute addresses `0x7E00–0x7FFF`. `mini328_check` also verifies the BIN SHA-256 against the regression-tested reference.

Install Clang/LLVM with AVR support or AVR-GCC/avr-objcopy, and `readelf`.
Assemblers that emit local relocations also require `avr-ld` from binutils-avr.
Linking preserves section offsets and instruction sizes; Intel HEX supplies the
boot address. Unresolved relocations are rejected before extracting the binary.

Only the ATmega328P with a 16 MHz clock and USART0 at 115200 baud is supported by this frozen reference build. For the maintained multi-device target, use [README_MINI_MULTI.md](README_MINI_MULTI.md). It does not inherit all customization options from the original C implementation.

## Added features compared with the usual Arduino Uno firmware

- EEPROM read/write/verify through the existing STK500v1 `READ_PAGE` / `PROG_PAGE` commands. EEPROM writes accept 1–16 bytes per request.
- Read-only access to low, high, and extended fuses and lock bits using supported `STK_UNIVERSAL` read opcodes.
- `STK_GET_SIGN_ON` response containing the standard `AVR STK` string.
- Additional checks on packet length, memory type, address overflow and the protected boot region.

Normal Flash upload and verify through `avrdude -c arduino` are retained. No custom desktop programmer is required. The comparison is with the common *prebuilt* Optiboot firmware, not every optional configuration of upstream Optiboot.

For sample commands, wire formats, compatibility limitations, and details of the `UNIVERSAL` subset, see [README_MINI328_AVRDUDE.md](README_MINI328_AVRDUDE.md).

## Important limitations

- This is a compatible subset of STK500v1, **not the complete AVR061 programmer firmware**.
- Fuse and lock bits are read-only; writing them requires ISP.
- `CHIP_ERASE` retains the classic Optiboot compatibility acknowledgement but does **not** perform true ISP chip erase.
- Incomplete Flash writes erase an entire 128-byte page; bytes outside the supplied fragment are not preserved. Read-modify-write is not implemented.
- There is no LED start-up animation or application-callable SPM entry.
- The image has been checked with software emulation and byte-for-byte build comparisons, **not on physical hardware**.

The separate multi-device target has [physical results for ATmega328PB](../../../docs/mini/HARDWARE_ATMEGA328PB_2026-10-30.md). That evidence does not qualify this frozen ATmega328P image on silicon.

Do not replace the stock `optiboot_atmega328.hex` without making a separate backup. Verify BOOTSZ/BOOTRST, the clock source, and the actual target part before flashing through an external ISP programmer.

This code is derived from Optiboot; see the assembly header and the repository-root [LICENSE](../../../LICENSE) for licensing and attribution.

## Maintenance and language policy

All future Mini328 source comments, Makefile/Python messages, documentation, test descriptions, and release notes must be written **in English**. See [CONTRIBUTING_MINI328.md](CONTRIBUTING_MINI328.md).
