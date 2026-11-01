# Contributing to Optiboot Mini

Keep Mini an optional assembly target alongside the stock C implementation.
Preserve the existing repository layout and board targets. Write Mini code,
comments, documentation, messages and test descriptions in English.

## Compatibility and regression checks

1. Preserve the hardware-UART ATmega `avrdude -c arduino` interface and documented
   Flash page semantics. ATtiny Virtual Boot has a separate uploader and vector
   contract; do not imply interchangeable host support.
2. Retain EEPROM transfers, read-only fuse/lock `UNIVERSAL` operations, sign-on,
   watchdog handling, EEPROM/SPM interlocks and address/boot-boundary checks.
3. Keep the default ATmega328P image within its 512-byte boot section at
   `0x7E00..0x7FFF`. Respect each other MCU's physical boot-section minimum.
4. Review signature, register map, Flash/EEPROM geometry, reset behavior and
   datasheet clock limits before adding a device profile. Regenerate the device
   includes with `python3 gen_devices.py` when changing its profile inputs.
5. Compare BIN/HEX addresses, size and SHA-256 after assembly changes. Explain
   intentional changes to a reference image rather than silently replacing the
   baseline hash. The fixed `mini328` target is a separate regression reference.
6. Keep build, instruction-model and physical test evidence distinct. A passing
   software model does not qualify UART timing, silicon errata or power loss.

## Minimum checks

Run from `optiboot/bootloaders/optiboot/`, with the toolchain described in
[README_MINI_MULTI.md](README_MINI_MULTI.md):

```sh
make mini328_check
python3 build_mini.py --verify-baseline
make mini_test
```

The self-contained `mini_tests/` suite covers 38 builds and selected
instruction-level protocol/peripheral models. No external handoff archive is
required. `mini_tests/fixtures/` holds earlier legacy images and symbol objects
used for response-equivalence regression tests.

For changes to shared Makefile integration, also build the stock `atmega328`
target and compare its output with an unmodified checkout using the same
toolchain and options.

## Hardware evidence and documentation

The [ATmega328PB report](../../../docs/mini/HARDWARE_ATMEGA328PB_2026-10-30.md)
records 42 successful physical checks on one Nano at 16 MHz / 115200 baud.
The [reproduction guide](../../../scripts/hardware/nano_usb/README.md) provides
the runner, probes, backups and restoration procedure for that exact build.
Other profiles still need hardware validation. Keep board memory backups and
raw captures local; they are not test fixtures for publication.

When changing a protocol operation, document its packet/response format,
address units, host command, bounds and failure behavior. Update
[README_MINI328_AVRDUDE.md](README_MINI328_AVRDUDE.md) for the fixed 328P
reference, [README_MINI_MULTI.md](README_MINI_MULTI.md) for the current target,
and the [feature comparison](../../../docs/mini/FEATURES.md) when claims change.
Record exact clock, baud, boot layout, image hash and test method for new
hardware evidence.
