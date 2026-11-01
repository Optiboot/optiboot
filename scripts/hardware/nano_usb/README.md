# ATmega328PB Nano USB hardware tests

This is a repeatable bench test for **Optiboot Mini on an ATmega328PB Nano,
16 MHz, USART0 at 115200 baud, and a 512-byte boot section at `0x7E00`**.
It uses real AVRDUDE transfers and a serial protocol client. It does not
simulate a chip or qualify the other device profiles.

The pinned boot BIN SHA-256 is
`149ebc0347815b42daaa20defce83342462306e9ddd3f15c63eeb59baafd70d1`.
The runner checks this against a fresh read of the installed boot section,
along with signature `1E9516`, fuses `FF / DE / FD`, and lock byte `CF`.
It refuses a different configuration before writing test data.

## Requirements

- Windows with PowerShell 7.2 or later.
- AVRDUDE 8 with the `m328pb` and `arduino` definitions.
- AVR-GCC, `avr-objcopy`, and `avr-objdump`, either on PATH or in WSL.
- A Nano connected by its own USB cable, with the correct serial driver.
  Close other applications using its COM port.

Both standalone C probes use the ATmega328P register definitions shared by
this ATmega328PB configuration. The diagnostics obtain the actual device
signature from silicon. No Arduino core, Python package or GUI is required.

## Run

From the repository root, set the COM port and the installed AVRDUDE path.
For the Arduino-packaged version used in the recorded run:

```powershell
$avrTool = Join-Path $env:LOCALAPPDATA 'Arduino15\packages\arduino\tools\avrdude\8.0.0-arduino1'
$testArgs = @{
    Port = 'COM3'
    AvrdudePath = Join-Path $avrTool 'bin\avrdude.exe'
    UseWslToolchain = $true
}

# Build the probes and make verified backups, without writing test data.
.\scripts\hardware\nano_usb\test_nano_usb.ps1 @testArgs

# Run the complete bench test, including the physical power-cycle step.
.\scripts\hardware\nano_usb\test_nano_usb.ps1 @testArgs -Run -ColdStart
```

The runner finds `../etc/avrdude.conf` beside the executable when available.
Use `-AvrdudeConfig <absolute-path>` to override it. Omit
`-UseWslToolchain` when the AVR build tools are on the Windows PATH.

Each invocation creates a **new** output directory under the checkout's
ignored `.worktree/` directory. `-OutputDirectory` can select a new directory
there or outside the checkout. An existing output directory is refused.
Output contains the board's current application and EEPROM: keep it local
and do not commit or publish it.

When `READY_FOR_POWER_CYCLE` appears, unplug **all** power to the Nano,
including any ISP power, then reconnect only its USB cable to the same
computer USB port so that the COM-port assignment stays the same. The test waits
for the port to disappear and return, then queries the running application.
Before this step an application command clears and verifies an entry record
and uptime counter at EEPROM addresses `1014..1023`, then stops updating
them in the running application. The serial handle stays open until physical
USB removal, so closing it cannot reset the armed application first.
The next application invocation records its handoff state and advances a
500 ms counter. Later invocations preserve that evidence.
After USB returns, the host waits four seconds before opening the port and
requires at least three saved ticks, zero initial `r1`/`MCUSR`, and a disabled
watchdog. This confirms autonomous application startup before the host
connection, even if opening the serial driver resets the AVR again.
The complete original EEPROM is restored afterward.

Do not require PORF in the application: the bootloader clears MCUSR and can
wait in UART mode after an external-reset indication, then start the
application through its watchdog. The observed handoff can therefore contain
WDRF after a physical power cycle.
A 120-second timeout applies to each port transition.

For an unattended run, omit `-ColdStart`. The JSON result explicitly records
that physical cold start was not requested. That run does not cover power-on
behavior. Creating `cancel-cold-start.txt` in the output directory cancels a
pending power-cycle step and enters restoration.

## Coverage

- Read and independently verify complete Flash, EEPROM, signature, fuses
  and lock bits before the first write.
- Write and verify all **32,256 application bytes**, including the final
  page before Boot, and all **1,024 EEPROM bytes** using AVRDUDE.
- Run direct fuse/lock diagnostics from the application; require three stable
  reports and confirm blocked application reads of the boot section.
- Check STK500v1 sync, sign-on, version, signature, 256-byte reads, the legacy
  zero-length read encoding, and reads across the application/Boot boundary.
- Check EEPROM writes of every size 1 through 16 at its upper boundary.
- Check a two-byte Flash write, including the documented erasure of the
  unspecified bytes in that page, then restore the complete test page.
- Reject unknown memory types, invalid lengths, unaligned writes,
  overflowing addresses, out-of-range reads/writes, invalid terminators and
  incomplete Flash/EEPROM packets. Verify the complete test memories again.
- Reject a boot-page programming packet whose payload is identical to the
  installed page. Verify that the boot section is unchanged. The test does
  not bypass the firmware guard to destructively probe hardware lock bits.
- Check the unsupported UNIVERSAL zero response and the existing no-op
  Chip Erase acknowledgement.
- Observe initial `r1`, `MCUSR`, reset reason in `r2`, and disabled watchdog
  before the application deliberately enables it. The watchdog probe does
  **not** disable the watchdog on startup.
- Perform **50 application-triggered 15 ms watchdog resets**,
  **20 DTR reset / sync / leave cycles**, UART echo and BREAK recovery,
  and three consecutive real AVRDUDE sessions.
- Optionally verify autonomous startup after a physical power cycle, then restore the original
  application and EEPROM and verify **all 32,768 Flash bytes**, EEPROM,
  signature, fuses and lock bits against the initial backup.

`usb-test-result.json` records individual results, restoration status and
cold-start coverage. The watchdog restart time is observed by the host and
includes USB and host scheduling delays; it is not an oscillator measurement.

## Restoration and interruption recovery

Once test writes begin, the runner attempts restoration in `finally`, even
when a test fails. It never changes fuses or lock bits and does not include
the boot section in its normal Flash write images. Restoration requires a
connected, responsive board. Keep the backup directory if a cable is removed
or the process is interrupted.

To restore from that original directory after reconnecting the same board:

```powershell
.\scripts\hardware\nano_usb\test_nano_usb.ps1 @testArgs `
    -RestoreFromDirectory 'C:\path\to\the\saved\output' -Run
```

This checks the device configuration, restores the saved application and
EEPROM, and verifies the complete memories. It preserves the `before-*`
backup files. Do not start a new test to recover a board whose original
application has not yet been restored.

The suite is functional validation of one configuration. It does not measure
voltage or temperature margins, oscillator accuracy, Flash endurance,
power-loss atomicity during erase/write, or other MCU variants.
