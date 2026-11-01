#!/usr/bin/env python3
"""Experimental AVR Virtual Boot application vector patcher (ATtiny841/1634/84/85).

Use on the APPLICATION HEX before sending it with the serial Optiboot Mini.
This does not install the bootloader: an ISP programmer is required to install
bootloader code in the top 1 KiB, plus the reset-vector RJMP at address 0.

Application requirements: 16-bit RJMP from reset to application start, a free
EEPROM-ready interrupt vector (overwritten explicitly), and no application
bytes in the reserved 1-KiB boot region.

NOT hardware tested; preserve an ISP recovery path. GPL-2.0-or-later.
"""
import argparse
from pathlib import Path
import sys

TARGETS = {
    'attiny841': {'flash':8192, 'boot':1024, 'vector':14},
    'attiny1634': {'flash':16384, 'boot':1024, 'vector':26},
    'attiny85': {'flash':8192, 'boot':1024, 'vector':6},
    'attiny84': {'flash':8192, 'boot':1024, 'vector':14},
}


def read_hex(path):
    """Read Intel HEX, reject duplicate conflicting bytes and unchecked records."""
    memory = {}
    upper = 0
    done = False
    for lineno, raw in enumerate(Path(path).read_text(encoding='ascii').splitlines(), 1):
        raw = raw.strip()
        if not raw:continue
        if done: raise ValueError(f'{path}:{lineno}: records after EOF')
        if not raw.startswith(':') or len(raw) < 11 or (len(raw)-1) % 2:
            raise ValueError(f'{path}:{lineno}: invalid record')
        try:record=bytes.fromhex(raw[1:])
        except ValueError as exc:raise ValueError(f'{path}:{lineno}: malformed hex') from exc
        n=record[0]
        if len(record)!=n+5 or sum(record)%256:
            raise ValueError(f'{path}:{lineno}: bad length/checksum')
        offset=(record[1]<<8)|record[2]
        typ=record[3]
        data=record[4:-1]
        if typ==0:
            for i,v in enumerate(data):
                addr=upper+offset+i
                if addr in memory and memory[addr]!=v:
                    raise ValueError(f'{path}:{lineno}: conflicting data at 0x{addr:x}')
                memory[addr]=v
        elif typ==1:
            if n!=0 or offset:raise ValueError('Malformed EOF')
            done=True
        elif typ==2:
            if n!=2 or offset:raise ValueError('Malformed segment address')
            upper=int.from_bytes(data,'big')<<4
        elif typ==4:
            if n!=2 or offset:raise ValueError('Malformed linear address')
            upper=int.from_bytes(data,'big')<<16
        else:
            raise ValueError(f'{path}:{lineno}: unsupported Intel HEX record type {typ}')
    if not done:raise ValueError('Missing HEX EOF record')
    return memory


def make_hex(memory):
    lines=[]
    upper=None
    keys=sorted(memory)
    def add_record(addr,typ,payload):
        record=bytes([len(payload),(addr>>8)&255,addr&255,typ])+bytes(payload)
        lines.append(':'+record.hex().upper()+f'{(-sum(record))&255:02X}')
    i=0
    while i<len(keys):
        start=keys[i]
        newupper=start>>16
        if newupper!=upper:
            if newupper: add_record(0,4,[(newupper>>8)&255,newupper&255])
            upper=newupper
        end=i+1
        while end<len(keys) and keys[end]==start+(end-i) and end-i<16 and keys[end]>>16==newupper:
            end+=1
        add_record(start&0xffff,0,[memory[a] for a in keys[i:end]])
        i=end
    add_record(0,1,[])
    return '\n'.join(lines)+'\n'


def rjmp(from_word,to_word,total_words):
    """Return AVR RJMP word, including PC wrap at end of Flash."""
    delta=(to_word-(from_word+1))%total_words
    if delta >= total_words//2:delta-=total_words
    if not -2048<=delta<=2047:
        raise ValueError(f'RJMP from word 0x{from_word:x} to 0x{to_word:x} out of reach')
    return 0xc000|(delta&0xfff)


def decode_rjmp(from_word,opcode,total_words):
    if opcode&0xf000 != 0xc000:
        raise ValueError(f'Reset opcode 0x{opcode:04X} is not RJMP; JMP and other reset vectors are unsupported')
    d=opcode&0xfff
    if d&0x800:d-=4096
    return (from_word+1+d)%total_words


def patch_application(memory,mcu,force_vector=False):
    if mcu not in TARGETS:raise ValueError('Supported ATtiny targets: '+', '.join(TARGETS))
    target=TARGETS[mcu]
    flash,boot,index=target['flash'],target['boot'],target['vector']
    stop=flash-boot
    if not memory:raise ValueError('Empty application image')
    if min(memory)<0 or max(memory)>=stop:
        raise ValueError(f'Application image overlaps reserved boot region 0x{stop:X}..0x{flash-1:X}')
    if 0 not in memory or 1 not in memory:raise ValueError('Missing 16-bit reset RJMP at address 0')
    reset=(memory[1]<<8)|memory[0]
    words=flash//2
    app_dest=decode_rjmp(0,reset,words)
    if app_dest*2>=stop:
        raise ValueError('Original reset vector jumps to reserved boot region')
    vec_addr=index*2
    old=memory.get(vec_addr,0xff)|(memory.get(vec_addr+1,0xff)<<8)
    if not force_vector and old != 0xffff:
        raise ValueError(f'EEPROM-ready vector at 0x{vec_addr:X} occupied (0x{old:04X}); '+
                         'use --overwrite-ee-vector only after checking the ISR is unused')
    new_reset=rjmp(0,stop//2,words)
    new_vector=rjmp(index,app_dest,words)
    changed=dict(memory)
    changed[0]=new_reset&0xff
    changed[1]=new_reset>>8
    changed[vec_addr]=new_vector&0xff
    changed[vec_addr+1]=new_vector>>8
    return changed,{'mcu':mcu,'application_original_reset_word':hex(app_dest),
                    'reserved_ee_ready_vector_word':hex(index),
                    'patched_reset_instruction':hex(new_reset),
                    'patched_ee_vector_instruction':hex(new_vector),
                    'boot_start':hex(stop)}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--mcu',required=True,choices=sorted(TARGETS))
    p.add_argument('input_hex',type=Path)
    p.add_argument('output_hex',type=Path)
    p.add_argument('--overwrite-ee-vector',action='store_true',
                   help='WARNING: destroy existing EEPROM-ready ISR vector to reserve it as app trampoline')
    args=p.parse_args()
    if args.input_hex.resolve()==args.output_hex.resolve():
        p.error('Use separate input and output files; this tool never overwrites the source')
    mem=read_hex(args.input_hex)
    patched,info=patch_application(mem,args.mcu,args.overwrite_ee_vector)
    args.output_hex.parent.mkdir(parents=True,exist_ok=True)
    args.output_hex.write_text(make_hex(patched),encoding='ascii')
    print('Experimental Virtual Boot vector patch:',info)
    print('WARNING: boot code itself must first be installed via ISP; output is ONLY the patched application.')

if __name__=='__main__':
    try:main()
    except (OSError,ValueError) as exc:
        print('ERROR:',exc,file=sys.stderr)
        sys.exit(1)
