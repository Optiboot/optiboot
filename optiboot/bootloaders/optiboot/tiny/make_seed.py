#!/usr/bin/env python3
"""Create an ISP-only, sparse virtual-boot seed Intel HEX for ATtiny841/1634/84/85.

Seed includes bootloader at top of Flash, reset RJMP to boot, and a temporary
EEPROM-ready RJMP to boot. User application must be patched before UART upload.
Do NOT use this as a serial upload image. Requires SELFPRGEN fuse and ISP.
"""
import argparse
from pathlib import Path
import sys
from patch_vectors import TARGETS,make_hex,rjmp

def make_seed(blob,mcu):
    t=TARGETS[mcu]
    if len(blob)!=t['boot']:
        raise ValueError(f'Expected boot image {t["boot"]} bytes; got {len(blob)}')
    boot=t['flash']-t['boot'];words=t['flash']//2
    image={boot+i:b for i,b in enumerate(blob)}
    for frm in (0,t['vector']):
        inst=rjmp(frm,boot//2,words)
        image[frm*2]=inst&255
        image[frm*2+1]=inst>>8
    return image

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--mcu',required=True,choices=TARGETS)
    p.add_argument('boot_bin',type=Path)
    p.add_argument('output_hex',type=Path)
    args=p.parse_args()
    img=make_seed(args.boot_bin.read_bytes(),args.mcu)
    args.output_hex.write_text(make_hex(img),encoding='ascii')
    print(f'Created experimental ISP seed {args.mcu}: {args.output_hex}')
    print('WARNING: not a serial-upload image. Enable SELFPRGEN fuse and install via ISP only; hardware not tested.')
if __name__=='__main__':
    try:main()
    except (ValueError,OSError) as exc:
        print('ERROR:',exc,file=sys.stderr);sys.exit(1)
