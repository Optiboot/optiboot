# Optiboot Mini: features, benefits and validation

Optiboot Mini is an experimental, optional assembly implementation for classic
AVR devices. Its main contribution is a compact combination of EEPROM
transfers, actual fuse/lock reads and explicit protocol bounds checks. The
ATmega328P/PB configuration fits 500 bytes of instructions and a 12-byte footer
inside a 512-byte boot section, leaving 32,256 application bytes.

The existing C implementation already offers a 512-byte bootloader, fast
uploads, fast application startup and broad AVR support. Mini does not claim
to introduce those capabilities to Optiboot or to outperform it in measured
upload speed. The comparison below uses the stock C source defaults at
`e363ecbe2b6542b5db5b4209589f274c87d833a7`; optional C configurations can differ.

| Capability | Existing Optiboot C implementation | Mini addition and practical benefit | Evidence / limit |
|---|---|---|---|
| Compact boot section | Already targets 512 bytes on ATmega328P | Combines EEPROM R/W, fuse/lock reads, sign-on and bounds checks in the same 512-byte section on 328P/PB | Bit-identical 328P reference; 500 B code + 12 B footer; PB hardware tested |
| EEPROM read/write/verify | Available through `SUPPORT_EEPROM` or `BIGBOOT`; disabled by default | Included in Mini's base image; allows serial backup, configuration updates and readback without an ISP session | Writes of 1..16 B per packet; all 1024 B and each write length tested on PB |
| Fuse and lock inspection | `UNIVERSAL` normally returns a dummy byte, apart from extended-address handling | Returns actual low/high/extended fuse and lock bytes through four read-only `UNIVERSAL` opcodes | Compared with direct silicon reads on PB; writes still require ISP |
| Standard sign-on | No `GET_SIGN_ON` payload handler in the default C source | Replies with `AVR STK`, useful for protocol clients and diagnostics | Protocol-model tests and PB hardware |
| Malformed packets and memory bounds | Existing Optiboot framing and programming behavior | Explicit memory-type, length, alignment, overflow and boot-write checks; receive data before committing an erase/write | Negative cases in models and on PB; not power-loss atomicity |
| Device configuration | Broad AVR support already exists | Adds 38 Mini build profiles with reviewed register maps, signatures and memory geometry in one builder | All 38 assemble; only the specified PB configuration has physical evidence |
| Large Flash addressing | Existing C supports extended addresses | Mini implements RAMPZ/ELPM/SPM and 256 KiB bank selection while retaining its checks | Instruction-model coverage; no physical large-AVR validation; 1024 B boot section |
| Clock and baud selection | Existing build options select clock and baud | Builder chooses normal/U2X divisors, checks nominal error and device limits, and records layout, settings and hash in JSON | Divider checks are software checks; no oscillator calibration or fuse changes |
| Small ATtiny experiments | Existing Optiboot has tiny/Virtual Boot options | Four Mini Virtual Boot profiles, vector/seed tools and a dedicated uploader; software UART on ATtiny84/85 | Software tests only; 1024 B reservation, ISP seed and vector relocation required |
| Repeatable validation | Existing project build/test facilities | Adds 70 software tests and a reproducible Windows USB bench runner with verified backups and restoration | 42 named physical checks on one ATmega328PB Nano; detailed report linked below |

## Integration and usage

The original source tree is preserved. Mini sources, device/UART includes and
build helpers live in `optiboot/bootloaders/optiboot/`. `Makefile.custom`
includes optional Mini targets; `optiboot.c`, the main Makefile and stock HEX
images are unchanged. The frozen `mini328` target retains a fixed 328P
reference; `mini` selects the current multi-device implementation.

From `optiboot/bootloaders/optiboot/`:

```sh
make mini                                # ATmega328P, 16 MHz, 115200
make mini MINI_MCU=atmega328pb
make mini328_check                       # fixed reference image
make mini_test                           # software regression suite
```

See the [build and device guide](../../optiboot/bootloaders/optiboot/README_MINI_MULTI.md)
for toolchain requirements, clock limits and all profiles. Hardware-UART ATmega
uploads use standard AVRDUDE with the exact part selection. Experimental
ATtiny uploads require the [Virtual Boot workflow](VIRTUAL_BOOT.md).

## Validation scope and tradeoffs

The [2026-10-30 hardware report](HARDWARE_ATMEGA328PB_2026-10-30.md) records
42 checks on one ATmega328PB Nano: complete application Flash and EEPROM
transfers, actual fuse/lock inspection, invalid-packet and boundary rejection,
50 watchdog resets, 20 external-reset cycles, autonomous startup after a
physical power cycle, and byte-for-byte restoration of the original memories.
The [public runner and probe sources](../../scripts/hardware/nano_usb/README.md)
make that configuration reproducible. User application/EEPROM backups and raw
captures are excluded.

Other MCUs, alternate clocks and electrical margins have not been validated
on silicon. This remains an experimental alternative with fewer customization
options than the C implementation: no startup LED animation or
application-callable SPM service, and no complete AVR061 programmer interface.
Fuse/lock writes and true Chip Erase remain ISP operations. Partial Flash
writes erase unspecified bytes in the page rather than preserving them.
ATtiny Virtual Boot lacks hardware boot-section protection; its software-UART
receiver also lacks the hardware-UART inactivity exit after external reset.
Those constraints are documented in the device and Virtual Boot guides.
