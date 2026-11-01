# Optiboot Mini 328P — AVRDUDE support and added STK500v1 features

**Target:** ATmega328P at 16 MHz, USART0 at 115200 baud, 8N1, 512-byte boot area (`0x7E00–0x7FFF`).
**Build:** `make mini328` in the existing Optiboot source tree.
**Host:** standard AVRDUDE using `-c arduino -p m328p`; no AVRDUDE or Arduino IDE patch is required.

> **Experimental status:** The image has been tested with an AVR instruction/peripheral simulator and modeled AVRDUDE exchanges. It has **not** been tested on a physical ATmega328P with a real UART connection to AVRDUDE. Keep an ISP programmer available for recovery.

## Added capabilities at a glance

The comparison is with the **usual prebuilt Arduino Uno/Nano Optiboot firmware**. This does not imply that all configurations of the original Optiboot source lack these options: upstream can be built with optional EEPROM support.

| Operation | Typical prebuilt Uno Optiboot | Optiboot Mini 328P | AVRDUDE operation |
| --- | --- | --- | --- |
| Flash program/read/verify | Supported | Preserved | `flash:w`, `flash:r`, `flash:v` |
| EEPROM read (1 KiB) | Normally unavailable | **Added** | `eeprom:r` |
| EEPROM write | Normally unavailable | **Added**, 1–16 bytes per packet | `eeprom:w` |
| EEPROM verify | Normally unavailable | **Added** | `eeprom:v` |
| Low/high/extended fuse read | Normally unavailable | **Added**, read-only | `lfuse:r`, `hfuse:r`, `efuse:r` |
| Lock-bit read | Normally unavailable | **Added**, read-only | `lock:r` |
| Standard sign-on (`AVR STK`) | Not provided by common minimal HEX | **Added** | `STK_GET_SIGN_ON` clients |
| ATmega328P signature | Supported | Preserved | `STK_READ_SIGN` during connection |
| True ISP Chip Erase | Not available | Not available | Do **not** treat `-e` as a real chip erase |
| Fuse/lock programming | Not available | Not available | Requires an external ISP programmer |

Additional changes strengthen validation of 16-bit block lengths, memory type, address overflow and bounds, and protect the boot area. These are correctness changes, not new host-side operations.

## AVRDUDE command examples

The examples assume `/dev/ttyUSB0`. Adjust this for the actual port (`/dev/ttyACM0`, `COM5`, etc.) and confirm that the chip really has the expected bootloader and baud rate.

### Read the full EEPROM to a raw binary backup

```sh
avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 \
  -U eeprom:r:eeprom-backup.bin:r
```

### Write EEPROM from a raw binary file

```sh
avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 \
  -U eeprom:w:eeprom-backup.bin:r
```

### Verify EEPROM against the backup, without writing

```sh
avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 \
  -U eeprom:v:eeprom-backup.bin:r
```

### Read fuse and lock bytes (read-only)

```sh
avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 \
  -U lfuse:r:-:h -U hfuse:r:-:h \
  -U efuse:r:-:h -U lock:r:-:h
```

### Standard Flash upload remains unchanged

```sh
avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 \
  -U flash:w:sketch.hex:i
```

The new EEPROM and fuse/lock examples are **expected to work based on the simulated protocol**, but remain hardware-validation steps rather than completed board tests. Back up EEPROM and chip configuration with a known-good ISP programmer before experimenting with EEPROM writes.

## Exact supported STK500v1 command subset

| Opcode | Command | Behavior in this image |
| --- | --- | --- |
| `0x30` | `STK_GET_SYNC` | `14 10` (`INSYNC`, `OK`) |
| `0x31` | `STK_GET_SIGN_ON` | `14` + seven ASCII bytes `AVR STK` + `10` |
| `0x40` | `STK_SET_PARAMETER` | Consumes two parameter bytes; acknowledges without applying them |
| `0x41` | `STK_GET_PARAMETER` | Reports SW major 8 and minor 3; other values follow the Optiboot-compatible fallback |
| `0x42` | `STK_SET_DEVICE` | Consumes and ignores the usual 20 parameter bytes |
| `0x45` | `STK_SET_DEVICE_EXT` | Consumes and ignores five parameter bytes |
| `0x50` | `STK_ENTER_PROGMODE` | Compatible acknowledgement |
| `0x51` | `STK_LEAVE_PROGMODE` | Acknowledgement followed by watchdog-assisted application startup |
| `0x55` | `STK_LOAD_ADDRESS` | Accepts word address, converts it to a byte address and checks overflow |
| `0x56` | `STK_UNIVERSAL` | Reads **only** supported fuse/lock opcodes; unsupported ISP operations return a dummy zero |
| `0x64` | `STK_PROG_PAGE` | `F`: aligned, even Flash blocks 2–128 bytes; `E`: EEPROM blocks 1–16 bytes |
| `0x74` | `STK_READ_PAGE` | `F`/`E`: length 1–256 bytes, with legacy zero-length encoding for 256 bytes; checks bounds |
| `0x75` | `STK_READ_SIGN` | `14 1E 95 0F 10` |

Most successful replies follow `STK_INSYNC=0x14`, optional payload, then `STK_OK=0x10`. `CRC_EOP=0x20` is a packet terminator, **not an actual computed CRC**.

### Implemented `STK_UNIVERSAL` fuse and lock reads

The host sends four ISP command bytes followed by `CRC_EOP`. The first two command bytes select the register. The extra command bytes must still be supplied so framing remains correct.

| ISP opcode prefix | Register | AVRDUDE memory name |
| --- | --- | --- |
| `50 00` | Low fuse | `lfuse` |
| `58 00` | High fuse | `hfuse` |
| `50 08` | Extended fuse | `efuse` |
| `58 08` | Lock bits | `lock` |

Successful output is `14`, one actual data byte, `10`. An unsupported `UNIVERSAL` request returns a dummy `0x00`; a positive acknowledgement **does not mean that an ISP write or chip erase took place**. The three device-signature bytes are provided separately by `STK_READ_SIGN` rather than arbitrary `UNIVERSAL` signature opcodes.

### EEPROM addressing and block limits

For the ATmega328P and `-c arduino` programmer profile, the AVRDUDE version inspected during development passes EEPROM addresses to `STK_LOAD_ADDRESS` in *word-address units* for classic AVR devices. This bootloader multiplies the received address by two, matching that host behavior. Do not assume that every third-party STK500v1 implementation follows this convention.

- EEPROM physical byte address range: `0x0000–0x03FF` (1024 bytes).
- EEPROM write block size accepted by this bootloader: 1–16 bytes. The inspected AVRDUDE implementation uses 16-byte blocks for classic AVR bootloaders with byte-oriented EEPROM pages.
- Incoming EEPROM data is buffered on the AVR stack, then committed only after a valid end marker is received. A power failure during the actual EEPROM writes can still leave a partially updated block.

## Compatibility notes and limitations

- **Not a full Atmel STK500 programmer:** this is an Optiboot-style compatible STK500v1 subset running *on the target chip*, not an ISP programming device for a different AVR.
- **True Chip Erase is not implemented.** As in the original minimal Optiboot, some Chip Erase requests are acknowledged without actually erasing the device. Do not use AVRDUDE's `-e` option as proof of a full erase.
- **Fuse and lock programming is not implemented.** Use an external ISP programmer for writes.
- **Arbitrary ISP operations are not executed** by `UNIVERSAL`; only the four listed read operations are supported.
- **Unsupported commands and invalid packet ends** retain the bootloader's established Optiboot compatibility/error-recovery behavior. They do not implement every AVR061 error response.
- **Partial Flash page writes erase the entire physical 128-byte page.** Unspecified old bytes are not preserved, consistent with original Optiboot. Read-modify-write is intentionally not added.
- No optional startup LED animation or application-callable SPM service.
- The version returned by `GET_PARAMETER` is the compatibility value 8.3; it does not identify this custom assembly build as a separately released upstream Optiboot version.
- Fixed target: ATmega328P at 16 MHz with USART0, 115200 baud and 512-byte boot area. Not intended as a drop-in assembly replacement for other AVR families or clocks.

## Testing and references

- Rebuilt BIN (512 bytes) SHA-256: `be531a3f0b7294c6e9f00f4d02a8c5e7ff28d8fb2b7c90cb6501d0294cfae00b`.
- Software-level AVR emulation and modeled protocol tests have exercised all user-application Flash pages, the full 1-KiB EEPROM, mixed commands, fuse/lock reads and negative cases. Certain differences from full AVR061 are recorded as expected deviations.
- **No physical ATmega328P validation or real AVRDUDE-to-chip transfer has been performed.** Clock, serial electrical timing, watchdog and SPM/EEPROM interactions must be verified on hardware before deployment.
- Upstream sources: [Optiboot](https://github.com/Optiboot/optiboot), particularly `optiboot.c` and `stk500.h`.
- Host implementation: [AVRDUDE](https://github.com/avrdudes/avrdude), especially `src/stk500.c`, `src/arduino.c`, and `src/avr.c`.
- Hardware documentation: Atmel AVR061 (STK500v1), Microchip ATmega328P datasheet, and AVR Instruction Set Manual.

**Maintenance rule:** preserve the standard `avrdude -c arduino` interface, existing Flash semantics and the added EEPROM/fuse-read functionality. Keep the 512-byte boundary and rerun regression tests before accepting changes.
