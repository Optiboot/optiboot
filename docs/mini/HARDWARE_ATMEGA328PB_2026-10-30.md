# ATmega328PB Nano hardware validation

Date: 2026-10-30. One connected Nano with a physical ATmega328PB was tested
through its USB serial interface. **All 42 named hardware checks passed**,
including a physical power cycle. The original application and complete EEPROM
were restored and verified; signature, fuses and lock bits were verified unchanged.
The separate `-RestoreFromDirectory -Run` recovery command was then executed
successfully and verified the complete original Flash and EEPROM again.

## Tested configuration

| Item | Value |
|---|---|
| Firmware source base | `ccfc7f9470d9823b945092fb2e2905c193c75ec8` |
| MCU / signature | ATmega328PB / `1E9516` |
| Configured CPU clock | 16 MHz, external crystal |
| Host UART | 115200 baud, 8N1, USART0 |
| Flash / page / EEPROM | 32768 / 128 / 1024 bytes |
| Boot section / code | 512 / 500 bytes, start `0x7E00` |
| Application region | 32256 bytes, `0x0000..0x7DFF` |
| LFUSE / HFUSE / EFUSE | `FF / DE / FD` |
| Lock byte | raw `CF`, implemented bits `0F` |
| Host | Windows, PowerShell 7, AVRDUDE 8.0-arduino.1 |
| Probe build | AVR-GCC in WSL, standalone C, shared ATmega328P register definitions |

Installed boot BIN SHA-256:

```text
149ebc0347815b42daaa20defce83342462306e9ddd3f15c63eeb59baafd70d1
```

The boot section was read through the running bootloader and matched the
known build before testing. The probes read the actual signature and fuse/
lock bytes directly from the MCU; they do not rely only on protocol replies.
Rebuilding after the documentation/comment updates produced the same boot
binary hash; no firmware instructions were changed for this validation.

## Physical results

| Area | Result |
|---|---|
| Full application Flash write/read/verify | PASS, all 32256 bytes |
| Full EEPROM write/read/verify | PASS, all 1024 bytes |
| EEPROM block lengths | PASS, every length 1 through 16 at the upper boundary |
| Flash partial-page behavior | PASS, a two-byte write clears unspecified bytes; full page restored |
| Protocol | PASS, sync, sign-on, version, signature, 256-byte reads and legacy zero encoding |
| Memory boundaries | PASS, last application page and reads across the app/Boot boundary |
| Invalid packets | PASS, unknown memory, invalid lengths, unaligned writes, overflow, bad EOP and incomplete payloads |
| Boot protection | PASS, programming request rejected, boot unchanged, application LPM boot reads blocked |
| Application handoff | PASS, initial r1 and MCUSR zero; watchdog disabled |
| Application watchdog stress | PASS, 50 deliberate 15 ms watchdog resets |
| External-reset stress | PASS, 20 DTR reset/sync/signature/leave cycles |
| UART and reconnects | PASS, application echo, BREAK recovery and three consecutive AVRDUDE sessions |
| Physical power cycle | PASS, autonomous application execution before the host opens the port |
| Final restoration | PASS, all 32768 Flash bytes, all EEPROM bytes, signature, fuses and lock bits match the original snapshot |

The maximum host-observed restart delay during the 50 watchdog cycles was
125 ms. This includes USB and host scheduling delays and is not a measurement
of the watchdog oscillator.

For power-cycle validation the running application first cleared its test
record and stopped updating it. The serial handle stayed open until physical
USB removal. After reconnecting USB the host waited four seconds before
opening the port. The first application invocation had stored a valid handoff
record and advanced the 500 ms counter three times. The captured first-entry
reset indication was `04` (BORF); the later host query observed `08` (WDRF).
This verifies autonomous startup after the user-controlled power cycle; it
does not assume the application retains PORF after the bootloader clears
MCUSR or starts the application through its watchdog.

The cold-start probe uses EEPROM addresses `1014..1023` temporarily. The
entire original EEPROM was restored afterward. Hardware boot-lock write
protection was decoded from real lock reads; the test does not bypass the
firmware's boot-address guard for a destructive lock-bit experiment.

During development, too-strict cold-start assertions failed and the runner's
failure path restored and verified all original data. The final cold-start
criterion uses saved autonomous uptime instead. AVRDUDE also recovered an
initial sync retry when switching away from a streaming diagnostic. These
observations are not treated as firmware corruption or hidden test passes.

## Software regression

From `optiboot/bootloaders/optiboot/`:

```sh
python3 -m unittest discover -s mini_tests -v
```

Result: **70 tests passed**, in 26.990 seconds. This includes builds for all
38 profiles and protocol/emulator checks for the other device families.
Those results do not establish physical validation of the other profiles.

## Reproduction and scope

The [runner and instructions](../../scripts/hardware/nano_usb/README.md)
include both standalone probe sources, a bounded native-process helper,
fresh backup verification, the hardware cases and interruption recovery.
The default mode is read-only; `-Run -ColdStart` requests the complete bench
test. Generated images, original application/EEPROM backups and raw local
captures are excluded from version control.

This result applies to one board and the exact configuration/hash above.
Voltage/temperature margins, oscillator accuracy, endurance, interrupted
erase/write atomicity and other MCUs were not qualified. Build manifests
retain conservative default verification flags; this report records the
configuration-specific physical evidence.
