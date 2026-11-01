# Optiboot Mini — multi-device AVR build

This is an **experimental multi-device port** of the existing Optiboot Mini assembly, not a replacement for upstream `optiboot.c`. The default is **ATmega328P / 16 MHz / external crystal / UART 115200, 8N1**, as on the Arduino Uno.

There are **38 profiles: 34 ATmega and four experimental Virtual Boot ATtiny**.
ATmega2560/2561 support four Flash banks; ATtiny84/85 use software UART.
See the [feature comparison](../../../docs/mini/FEATURES.md)
for the additions, tradeoffs and validation scope, and the
[Virtual Boot guide](../../../docs/mini/VIRTUAL_BOOT.md) for ATtiny installation.

The stock `make atmega328` target and standalone `make mini328` target remain available. `make mini` consumes `MINI_MCU`, `MCU` or an explicitly supplied `MCU_TARGET`. It uses `F_CPU` or the project's `AVR_FREQ`; otherwise it defaults to 16 MHz. ATtiny84/85 default to 8 MHz internal RC and 19200 baud even when the parent Makefile has unrelated board defaults. Set `F_CPU=12000000` for ATtiny1634. Device selection is automatic for anyone compiling `optiboot_mini.S` using `-mmcu=<chip>`: `devices/select.inc` reads compiler-defined `__AVR_ATmega...__` and `__AVR_ATtiny...__` macros.

## Examples

```sh
make mini                                   # Uno default: atmega328p / 16 MHz
make mini MINI_MCU=atmega328pb
make mini MCU=atmega168p F_CPU=12000000
make mini MCU_TARGET=atmega32a F_CPU=16000000
make mini MCU=atmega328p F_CPU=8000000 CLOCK_SOURCE=internal_rc
make mini MCU=atmega328p F_CPU=20000000 BAUD_RATE=115200
make mini MCU=atmega328p F_CPU=24000000 MINI_ALLOW_OVERCLOCK=1
make mini MCU=atmega128a F_CPU=16000000  # 1024-byte boot; extended Flash addressing
make mini MCU=atmega1284p F_CPU=20000000 # 1024-byte boot; MCU rated up to 20 MHz
make mini MCU=atmega8
make mini MCU=atmega2560                 # 256 KiB Flash, 1024-byte boot section
make mini MCU=attiny841                  # hardware UART, Virtual Boot
make mini MCU=attiny1634 F_CPU=12000000   # 128-byte Flash transfer groups
make mini MCU=attiny84                   # software UART, 8 MHz / 19200
make mini MCU=attiny85
make mini_list
make mini_test
```

The equivalent standalone invocation is `python3 build_mini.py --mcu atmega324pa --f-cpu 12000000 --baud auto`.

The toolchain requires Clang/LLVM AVR support or AVR-GCC/avr-objcopy, plus `readelf` and `llvm-objdump`. Assemblers that emit local relocations also require `avr-ld` from binutils-avr. The build resolves those relocations at section address zero without linker relaxation; Intel HEX supplies the physical boot address. It generates `output/<mcu>/<mcu>.hex`, `.bin`, `.o`, optional `.elf` and `.json` (build manifest). A failed size or baud check produces an error instead of a misleading ready-to-flash HEX. Use `--verify-baseline` with the standalone builder to require the previous regression-tested Uno binary SHA-256.

## Build support table

| MCU | Flash | Flash page | EEPROM | Current boot section | Build verified | Hardware tested |
|---|---:|---:|---:|---:|---|---|
| ATmega328P | 32 KiB | 128 B | 1 KiB | 512 B | Yes (bit-identical baseline) | No |
| ATmega328 | 32 KiB | 128 B | 1 KiB | 512 B | Yes | No |
| ATmega328PB | 32 KiB | 128 B | 1 KiB | 512 B | Yes | Yes: 16 MHz / 115200, [report](../../../docs/mini/HARDWARE_ATMEGA328PB_2026-10-30.md) |
| ATmega168 / ATmega168P | 16 KiB | 128 B | 512 B | 512 B | Yes | No |
| ATmega88 / ATmega88P | 8 KiB | 64 B | 512 B | 512 B | Yes | No |
| ATmega324PA | 32 KiB | 128 B | 1 KiB | 512 B | Yes | No |
| ATmega164PA | 16 KiB | 128 B | 512 B | 512 B | Yes | No |
| ATmega16 | 16 KiB | 128 B | 512 B | **512 B** | Yes (emulator smoke-tested) | No |
| ATmega32A | 32 KiB | 128 B | 1 KiB | **512 B** | Yes (emulator smoke-tested) | No |
| ATmega64A | 64 KiB | 256 B | 2 KiB | 1024 B (minimum) | Yes | No |
| **ATmega128A** | 128 KiB | 256 B | 4 KiB | **1024 B** (606 B code) | Yes (emulated) | No |
| **ATmega1284 / 1284P** | 128 KiB | 256 B | 4 KiB | **1024 B** (592 B code) | Yes (emulated) | No |
| ATmega8 / 8A / 8515 / 8535 | 8 KiB | 64 B | 512 B | 512 B | Yes | No |
| ATmega162 | 16 KiB | 128 B | 512 B | 512 B | Yes | No |
| ATmega164A / 164P | 16 KiB | 128 B | 512 B | 512 B | Yes | No |
| ATmega324A / 324P / 324PB | 32 KiB | 128 B | 1 KiB | 512 B | Yes | No |
| ATmega644 / 644A / 644P / 644PA | 64 KiB | 256 B | 2 KiB | 1024 B | Yes | No |
| ATmega640 | 64 KiB | 256 B | 4 KiB | 1024 B | Yes | No |
| ATmega1280 / 1281 | 128 KiB | 256 B | 4 KiB | 1024 B | Yes | No |
| ATmega2560 / 2561 | 256 KiB | 256 B | 4 KiB | 1024 B | Yes | No |
| ATtiny841 | 8 KiB | 16 B | 512 B | 1024 B, Virtual Boot | Yes | No |
| ATtiny1634 | 16 KiB | 32 B, erase 4 pages | 256 B | 1024 B, Virtual Boot | Yes | No |
| ATtiny84 / 85 | 8 KiB | 64 B | 512 B | 1024 B, Virtual Boot, software UART | Yes | No |

The ATmega16/32A use `uart/legacy.inc`: `SBIS` polls RXC, FE and UDRE directly in the low I/O space without a temporary `IN` instruction. This preserves UART status checks and saves four bytes, allowing **512-byte** boot sections for both parts. ATmega16 boots at `0x3E00`, ATmega32A at `0x7E00` (when BOOTSZ/BOOTRST fuses are correctly configured). The ATmega64A requires 1024 bytes of boot Flash at minimum; it also has a different SPMCSR location and a 256-byte page. Its 256-byte `PROG_PAGE` form is handled with a distinct code path. The boot address is derived from device Flash size and the actual selected boot section, never from a hardcoded ATmega328P value.

ATmega128A, ATmega1284, and ATmega1284P now have a 17-bit Flash address path. They use the full 1024-byte minimum boot section; the original 512-byte limit does **not** apply. ATmega128A uses the ATmega64A-style USART0 register map and SPMCSR at data address `0x68`; ATmega1284/1284P use modern USART0 and SPMCSR at data address `0x57`. These chips additionally require RAMPZ, ELPM and extended linear-address Intel HEX records. For a byte-for-byte explanation and fuse setup, read [the 128 KiB port guide](README_128K.md). There is no officially named ATmega1284A part: select ATmega1284 or ATmega1284P based on the actual chip/signature.

Chip signatures, UART register maps, SPM location and EEPROM address registers live in `devices/*.inc`. New unrecognised devices are deliberately rejected by `select.inc` until their maps are reviewed.

ATmega2560/2561 use `STK_UNIVERSAL 0x4D` to latch the extended word-address
bank, with RAMPZ/ELPM/SPM and Intel HEX extended-linear-address records.
ATtiny841/1634 use dedicated hardware UART modules. All four ATtiny profiles
require ISP seed installation and application RESET/EE_RDY vector relocation.
ATtiny1634 must receive complete 128-byte groups because one erase affects four
32-byte pages. The dedicated [ATtiny uploader](tiny/flash_tiny.py) handles this.

## Clock and baud-rate configuration

`F_CPU` is the **actual CPU clock**, not merely the oscillator's nominal frequency: take CKDIV8/CLKPR into account. `CLOCK_SOURCE=external_crystal` is the hardware-UART default; ATtiny84/85 default to `internal_rc`. `external_clock` and `internal_rc` are supported as build settings, but **neither this tool nor the bootloader changes fuses or calibrates the oscillator**.

When `BAUD_RATE=auto`, external clocks prefer 115200 if the calculated UART error is within 2.5%, otherwise 57600. Internal RC defaults to 9600 (1 MHz), 19200 (2 MHz), or 38400 (4/8 MHz), with a tighter 1.5% calculated-divider-error limit. The build searches normal (16x) and double-speed (8x) UART modes, selecting the lowest nominal error. Actual internal RC oscillator tolerance, voltage, and temperature drift are additional and **must be measured**.

Software UART has a separate policy: 9600 baud up to 2 MHz, 19200 up to 8 MHz,
38400 up to 16 MHz, and 57600 above that. Its delay loops require 100–750 CPU
cycles per bit; unsupported combinations are rejected. ATtiny84 uses TX PA1 /
RX PA2; ATtiny85 uses TX PB0 / RX PB1, 8N1, without timer interrupts.

Supported build input frequencies include 10, 12, 16 and 20 MHz (subject to each device's datasheet limit). 24, 25 and 28 MHz are deliberate **out-of-spec overclocking experiments** on all listed parts and require `MINI_ALLOW_OVERCLOCK=1` or `--allow-overclock`. This switch is an acknowledgement, not a guarantee that the chip, clock source, or UART works. The datasheet operating-frequency and voltage limits still apply. For legacy ATmega16/32A, calibrated RC frequency selection and OSCCAL values require particular care: the ATmega32A datasheet documents factory calibration loading for its default 1 MHz RC mode. A build option does not select a fuse nor load OSCCAL automatically.

## Protocol and compatibility

The assembly retains the existing Optiboot Mini STK500v1-compatible subset: page write/read of Flash, 1–16-byte EEPROM writes, EEPROM reads, `GET_SIGN_ON`, signature, read-only fuse/lock `UNIVERSAL`, watchdog timeout, and the established classic Optiboot no-op Chip Erase acknowledgement. ATmega2560/2561 additionally support the `UNIVERSAL 0x4D` address-bank command. Read-modify-write of partially written Flash pages is *not* added. Real Chip Erase and fuse writes remain ISP-only. The EEPROM memory addressing assumes the **classic-AVR `avrdude -c arduino` word-address convention**; configure avrdude for the correct exact chip (`-p ...`). Virtual Boot ATtiny uploads require the dedicated uploader and patched application described in the linked guide.

**Verification status:** the old ATmega328P 16 MHz image is bit-identical and its historical software-emulation results carry over to that configuration. ATmega16/32A and ATmega128A/1284/1284P additionally have AVR-opcode-level peripheral smoke tests (not hardware-accurate timing). The **ATmega328PB / 16 MHz / external crystal / 115200 / 512-byte Boot** configuration passed 42 physical checks with real AVRDUDE, including Flash/EEPROM transfers, watchdog/reset stress, protected-region rejection and autonomous startup after a power cycle. See the [hardware report and reproduction steps](../../../docs/mini/HARDWARE_ATMEGA328PB_2026-10-30.md). Other configurations remain unverified on silicon; all ports remain experimental rather than production-qualified.

To avoid bricking a target, check fuse settings, BOOTRST, BOOTSZ, CKDIV8, oscillator, USART wiring, and ISP recovery availability. Consult the chip datasheet for the exact fuse encoding; do not reuse ATmega328P fuse values on legacy devices.

## License and project conventions

The source is derived from the Optiboot project and retains its GPL licensing and attribution; see the repository-root [LICENSE](../../../LICENSE). Keep Mini source comments, test output and documentation in English. Hardware-UART ATmega targets use standard avrdude; the experimental ATtiny workflow uses the dedicated uploader. Integration adds optional includes to `Makefile.custom`; the stock C source and main Makefile are unchanged.

For ATmega64A, verify that the ATmega103 compatibility fuse is disabled when using the modern USART0 and memory map. This port has not been hardware-tested.

## Legacy USART module

`uart/legacy.inc` owns all ATmega16/32A initialization and RX/TX poll sequences.
`uart/modern.inc` preserves the bit-identical ATmega328P reference image, and
`uart/mega64.inc` uses the ATmega64A/ATmega128A extended USART0 register addresses. The main
source uses `MINI_UART_INIT`, `MINI_UART_RX_POLL`, `MINI_UART_TX_POLL`,
`MINI_UART_RECEIVE`, and `MINI_UART_TRANSMIT` assembler macros.

The legacy `UCSRC` and `UBRRH` share I/O address `0x20`; writes use the
`URSEL` bit explicitly (1 for 8N1 UCSRC, 0 for UBRRH). Every build configures
RX/TX and UBRR explicitly, including when entering without hardware reset.
No function or protocol command has been removed to achieve 512 bytes.

### Verified results (simulation only)

| MCU | Instructions | Footer | Boot area | Origin |
|---|---:|---:|---:|---|
| ATmega16 | 500 B | 12 B | 512 B | `0x3E00` |
| ATmega32A | 500 B | 12 B | 512 B | `0x7E00` |
| ATmega328P | 500 B | 12 B | 512 B | `0x7E00` |
| ATmega64A | 568 B | 12 B | 1024 B (hardware minimum) | `0xFC00` |
| ATmega128A | 606 B | 12 B | 1024 B (hardware minimum) | `0x1FC00` |
| ATmega1284 | 592 B | 12 B | 1024 B (hardware minimum) | `0x1FC00` |
| ATmega1284P | 592 B | 12 B | 1024 B (hardware minimum) | `0x1FC00` |

The tests execute AVR instructions for sync, sign-on, signatures, fuse reads,
Flash page programming and verification, EEPROM R/W, RX framing-error
watchdog handling, and power-on application start. They also compare legacy
512-byte UART answers to the prior 1024-byte builds and cover invalid EOP.
This is not a silicon, voltage/timing, or real avrdude hardware test.

## AVRDUDE EEPROM addressing modes (ATmega64A, ATmega128A only)

Historically some AVRDUDE STK500v1 implementations used **byte** EEPROM addresses for ATmega64(A)/128(A), while newer releases unified the **word** address convention across parts. The two modes use the same wire format and cannot be autodetected safely. The default is **word** (recommended with modern AVRDUDE). For a known older host:

```sh
python3 build_mini.py --mcu atmega64a --legacy-eeprom-byte-address
make mini MCU=atmega128a MINI_OLD_AVRDUDE_EEPROM=1
```

The old-host variant only changes EEPROM address interpretation; Flash always takes a word address. **Do not mix host conventions** for EEPROM read/write. The local `mini_tests/test_avrdude_compat_audit.py` suite covers these two explicit modes; physical validation of the legacy devices remains outstanding.
