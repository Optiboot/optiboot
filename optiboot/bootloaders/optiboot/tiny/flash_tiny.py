#!/usr/bin/env python3
"""EXPERIMENTAL Virtual Boot STK500v1 host for ATtiny841/ATtiny1634/ATtiny84/ATtiny85.

Never use without an ISP recovery programmer. Before uploading an application,
run patch_vectors.py on its HEX. The bootloader must be preinstalled with ISP,
SELFPRGEN enabled, and reset redirected to the bootloader.

ATtiny1634 erases four 32-byte pages at once; its safe transfer size is 128,
not the physical 32-byte page size often advertised by AVRDUDE.

This script is NOT tested against real hardware. Requires pyserial on host.
"""
import argparse
from pathlib import Path
import sys

from patch_vectors import TARGETS, read_hex, decode_rjmp

SYNC=b'\x30\x20'

class Stk500Mini:
    def __init__(self,ser):self.ser=ser
    def transact(self,request,count=0):
        self.ser.write(request)
        self.ser.flush()
        prefix=self.ser.read(1)
        if prefix!=b'\x14':
            raise RuntimeError(f'Expected STK_INSYNC 0x14, got {prefix.hex() or "timeout"}')
        data=self.ser.read(count)
        if len(data)!=count:
            raise RuntimeError(f'Truncated STK500 reply ({len(data)} of {count})')
        tail=self.ser.read(1)
        if tail!=b'\x10':
            raise RuntimeError(f'Expected STK_OK 0x10, got {tail.hex() or "timeout"}')
        return data
    def sync(self): self.transact(SYNC)
    def signature(self):return self.transact(b'\x75\x20',3)
    def address(self,byteaddr):
        if byteaddr%2:raise ValueError('Flash address must be even')
        word=byteaddr//2
        if word>65535:raise ValueError('Address too large for Virtual Boot AVR')
        self.transact(bytes((0x55,word&255,word>>8,0x20)))
    def write_flash(self,byteaddr,payload):
        self.address(byteaddr)
        n=len(payload)
        self.transact(bytes((0x64,n>>8,n&255,ord('F')))+payload+b'\x20')
    def read_flash(self,byteaddr,n):
        self.address(byteaddr)
        return self.transact(bytes((0x74,n>>8,n&255,ord('F'),0x20)),n)
    def leave(self): self.transact(b'\x51\x20')


def upload(link,image,mcu,verify=True):
    t=TARGETS[mcu]
    boot=t['flash']-t['boot']
    group=128 if mcu=='attiny1634' else 16 if mcu=='attiny841' else 64
    if not image:raise ValueError('Empty Intel HEX')
    if max(image)>=boot or min(image)<0:
        raise ValueError('Image enters reserved boot region')
    # Preflight before any write: the reset vector must point to the bootloader.
    reset=image.get(0,-1)|(image.get(1,-1)<<8)
    if 0 not in image or 1 not in image or decode_rjmp(0,reset,t['flash']//2)!=boot//2:
        raise ValueError('Reset vector has not been patched to bootloader: run patch_vectors.py first')
    # Preflight relocated EE_RDY vector, present in all patcher outputs.
    if 2*t['vector'] not in image or 2*t['vector']+1 not in image:
        raise ValueError('Missing relocated EEPROM-ready vector')
    groups=sorted({(a//group)*group for a in image})
    if any(base+group>boot for base in groups):raise ValueError('Upload crosses boot boundary')
    for i,base in enumerate(groups,1):
        data=bytes(image.get(base+j,0xff) for j in range(group))
        link.write_flash(base,data)
        if verify:
            readback=link.read_flash(base,group)
            if readback!=data:raise RuntimeError(f'Flash verification FAILED at 0x{base:04X}')
    return len(groups),group


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--mcu',choices=TARGETS,required=True)
    p.add_argument('--port',required=True)
    p.add_argument('--baud',type=int,help='Default: 19200 for software UART, 115200 for tiny841/1634')
    p.add_argument('--timeout',type=float,default=1.0)
    p.add_argument('--reset-dtr',action='store_true',help='Pulse DTR on USB-UART adapter before initial sync')
    p.add_argument('--ack-experimental',action='store_true',help='I accept the risk of reset vector corruption and have ISP recovery')
    p.add_argument('--no-verify',action='store_true')
    p.add_argument('patched_hex',type=Path)
    args=p.parse_args()
    if not args.ack_experimental:
        p.error('Refusing to flash an experimental virtual bootloader: use --ack-experimental after arranging ISP recovery')
    image=read_hex(args.patched_hex)
    try:import serial
    except ImportError as e:raise RuntimeError('pyserial required: python3 -m pip install pyserial') from e
    import time
    baud=args.baud if args.baud is not None else (19200 if args.mcu in ('attiny84','attiny85') else 115200)
    with serial.Serial(args.port,baud,timeout=args.timeout,write_timeout=args.timeout) as ser:
        if args.reset_dtr:
            ser.dtr=False;time.sleep(0.05);ser.dtr=True;time.sleep(0.1);ser.dtr=False
        link=Stk500Mini(ser)
        link.sync()
        sig=link.signature()
        expected=bytes({'attiny841':(0x1e,0x93,0x15),'attiny1634':(0x1e,0x94,0x12),
                        'attiny84':(0x1e,0x93,0x0c),'attiny85':(0x1e,0x93,0x0b)}[args.mcu])
        if sig!=expected:raise RuntimeError(f'Signature mismatch: read {sig.hex()}, expected {expected.hex()}')
        count,step=upload(link,image,args.mcu,verify=not args.no_verify)
        link.leave()
        print(f'EXPERIMENTAL upload finished: {count} x {step}-byte groups, '+('verified' if not args.no_verify else 'not verified'))

if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError) as exc:
        print('ERROR:',exc,file=sys.stderr)
        sys.exit(1)
