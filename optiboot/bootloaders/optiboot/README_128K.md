# Optiboot Mini for 128 KiB classic AVR parts (2026-10-30)

## Policy and supported parts

The 512-byte size objective applies only to smaller MCUs when their hardware
boot section permits it. For the **ATmega128A**, **ATmega1284**, and
**ATmega1284P**, use the minimum **1024-byte physical boot section**. Do not
sacrifice protocol checks to obtain a 512-byte code image on these parts.
The label **ATmega1284A** is not the supported silicon part name; choose
`atmega1284` (signature `1E 97 06`) or `atmega1284p` (`1E 97 05`).

| MCU | Flash | EEPROM | Page | Signature | Code | Footer | Boot | Boot byte start |
|---|---:|---:|---:|---|---:|---:|---:|---|
| ATmega128A | 128 KiB | 4 KiB | 256 B | 1E 97 02 | 606 B | 12 B | 1024 B | `0x1FC00` |
| ATmega1284 | 128 KiB | 4 KiB | 256 B | 1E 97 06 | 592 B | 12 B | 1024 B | `0x1FC00` |
| ATmega1284P | 128 KiB | 4 KiB | 256 B | 1E 97 05 | 592 B | 12 B | 1024 B | `0x1FC00` |

All sizes are build results for the default 16 MHz / 115200 baud / U2X=1
configuration compiled with LLVM/Clang AVR. The boot section also includes
0xFF padding. The last 12 bytes contain the signature, `AVR STK` sign-on,
and bootloader version. At 16 MHz with `UBRR=16`, nominal UART speed error
is approximately +2.12%, excluding oscillator tolerance.

## How 17-bit addressing works

The STK500v1 `LOAD_ADDRESS` command supplies a 16-bit **word** address,
which represents 128 KiB of Flash as `word_address << 1`.
The conversion generates a 17-bit byte address. Registers `r22:r23` retain
its low 16 bits; `r3` retains its high bit. The latter is written to RAMPZ
(I/O offset `0x3B`, data-space address `0x5B`) before Flash accesses.

- Flash reads use `ELPM r16,Z+`, including the automatic carry into RAMPZ
  while crossing the 64 KiB boundary at byte address `0x10000`.
- Page-buffer fills, page erases and writes use `SPM` with RAMPZ set first.
  SPM addresses are page-aligned (256 B) and validated.
- Flash end-overflow, EEPROM accesses with an upper-bank bit set, and
  writes to the selected boot section are rejected.
- Footer/signature reads switch RAMPZ to bank 1 explicitly.
- Intel HEX outputs include an `04` extended-linear-address record
  (for example `:020000040001F9`) before any data above `0xFFFF`.
- Unlike smaller devices, application dispatch uses an absolute `JMP 0`.

The original 328P/16 MHz/115200-baud `optiboot_mini.S` build remains
bit-identical (SHA-256:
`be531a3f0b7294c6e9f00f4d02a8c5e7ff28d8fb2b7c90cb6501d0294cfae00b`).

## Distinct peripheral configurations

- ATmega128A: ATmega64A-like USART0 (`UCSR0A` I/O `0x0B`, `UCSR0C`
  data `0x95`), SPMCSR at **data `0x68`**, older WDT at I/O `0x21`.
  Ensure its ATmega103 compatibility mode fuse is **disabled**.
- ATmega1284/1284P: USART0 data `0xC0` through `0xC6`, WDTCSR at
  data `0x60`, SPMCSR at data `0x57` (I/O `0x37`).

## BOOTSZ and fuse conditions

For the minimum boot size, set **BOOTSZ1=1, BOOTSZ0=1** (both unprogrammed)
and **BOOTRST=0** (programmed) on the target. These fuses correspond to a
1024-byte boot section starting at byte address `0x1FC00` (word `0xFE00`).
Do not blindly copy complete fuse bytes between devices, especially oscillator,
watchdog and ATmega103 compatibility selections. Preserve ISP recovery.
For larger sections, `--boot-bytes 2048`, `4096` and `8192` are available,
provided the fuses match the selected layout.

## Build and inspect

After installing the add-on in the official Optiboot source tree:

```sh
make mini MCU=atmega128a  F_CPU=16000000 BAUD_RATE=115200
make mini MCU=atmega1284 F_CPU=16000000 BAUD_RATE=115200
make mini MCU=atmega1284p F_CPU=16000000 BAUD_RATE=115200
make mini MCU=atmega1284p F_CPU=20000000 BAUD_RATE=115200
make mini_test
```

Alternatively, run `python3 build_mini.py --mcu atmega128a --boot-bytes 1024`.
The generated `<mcu>.json` includes the exact start address and size.
Use the actual clock rate (including CKDIV8/CLKPR) and configure the
oscillator independently of the bootloader build.

## Test evidence and limitations

Assembly, boot layout, signature/footer, extended Intel HEX and default
328P binary identity were verified. A limited AVR instruction simulator
exercised STK500v1 sync, sign-on, signature, fuse read, both Flash banks,
ELPM across the 64 KiB boundary, 256-byte page writing, last application
page, boot protection, 4 KiB EEPROM read/write, invalid EOP preserving the
existing Flash page, invalid memory type, end overflow and reset-to-app.

**No actual AVR silicon, UART timing or real avrdude communication was
available for validation.** The emulator models selected instructions and
peripherals, not exact per-cycle electrical timing or every chip erratum.
Keep an ISP programmer connected for initial flashing and recovery; do not
use these experimental images for deployment without testing on hardware.

Official datasheets:

- [ATmega128A](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-8151-8-bit-AVR-ATmega128A_Datasheet.pdf)
- [ATmega1284P and family](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-42719-ATmega1284P_Datasheet.pdf)
