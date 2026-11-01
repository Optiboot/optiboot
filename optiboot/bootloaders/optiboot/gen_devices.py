#!/usr/bin/env python3
"""Deterministic classic-ATmega hardware profiles for Optiboot Mini.

Build geometry and validation: see README_MINI_MULTI.md and docs/mini/FEATURES.md
at the repository root for configuration-specific physical validation.
All paths remain under optiboot/bootloaders/optiboot/.
"""
from pathlib import Path
import json
import re

ROOT=Path(__file__).resolve().parent
P=ROOT/'devices'
P.mkdir(exist_ok=True)
# chip, FLASH bytes, EEPROM bytes, page bytes, signature, max MHz,
# USART register profile, SRAMEND
# UART styles: modern: base 0xC0; legacy: UCSRC/UBRRH shared I/O 0x20;
# mega64: mixed registers 0x0B/0x0C/0x90/0x95.
devices = [
 ('atmega328p',32768,1024,128,(0x1e,0x95,0x0f),20,'modern',0x08ff),
 ('atmega328',32768,1024,128,(0x1e,0x95,0x14),20,'modern',0x08ff),
 ('atmega328pb',32768,1024,128,(0x1e,0x95,0x16),20,'modern',0x08ff),
 ('atmega168p',16384,512,128,(0x1e,0x94,0x0b),20,'modern',0x04ff),
 ('atmega168',16384,512,128,(0x1e,0x94,0x06),20,'modern',0x04ff),
 ('atmega88p',8192,512,64,(0x1e,0x93,0x0f),20,'modern',0x04ff),
 ('atmega88',8192,512,64,(0x1e,0x93,0x0a),20,'modern',0x04ff),
 ('atmega324pa',32768,1024,128,(0x1e,0x95,0x11),20,'modern',0x08ff),
 ('atmega164pa',16384,512,128,(0x1e,0x94,0x0a),20,'modern',0x04ff),
 ('atmega16',16384,512,128,(0x1e,0x94,0x03),16,'legacy',0x045f),
 ('atmega32a',32768,1024,128,(0x1e,0x95,0x02),16,'legacy',0x085f),
 ('atmega64a',65536,2048,256,(0x1e,0x96,0x02),16,'mega64',0x10ff),
 ('atmega128a',131072,4096,256,(0x1e,0x97,0x02),16,'mega128',0x10ff),
 ('atmega1284',131072,4096,256,(0x1e,0x97,0x06),20,'modern128',0x40ff),
 ('atmega1284p',131072,4096,256,(0x1e,0x97,0x05),20,'modern128',0x40ff),
 # Classical ATmega targets added Oct 30: legacy USART, 64-byte pages
 ('atmega8',8192,512,64,(0x1e,0x93,0x07),16,'legacy',0x045f),
 ('atmega8a',8192,512,64,(0x1e,0x93,0x07),16,'legacy',0x045f),
 ('atmega8515',8192,512,64,(0x1e,0x93,0x06),16,'legacy',0x025f),
 ('atmega8535',8192,512,64,(0x1e,0x93,0x08),16,'legacy',0x025f),
 ('atmega162',16384,512,128,(0x1e,0x94,0x04),16,'legacy',0x04ff),
 # Classic ATmega xx4 variants, contiguous modern USART0 at 0xC0
 ('atmega164p',16384,512,128,(0x1e,0x94,0x0a),20,'modern',0x04ff),
 ('atmega164a',16384,512,128,(0x1e,0x94,0x0f),20,'modern',0x04ff),
 ('atmega324p',32768,1024,128,(0x1e,0x95,0x08),20,'modern',0x08ff),
 ('atmega324a',32768,1024,128,(0x1e,0x95,0x15),20,'modern',0x08ff),
 ('atmega324pb',32768,1024,128,(0x1e,0x95,0x17),20,'modern',0x08ff),
 ('atmega644',65536,2048,256,(0x1e,0x96,0x09),20,'modern',0x10ff),
 ('atmega644p',65536,2048,256,(0x1e,0x96,0x0a),20,'modern',0x10ff),
 ('atmega644a',65536,2048,256,(0x1e,0x96,0x09),20,'modern',0x10ff),
 ('atmega644pa',65536,2048,256,(0x1e,0x96,0x0a),20,'modern',0x10ff),
 # 64/128/256 KiB mega0/mega1 family, USART0 0xC0
 ('atmega640',65536,4096,256,(0x1e,0x96,0x08),16,'modern',0x21ff),
 ('atmega1280',131072,4096,256,(0x1e,0x97,0x03),16,'modern128',0x21ff),
 ('atmega1281',131072,4096,256,(0x1e,0x97,0x04),16,'modern128',0x21ff),
 ('atmega2560',262144,4096,256,(0x1e,0x98,0x01),16,'modern256',0x21ff),
 ('atmega2561',262144,4096,256,(0x1e,0x98,0x02),16,'modern256',0x21ff),
 # Virtual-boot ATtiny: no BOOTRST; application image MUST be vector-patched.
 ('attiny841',8192,512,16,(0x1e,0x93,0x15),16,'tiny841',0x02ff),
 ('attiny1634',16384,256,32,(0x1e,0x94,0x12),12,'tiny1634',0x04ff),
 # Virtual boot with software UART on the two analog-comparator pins.
 ('attiny85',8192,512,64,(0x1e,0x93,0x0b),20,'soft85',0x025f),
 ('attiny84',8192,512,64,(0x1e,0x93,0x0c),20,'soft84',0x025f),
]

jsonout={}
for name,flash,eeprom,page,sig,maxmhz,profile,ramend in devices:
    banked=flash>65536
    large=flash>131072
    # Size-dependent flag MINI_SPM_EXT historically also selects long-branch
    # variants; for the new 256-KiB target it keeps more headroom.
    spm_ext=profile in ('mega64','mega128','modern256')
    style=2 if profile in ('mega64','mega128') else 1 if profile=='legacy' else 3 if profile=='tiny841' else 4 if profile=='tiny1634' else 5 if profile in ('soft85','soft84') else 0
    virtual=profile.startswith('tiny') or profile.startswith('soft')
    default_boot=1024 if flash>=65536 or virtual else 512
    vals={'FLASH_SIZE':flash, 'EEPROM_SIZE':eeprom,'FLASH_PAGE_BYTES':page,
        'SIG0':sig[0],'SIG1':sig[1],'SIG2':sig[2],'SRAM_END':ramend,
        'MINI_UART_STYLE':style, 'MINI_LEGACY_STACK':int(style in (1,2,3,4,5)),
        'MINI_VIRTUAL_BOOT':int(virtual), 'MCUSR_IO':0x35 if profile=='tiny1634' else 0x34,
        'MINI_SPM_EXT':int(spm_ext),
        'MINI_FLASH_BANKED':int(banked), 'MINI_FLASH_128K':int(banked),
        'MINI_FLASH_256K':int(large),
        'SPMCSR_IO':0x37,'SPMCSR_MEM':0x68 if style==2 else 0x57,
        'WDT_IO':0x30 if profile=='tiny1634' else 0x21,
        'EEPROM_EECR_IO':0x1f if style==0 else 0x1c,
        'EEPROM_EEDR_IO':0x20 if style==0 else 0x1d,
        'EEPROM_EEARL_IO':0x21 if style==0 else 0x1e,
        'EEPROM_EEARH_IO':0x22 if style==0 else 0x1f}
    if style in (1,2): vals.update({'UART_A_IO':0x0b,'UART_B_IO':0x0a,'UART_DATA_IO':0x0c,'UART_L_IO':0x09})
    if virtual:vals['MINI_APP_VECTOR_WORD']=14 if profile in ('tiny841','soft84') else 6 if profile=='soft85' else 26
    if banked: vals['RAMPZ_IO']=0x3b
    if style==1: vals.update({'UART_C_IO':0x20,'UART_H_IO':0x20})
    if style==2: vals.update({'UART_C_MEM':0x95,'UART_H_MEM':0x90})
    if style==5: vals.update({'SOFT_PORT_IO':0x18 if profile=='soft85' else 0x1b,
                              'SOFT_DDR_IO':0x17 if profile=='soft85' else 0x1a,
                              'SOFT_PIN_IO':0x16 if profile=='soft85' else 0x19,
                              'SOFT_TX_BIT':0 if profile=='soft85' else 1,
                              'SOFT_RX_BIT':1 if profile=='soft85' else 2})
    io_names=('SIG','_IO','_MEM')
    body=[f'/* Hardware configuration for {name}. Generated by gen_devices.py. */']
    for k,v in vals.items():
        body.append(f'#define {k} {hex(v) if k.startswith("SIG") or k.endswith("_IO") or k.endswith("_MEM") or k=="SRAM_END" else v}')
    body+=['#ifndef BOOT_BYTES',f'#define BOOT_BYTES {default_boot}','#endif', '#define BOOT_START (FLASH_SIZE - BOOT_BYTES)','']
    (P/(name+'.inc')).write_text('\n'.join(body))
    jsonout[name]={'flash':flash,'eeprom':eeprom,'page':page,'sig':list(sig),'max_mhz':maxmhz,'profile':profile,'ramend':ramend,'default_boot':default_boot,'virtual_boot':virtual}
selectors=['/* MCU device selection from compiler -mmcu. */']
for i,(name,*_) in enumerate(devices):
    m=re.fullmatch(r'(atmega|attiny)(\d+)([a-z]*)',name)
    assert m,name
    macro=f'__AVR_{"ATmega" if m.group(1)=="atmega" else "ATtiny"}{m.group(2)}{m.group(3).upper()}__'
    selectors.extend([('#if' if i==0 else '#elif')+f' defined({macro})',f'#include "{name}.inc"'])
selectors+=['#else','#error Unsupported MCU: define a device profile','#endif','']
(P/'select.inc').write_text('\n'.join(selectors))
(ROOT/'devices.json').write_text(json.dumps(jsonout,indent=2)+'\n')
print(f'Generated {len(devices)} AVR device profiles')
