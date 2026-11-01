#!/usr/bin/env python3
"""Build the standalone Optiboot Mini AVR assembly in the stock Optiboot tree.

This is intentionally independent of the upstream optiboot.c and its linker
script: the bootloader is hand-positioned in a 512-byte data image at 0x7E00.
It should not be used for chips/clock configurations other than ATmega328P 16MHz.
"""
import argparse
import hashlib
from pathlib import Path
import re
import shutil
import subprocess
import sys

HERE = Path(__file__).resolve().parent
SOURCE = HERE / "optiboot_mini328.S"
REFERENCE = HERE / "optiboot_mini328.sha256"
BASE = 0x7E00
SIZE = 512
END = BASE + SIZE
FOOTER = bytes((0x1E, 0x95, 0x0F)) + b"AVR STK" + bytes((3, 8))


def toolchain(selection):
    options = {
        "clang": ("clang", "llvm-objcopy", ["-target", "avr", "-mmcu=atmega328p"]),
        "gcc": ("avr-gcc", "avr-objcopy", ["-mmcu=atmega328p"]),
    }
    for name in ([selection] if selection != "auto" else ["clang", "gcc"]):
        compiler, objcopy, flags = options[name]
        if shutil.which(compiler) and shutil.which(objcopy):
            return compiler, objcopy, flags
    raise RuntimeError("Install clang + llvm-objcopy, or avr-gcc + avr-objcopy")


def make_hex(binary):
    rows = []
    for offset in range(0, len(binary), 16):
        payload = binary[offset:offset + 16]
        address = BASE + offset
        record = bytes((len(payload), address >> 8, address & 255, 0)) + payload
        rows.append(":" + (record + bytes(((-sum(record)) & 255,))).hex().upper())
    rows.append(":00000001FF")
    return "\n".join(rows) + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--toolchain", choices=["auto", "clang", "gcc"], default="auto")
    ap.add_argument("--verify-reference", action="store_true",
                    help="Require byte-for-byte identity with last regression-tested image")
    args = ap.parse_args()
    compiler, objcopy, flags = toolchain(args.toolchain)
    obj = HERE / "optiboot_mini328.o"
    binpath = HERE / "optiboot_mini328.bin"
    hexpath = HERE / "optiboot_mini328.hex"
    subprocess.run([compiler, *flags, "-c", str(SOURCE), "-o", str(obj)], check=True)
    image_obj = obj
    if shutil.which("readelf"):
        reloc = subprocess.check_output(["readelf", "-r", str(obj)], text=True)
        if "There are no relocations in this file" not in reloc:
            if not shutil.which("avr-ld"):
                raise RuntimeError("Local AVR relocations require avr-ld (binutils-avr); do not flash an unlinked object")
            image_obj = HERE / "optiboot_mini328.elf"
            # Keep offsets and instruction sizes; Intel HEX supplies the boot address.
            subprocess.run(["avr-ld", "-m", "avr5", "-Ttext=0", "-e", "_start",
                            str(obj), "-o", str(image_obj)], check=True)
            reloc = subprocess.check_output(["readelf", "-r", str(image_obj)], text=True)
            if "There are no relocations in this file" not in reloc:
                raise RuntimeError("Linked image has unresolved relocations; do not flash this image")
    else:
        raise RuntimeError("Missing readelf: cannot verify AVR relocations safely")
    subprocess.run([objcopy, "-O", "binary", "-j", ".text", str(image_obj), str(binpath)], check=True)
    binary = binpath.read_bytes()
    if len(binary) != SIZE:
        raise RuntimeError(f"Expected {SIZE} bytes, got {len(binary)}")
    if not binary.endswith(FOOTER):
        raise RuntimeError("The signature, GET_SIGN_ON and version footer is not at 0x7FF4")
    if binary[:2] != bytes.fromhex("F894"):
        raise RuntimeError("Unexpected entry instructions (expected CLI at 0x7E00)")
    digest = hashlib.sha256(binary).hexdigest()
    if args.verify_reference:
        expected = REFERENCE.read_text(encoding="utf-8").split()[0].strip()
        if digest != expected:
            raise RuntimeError(f"Regression: SHA256 {digest} != {expected}")
    hexpath.write_text(make_hex(binary), encoding="ascii")
    print(f"Image: {binpath.name}, {len(binary)} bytes at 0x{BASE:04X}-0x{END - 1:04X}")
    print(f"HEX:   {hexpath.name}")
    print(f"SHA256: {digest}")
    if args.verify_reference:
        print("REFERENCE: OK")


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, subprocess.CalledProcessError, OSError) as err:
        print(f"ERROR: {err}", file=sys.stderr)
        sys.exit(1)
