#!/usr/bin/env python3
"""Build the Optiboot Mini multi-device AVR assembly (all output is English)."""
from __future__ import annotations
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
DEVICES = json.loads((ROOT / 'devices.json').read_text())
SOURCE = ROOT / 'optiboot_mini.S'
BASELINE_SHA256 = 'be531a3f0b7294c6e9f00f4d02a8c5e7ff28d8fb2b7c90cb6501d0294cfae00b'
SUPPORTED_CLOCKS = ('external_crystal', 'external_clock', 'internal_rc')


def best_baud(f_cpu, baud):
    choices=[]
    for multiplier, u2x in ((16,0),(8,1)):
        nominal=f_cpu/(multiplier*baud)-1
        for d in {int(math.floor(nominal)), int(math.ceil(nominal)),int(round(nominal))}:
            if 0<=d<=4095:
                actual=f_cpu/(multiplier*(d+1))
                error=100*(actual-baud)/baud
                choices.append((abs(error),u2x,d,actual,error))
    if not choices:
        raise ValueError('BAUD_RATE is out of the USART divider range')
    # For an equally good error, prefer normal speed, which has wider sampling margin.
    choices.sort(key=lambda x:(x[0],x[1]))
    _,u2x,divider,actual,error=choices[0]
    return {'u2x':u2x,'ubrr':divider,'actual_baud':actual,'baud_error_percent':error}


def make_hex(binary, base):
    """Intel HEX including extended linear addresses up to 256-KiB."""
    if base + len(binary) > 262144:
        raise ValueError('HEX image extends beyond 256 KiB')
    lines = []
    last_upper = 0
    for i in range(0,len(binary),16):
        data = binary[i:i+16]
        addr = base+i
        upper=addr>>16
        if upper != last_upper:
            ela=bytes((2,0,0,4,(upper>>8)&255,upper&255))
            lines.append(':'+ela.hex().upper()+format((-sum(ela))&255,'02X'))
            last_upper = upper
        record=bytes((len(data),(addr>>8)&255,addr&255,0))+data
        lines.append(':'+record.hex().upper()+format((-sum(record))&255,'02X'))
    return '\n'.join(lines+[':00000001FF',''])


def compiler_tools(selection):
    for name,cc,oc,flags in [
            ('clang','clang','llvm-objcopy',['-target','avr']),
            ('gcc','avr-gcc','avr-objcopy',[])]:
        if selection not in ('auto',name):continue
        if shutil.which(cc) and shutil.which(oc):
            return name,cc,oc,flags
    raise RuntimeError('Missing AVR toolchain: install clang+llvm-objcopy or avr-gcc+avr-objcopy')


def build(args):
    mcu=args.mcu.lower().strip()
    if mcu not in DEVICES:
        raise ValueError(f'Unsupported MCU {mcu!r}; use --list-chips')
    dev=DEVICES[mcu]
    if args.legacy_eeprom_byte_address and mcu not in ('atmega64a','atmega128a'):
        raise ValueError('Old AVRDUDE EEPROM byte addressing is only supported for ATmega64A/ATmega128A')
    f_cpu=int(args.f_cpu) if args.f_cpu is not None else int(os.environ.get('F_CPU', 8000000 if dev['profile'] in ('soft85','soft84') else 16000000))
    if f_cpu<100000 or f_cpu>28000000:
        raise ValueError('F_CPU must be between 100000 and 28000000 Hz')
    clock_source=args.clock_source or os.environ.get('CLOCK_SOURCE', 'internal_rc' if dev['profile'] in ('soft85','soft84') else 'external_crystal')
    if clock_source=='internal_rc' and f_cpu not in (1000000,2000000,4000000,8000000):
        raise ValueError('internal_rc requires an actual CPU clock of 1, 2, 4 or 8 MHz; check fuses/CLKPR')
    if f_cpu > dev['max_mhz']*1000000:
        if not args.allow_overclock:
            raise ValueError(f'{mcu}: rated maximum is {dev["max_mhz"]} MHz. Use --allow-overclock only for deliberate, unverified overclocking.')
        print(f'WARNING: {mcu} at {f_cpu/1e6:g} MHz exceeds rated {dev["max_mhz"]} MHz; clock/voltage/UART reliability NOT guaranteed.',file=sys.stderr)
    soft_serial = dev['profile'] in ('soft85','soft84')
    if args.baud == 'auto' and soft_serial:
        baud = 9600 if f_cpu <= 2000000 else 19200 if f_cpu <= 8000000 else 38400 if f_cpu <= 16000000 else 57600
    elif args.baud == 'auto':
        if clock_source=='internal_rc':
            # conservative nominal defaults: RC error and temperature drift are additional
            baud=9600 if f_cpu<=1000000 else (19200 if f_cpu<=2000000 else 38400)
        else:
            baud=115200 if abs(best_baud(f_cpu,115200)['baud_error_percent'])<=2.5 else 57600
    else:
        baud=int(args.baud)
        if baud<=0:raise ValueError('BAUD_RATE must be positive')
    uart=best_baud(f_cpu,baud) if not soft_serial else {
        'u2x':0,'ubrr':0,'actual_baud':f_cpu/round(f_cpu/baud),
        'baud_error_percent':100*(f_cpu/round(f_cpu/baud)-baud)/baud}
    if soft_serial and not (100 <= round(f_cpu/baud) <= 750):
        raise ValueError('Software UART needs 100..750 CPU cycles per bit; choose another baud/clock')
    error_lim=1.5 if clock_source=='internal_rc' else 2.5
    if abs(uart['baud_error_percent'])>error_lim and not args.allow_baud_error:
        raise ValueError(f'UART error {uart["baud_error_percent"]:+.2f}% exceeds {error_lim:.1f}% limit; change baud or --allow-baud-error')
    if clock_source=='internal_rc':
        print('NOTICE: internal RC uses fuse/CLKPR settings made outside this bootloader; RC drift requires calibration and real UART testing.',file=sys.stderr)
    tcname,cc,oc,tcflags=compiler_tools(args.toolchain)
    output=Path(args.output_dir).resolve() if args.output_dir else ROOT/'output'/mcu
    output.mkdir(parents=True,exist_ok=True)
    virtual = bool(dev.get('virtual_boot'))
    # Hardware BOOTSZ1:0 supports exactly four sizes (factor 2 each).
    # MAX is 2KiB on 8/16KiB AVRs, 4KiB on 32KiB AVRs, and
    # 8KiB on the supported 64/128/256KiB AVRs.  Flash//2 is NOT a valid
    # substitute: it silently allowed non-existent 4/8KiB fuse selections.
    max_boot = 2048 if dev['flash'] <= 16384 else (4096 if dev['flash'] <= 32768 else 8192)
    hardware_boot_sizes = [max_boot//8, max_boot//4, max_boot//2, max_boot]
    capacities = [dev['default_boot']] if virtual else [b for b in hardware_boot_sizes if b >= dev['default_boot']]
    if args.boot_bytes != 'auto':
        size=int(args.boot_bytes)
        if size not in capacities:
            raise ValueError(f'BOOT_BYTES={size} not allowed for {mcu}; choices are {capacities}')
        capacities=[size]
    flags=[*tcflags, '-mmcu='+mcu, f'-DF_CPU={f_cpu}UL',f'-DBAUD_RATE={baud}UL', f'-DMINI_U2X={uart["u2x"]}',f'-DMINI_UBRR={uart["ubrr"]}', '-I',str(ROOT)]
    if args.legacy_eeprom_byte_address:
        flags.append('-DMINI_OLD_AVRDUDE_EEPROM=1')
    last_message=''
    for boot_bytes in capacities:
        obj=output/(mcu+'.o')
        image=output/(mcu+'.bin')
        cmd=[cc,*flags,f'-DBOOT_BYTES={boot_bytes}','-c',str(SOURCE),'-o',str(obj)]
        completed=subprocess.run(cmd,capture_output=True,text=True)
        if completed.returncode:
            last_message=completed.stderr.strip()
            if args.boot_bytes == 'auto':
                continue
            raise RuntimeError(last_message)
        # Some assemblers leave local branches for the linker to resolve.
        image_obj = obj
        if shutil.which('readelf'):
            rel=subprocess.run(['readelf','-r',str(obj)],capture_output=True,text=True,check=True).stdout
            if 'There are no relocations in this file' not in rel:
                if not shutil.which('avr-ld'):
                    raise RuntimeError('Local AVR relocations require avr-ld (binutils-avr); do not flash an unlinked object')
                image_obj = output/(mcu+'.elf')
                if soft_serial or mcu == 'attiny841':
                    arch = 'avr25'
                elif virtual:
                    arch = 'avr35'
                elif dev['flash'] > 131072:
                    arch = 'avr6'
                elif dev['flash'] > 65536:
                    arch = 'avr51'
                else:
                    arch = 'avr4' if dev['flash'] <= 8192 else 'avr5'
                # Preserve section-relative offsets and instruction sizes. HEX supplies
                # the physical boot address; linker relaxation must remain disabled.
                subprocess.run(['avr-ld','-m',arch,'-Ttext=0','-e','_start',str(obj),'-o',str(image_obj)],
                               check=True,capture_output=True,text=True)
                rel=subprocess.run(['readelf','-r',str(image_obj)],capture_output=True,text=True,check=True).stdout
                if 'There are no relocations in this file' not in rel:
                    raise RuntimeError('Linked image has unresolved relocations; do not flash this image')
        else:
            raise RuntimeError('Missing readelf: cannot verify AVR relocations safely')
        subprocess.run([oc,'-O','binary','-j','.text',str(image_obj),str(image)],check=True,capture_output=True)
        b=image.read_bytes()
        if len(b)>boot_bytes:
            last_message=f'{len(b)} bytes exceed {boot_bytes} bytes of boot space'
            continue
        if len(b)!=boot_bytes:
            last_message=f'Generated {len(b)} bytes instead of {boot_bytes} bytes'
            continue
        footer=bytes(dev['sig'])+b'AVR STK'+bytes((3,8))
        if b[-12:]!=footer:
            raise RuntimeError(f'{mcu} boot footer does not match expected signature/version')
        if b[:2]!=bytes((0xF8,0x94)):
            raise RuntimeError('Unexpected boot entry, expected CLI opcode')
        base=dev['flash']-boot_bytes
        if base % dev['page']:
            raise RuntimeError('Boot start is not page-aligned')
        sha=hashlib.sha256(b).hexdigest()
        if args.verify_baseline and sha!=BASELINE_SHA256:
            raise RuntimeError('The baseline 328P 16MHz 115200 image changed: '+sha)
        (output/(mcu+'.hex')).write_text(make_hex(b,base))
        # count machine-code bytes by the assembler's _code_end symbol
        sym=subprocess.check_output(['llvm-objdump','-t',str(obj)],text=True)
        m=re.search(r'^([0-9a-fA-F]{8,16})\s+g\s+\.text\s+\S+\s+_code_end$',sym,re.M)
        code_bytes=int(m.group(1),16) if m else None
        if code_bytes is not None and code_bytes+12>boot_bytes:
            raise RuntimeError('Code section overlaps footer')
        result={'mcu':mcu,'flash_bytes':dev['flash'],'eeprom_bytes':dev['eeprom'],'flash_page_bytes':dev['page'],
            'f_cpu_hz':f_cpu,'clock_source':clock_source,'baud_requested':baud,**uart,'toolchain':tcname,
            'boot_bytes':boot_bytes,'boot_start_hex':f'0x{base:04X}','code_bytes':code_bytes,
            'boot_size_max_bytes':None if virtual else max_boot,
            'bootsz1_0_bits':None if virtual else f'{(hardware_boot_sizes.index(boot_bytes) ^ 3):02b}',
            'bootrst_programmed_required':not virtual,
            'virtual_boot_vector_patch_required':virtual,
            'selfprgen_fuse_programmed_required':virtual,
            'virtual_boot_flash_transfer_bytes':128 if mcu=='attiny1634' else 16 if mcu=='attiny841' else 64 if soft_serial else None,
            'soft_uart':soft_serial, 'soft_uart_tx_pin':'PB0' if mcu=='attiny85' else 'PA1' if mcu=='attiny84' else None,
            'soft_uart_rx_pin':'PB1' if mcu=='attiny85' else 'PA2' if mcu=='attiny84' else None,
            'hardware_boot_fuse_supported':not virtual,
            'eeprom_host_addressing':'byte' if args.legacy_eeprom_byte_address else 'word',
            'signature_hex':''.join(f'{v:02X}' for v in dev['sig']),'bin_sha256':sha,
            'production_ready':False,
            'silicon_tested':False,'avrdude_hardware_tested':False,'overclock':f_cpu>dev['max_mhz']*1000000}
        (output/(mcu+'.json')).write_text(json.dumps(result,indent=2)+'\n')
        if virtual:print('WARNING: Virtual Boot ATtiny cannot start from reset without patching application reset and EE_RDY vectors. Do not flash .hex as a complete image.',file=sys.stderr)
        print(f'OK {mcu}: {len(b)} bytes (code {code_bytes}) at 0x{base:04X} | {f_cpu/1e6:g} MHz / {baud} baud, U2X={uart["u2x"]}, UBRR={uart["ubrr"]}, error={uart["baud_error_percent"]:+.2f}%')
        print(f'    HEX {output/(mcu+".hex")} SHA256 {sha}')
        return result
    raise RuntimeError(f'Could not fit {mcu} in permitted boot sections: {last_message}')


def main():
    a=argparse.ArgumentParser(description=__doc__)
    a.add_argument('--mcu',default=os.environ.get('MINI_MCU',os.environ.get('MCU','atmega328p')))
    a.add_argument('--f-cpu',type=int)
    a.add_argument('--clock-source',choices=SUPPORTED_CLOCKS)
    a.add_argument('--baud',default=os.environ.get('BAUD_RATE','auto'))
    a.add_argument('--boot-bytes',default='auto')
    a.add_argument('--allow-overclock',action='store_true')
    a.add_argument('--allow-baud-error',action='store_true')
    a.add_argument('--legacy-eeprom-byte-address',action='store_true',
        help='ATmega64A/128A only: match old AVRDUDE STK500v1 EEPROM byte addressing (2022 and earlier clients may differ)')
    a.add_argument('--toolchain',choices=['auto','clang','gcc'],default='auto')
    a.add_argument('--output-dir')
    a.add_argument('--verify-baseline',action='store_true')
    a.add_argument('--list-chips',action='store_true')
    args=a.parse_args()
    if args.list_chips:
        for chip,info in DEVICES.items():print(f'{chip:14} {info["flash"]//1024:2} KiB flash  {info["page"]:3}B page  {info["eeprom"]:4}B EEPROM  {info["max_mhz"]}MHz rated')
        return
    build(args)
if __name__=='__main__':
    try:main()
    except (ValueError,RuntimeError,subprocess.CalledProcessError,OSError) as e:
        print('ERROR:',e,file=sys.stderr);sys.exit(1)
