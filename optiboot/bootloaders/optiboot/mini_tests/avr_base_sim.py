#!/usr/bin/env python3
"""Minimal instruction-level AVR simulator for testing optiboot_mini328 variants.
Only the opcodes present in the generated bootloaders are implemented.
This is a regression smoke test, NOT a substitute for a real ATmega328P board.
"""
from collections import deque
from pathlib import Path
import random

ROOT=Path(__file__).resolve().parent

class AVR:
    def __init__(self, boot: bytes, mcusr=2):
        assert len(boot)==512
        self.flash=bytearray(b'\xFF'*0x7E00 + boot)
        self.mem=bytearray(0x900)
        self.io={0x54:mcusr,0x57:0x00,0x60:0x00,0xC2:0x06}
        self.reg=[0]*32
        self.stack=[]
        self.pc=0x7e00
        self.zero=False
        self.carry=False
        self.tflag=False
        self.rx=deque()
        self.tx=[]
        self.page_buf={}
        self.erase_count=0
        self.write_count=0
        self.app_started=False
        self.cycles=0
        self.maxstack=0
        self.wdr_count=0
    def u16(self,p):return self.flash[p] | (self.flash[p+1]<<8)
    @staticmethod
    def signed(x,n):return x-(1<<n) if x&(1<<(n-1)) else x
    def z(self):return self.reg[30] | (self.reg[31]<<8)
    def set_z(self,x):self.reg[30]=x&255;self.reg[31]=(x>>8)&255
    def y(self):return self.reg[28] | (self.reg[29]<<8)
    def x(self):return self.reg[26] | (self.reg[27]<<8)
    def set_x(self,x):self.reg[26]=x&255;self.reg[27]=(x>>8)&255
    def io_read(self,addr):
        if addr==0x36:  # TIFR1.TOV1: model timer overflow (skip LED startup delay)
            return 0x01
        if addr==0xc0:
            return self.io.get(addr,0)|0x20|(0x80 if self.rx else 0)
        if addr==0xc6:
            if not self.rx:raise RuntimeError('Read UDR0 without RXC')
            return self.rx.popleft()
        if addr<0x100:return self.io.get(addr,0)
        return self.mem[addr]
    def io_write(self,addr,val):
        val&=255
        if addr==0xc6:
            self.tx.append(val)
        if addr in (0x54,0x57,0x60,0xc0,0xc1,0xc2,0xc4,0xc5,0xc6):
            self.io[addr]=val
        elif addr<0x100:self.io[addr]=val
        else:self.mem[addr]=val
    def spm(self):
        cmd=self.io[0x57]
        z=self.z()
        if cmd==3:
            page=z&~0x7f
            assert page<0x7e00,hex(page)
            self.flash[page:page+128]=b'\xFF'*128
            # Datasheet 30.8: SPM buffer can be filled *before* page erase.
            # Erase must not destroy temporary page-buffer contents.
            self.erase_count+=1
        elif cmd==1:
            self.page_buf[z&0x7f]=self.reg[0]
            self.page_buf[(z+1)&0x7f]=self.reg[1]
        elif cmd==5:
            page=z&~0x7f
            assert page<0x7e00
            assert len(self.page_buf)>0
            for offset,v in self.page_buf.items():self.flash[page+offset]=v
            self.write_count+=1
            self.page_buf={}
        elif cmd==0x11:
            pass
        else:
            raise RuntimeError(f'SPM unsupported cmd {cmd:02x}')
        self.io[0x57]=0
    def step(self):
        pc=self.pc
        if pc==0:
            self.app_started=True
            return
        op=self.u16(pc)
        self.cycles+=1
        newpc=pc+2
        rd=(op>>4)&0x1f
        rr=(op&0xf)|((op>>5)&0x10)
        if op & 0xf000 == 0xc000: # RJMP
            newpc+=2*self.signed(op&0xfff,12)
        elif op & 0xf000 == 0xd000: # RCALL
            self.stack.append(pc+2)
            self.maxstack=max(self.maxstack,len(self.stack))
            newpc+=2*self.signed(op&0xfff,12)
        elif op==0x9508: # ret
            if not self.stack:raise RuntimeError('Unexpected RET with empty stack')
            newpc=self.stack.pop()
        elif op==0x94f8: # cli
            pass
        elif op==0x9488: # clc
            self.carry=False
        elif op==0x9408: # sec
            self.carry=True
        elif op==0x95a8: # wdr
            self.wdr_count+=1
        elif op==0x95e8: # spm
            self.spm()
        elif op==0x940c: # JMP 0; only used absolute jump in this assembly
            imm=self.u16(pc+2)
            if imm !=0:raise RuntimeError(f'Unexpected JMP 0x{imm:04x}')
            newpc=0
        elif op & 0xf000 == 0xe000: # ldi
            d=16+((op>>4)&0xf); k=(op&0xf)|((op>>4)&0xf0)
            self.reg[d]=k
        elif op & 0xf000 == 0x3000: # cpi
            d=16+((op>>4)&0xf); k=(op&0xf)|((op>>4)&0xf0)
            self.carry=self.reg[d]<k;self.zero=self.reg[d]==k
        elif op & 0xf000 == 0x5000: # subi
            d=16+((op>>4)&0xf); k=(op&0xf)|((op>>4)&0xf0)
            v=self.reg[d];self.reg[d]=(v-k)&255
            self.carry=v<k;self.zero=self.reg[d]==0
        elif op & 0xf000 == 0x7000: # andi
            d=16+((op>>4)&0xf); k=(op&0xf)|((op>>4)&0xf0)
            self.reg[d]&=k;self.zero=self.reg[d]==0
        elif op&0xfc00==0x1400: # CP
            self.carry=self.reg[rd]<self.reg[rr];self.zero=self.reg[rd]==self.reg[rr]
        elif op&0xfc00==0x2800: # OR
            self.reg[rd]|=self.reg[rr];self.zero=self.reg[rd]==0
        elif op&0xfc00==0x1000: # CPSE
            if self.reg[rd]==self.reg[rr]:
                nextinst=self.u16(pc+2)
                newpc+=4 if ((nextinst&0xfe0f in (0x9000,0x9200)) or nextinst==0x940c) else 2
        elif op&0xfc00==0x2c00: # mov
            self.reg[rd]=self.reg[rr]
        elif op&0xff00==0x0100: # movw
            d=2*((op>>4)&0xf);r=2*(op&0xf)
            self.reg[d:d+2]=self.reg[r:r+2]
        elif op&0xfc00==0x2400: # clr r / eor
            self.reg[rd]^=self.reg[rr];self.zero=self.reg[rd]==0
        elif op&0xfc00==0x2000: # tst r / and
            self.reg[rd]&=self.reg[rr];self.zero=self.reg[rd]==0
        elif op&0xfc00==0x0c00: # lsl / add
            val=self.reg[rd]+self.reg[rr]
            self.carry=val>255;self.reg[rd]=val&255;self.zero=self.reg[rd]==0
        elif op&0xfc00==0x1c00: # rol / adc
            val=self.reg[rd]+self.reg[rr]+int(self.carry)
            self.carry=val>255;self.reg[rd]=val&255;self.zero=self.reg[rd]==0
        elif op&0xf800==0xb000: # in
            addr=(op&0xf)|((op>>5)&0x30)
            self.reg[rd]=self.io_read(addr+0x20)
        elif op&0xf800==0xb800: # out
            addr=(op&0xf)|((op>>5)&0x30)
            self.io_write(addr+0x20,self.reg[rd])
        elif op&0xfe0f==0x9000: # lds Rd, mem
            addr=self.u16(pc+2);self.reg[rd]=self.io_read(addr)
            newpc=pc+4
        elif op&0xfe0f==0x9200: # sts mem, Rr
            addr=self.u16(pc+2);self.io_write(addr,self.reg[rd])
            newpc=pc+4
        elif op&0xfe0f==0x9406: # LSR Rd
            old=self.reg[rd]
            self.carry=bool(old&1)
            self.reg[rd]=old>>1
            self.zero=self.reg[rd]==0
        elif op&0xfe0f==0x9407: # ROR Rd: old C becomes MSB; bit 0 becomes new C
            old=self.reg[rd]
            carry_in=self.carry
            self.carry=bool(old&1)
            self.reg[rd]=(old>>1)|(0x80 if carry_in else 0)
            self.zero=self.reg[rd]==0
        elif op&0xfe0f==0x9403: # inc
            self.reg[rd]=(self.reg[rd]+1)&255
            self.zero=self.reg[rd]==0
        elif op&0xfe0f==0x940a: # dec
            self.reg[rd]=(self.reg[rd]-1)&255
            self.zero=self.reg[rd]==0
        elif op&0xff00==0x9a00: # sbi
            addr=0x20+((op>>3)&0x1f); bit=op&7
            self.io_write(addr,self.io_read(addr)|(1<<bit))
        elif op&0xff00==0x9800: # CBI
            addr=0x20+((op>>3)&0x1f);bit=op&7
            self.io_write(addr,self.io_read(addr)&~(1<<bit))
        elif op&0xff00==0x9900: # SBIC
            addr=0x20+((op>>3)&0x1f);bit=op&7
            if not (self.io_read(addr)&(1<<bit)):
                nextinst=self.u16(pc+2)
                newpc+=4 if ((nextinst&0xfe0f in (0x9000,0x9200)) or nextinst==0x940c) else 2
        elif op&0xff00==0x9b00: # sbis
            addr=0x20+((op>>3)&0x1f); bit=op&7
            if self.io_read(addr)&(1<<bit):
                nextinst=self.u16(pc+2)
                newpc+=4 if ((nextinst&0xfe0f in (0x9000,0x9200)) or nextinst==0x940c) else 2
        elif op&0xfe0f==0x920f: # push
            self.stack.append(self.reg[rd]);self.maxstack=max(self.maxstack,len(self.stack))
        elif op&0xfe0f==0x900f: # pop
            self.reg[rd]=self.stack.pop()
        elif op&0xfe0f==0x8200: # st Z,r
            self.io_write(self.z(),self.reg[rd])
        elif op&0xfe0f==0x9201: # st Z+,r
            z=self.z();self.io_write(z,self.reg[rd]);self.set_z(z+1)
        elif op&0xfe08==0xfa00: # BST Rd,bit (T flag)
            self.tflag=bool(self.reg[rd] & (1<<(op&7)))
        elif op&0xfe08==0xf800: # BLD Rd,bit (copy T flag into bit)
            bit=op&7
            self.reg[rd]=(self.reg[rd]&~(1<<bit)) | (int(self.tflag)<<bit)
        elif op&0xfe08==0xfc00: # sbrc
            bit=op&7
            if self.reg[rd]&(1<<bit)==0:
                nextinst=self.u16(pc+2)
                newpc+=4 if ((nextinst&0xfe0f in (0x9000,0x9200)) or nextinst==0x940c) else 2
        elif op&0xfe08==0xfe00: # sbrs
            bit=op&7
            if self.reg[rd]&(1<<bit):
                nextinst=self.u16(pc+2)
                newpc+=4 if ((nextinst&0xfe0f in (0x9000,0x9200)) or nextinst==0x940c) else 2
        elif op&0xfc00 in (0xf000,0xf400): # brbs / brbc
            flag=op&7
            if flag==0:b=bool(self.carry)
            elif flag==1:b=bool(self.zero)
            elif flag==6:b=bool(self.tflag)
            else:raise RuntimeError('Non-emulated SREG flag '+str(flag))
            take=b if (op&0x400)==0 else not b
            if take:newpc+=2*self.signed((op>>3)&0x7f,7)
        elif op & 0xd208 == 0x8008: # LDD Rd,Y+q (USART base pointer)
            q=((op>>13)&1)*32+((op>>10)&3)*8+(op&7)
            self.reg[rd]=self.io_read(self.y()+q)
        elif op & 0xd208 == 0x8208: # STD Y+q,Rr
            q=((op>>13)&1)*32+((op>>10)&3)*8+(op&7)
            self.io_write(self.y()+q,self.reg[rd])
        elif op&0xfe0f==0x900d: # ld r, X+
            x=self.x();self.reg[rd]=self.mem[x];self.set_x(x+1)
        elif op&0xfe0f==0x920d: # st X+,r
            x=self.x();self.mem[x]=self.reg[rd];self.set_x(x+1)
        elif op&0xfe0f==0x9007: # ELPM Rd,Z+ (17+ bit Flash address)
            z=self.z()
            # RAMPZ is a bank register (1 bit on 128K, 2 bits on 256K).
            # ELPM Z+ increments the full RAMPZ:Z register on wrap.
            rampz=self.io.get(0x5b,0)
            self.reg[rd]=self.flash[(rampz<<16)|z]
            if z==0xffff:self.io[0x5b]=(rampz + 1)&0xff
            self.set_z(z+1)
        elif op&0xfe0f==0x9005: # lpm Rd, Z+
            z=self.z();self.reg[rd]=self.flash[z];self.set_z(z+1)
        elif op&0xff00==0x9600: # adiw
            d=24+2*((op>>4)&3)
            imm=(op&0xf)|((op>>2)&0x30)
            old=self.reg[d]|(self.reg[d+1]<<8)
            val=(old+imm)&0xffff
            self.reg[d]=val&255;self.reg[d+1]=val>>8
            self.zero=val==0
        elif op&0xff00==0x9700: # SBIW
            d=24+2*((op>>4)&3)
            imm=(op&0xf)|((op>>2)&0x30)
            old=self.reg[d]|(self.reg[d+1]<<8)
            val=(old-imm)&0xffff
            self.reg[d]=val&255;self.reg[d+1]=val>>8
            self.zero=val==0
        elif op==0: # nop
            pass
        else:raise RuntimeError(f'Unknown opcode 0x{op:04x} at 0x{pc:04x}')
        self.pc=newpc % len(self.flash)
        if not 0<=self.pc<len(self.flash):raise RuntimeError(f'PC outside flash: 0x{self.pc:05x}')
    def send(self,command,limit=500000):
        self.rx.extend(command)
        start=len(self.tx)
        expected=None
        for i in range(limit):
            # SUSPEND once all input consumed and UART has returned to idle input polling
            if i>10 and not self.rx and self.pc in (self.rxwait_pc,self.rxwait_pc+4,self.rxwait_pc+6):
                return bytes(self.tx[start:])
            self.step()
            if self.app_started:raise RuntimeError('Unexpected application launch')
        raise RuntimeError(f'Timeout at pc {self.pc:04x}, remaining RX {len(self.rx)} bytes')

def test(name):
    print(f'Testing {name}...')
    boot=(ROOT/(name+'.bin')).read_bytes()
    a=AVR(boot,2)
    # rx_wait_pc: first lds 0xC0 instruction sequence in rx_char
    needle=bytes.fromhex('2091c00027ff') # likely r18 lds, sbrs r18 bit 7
    idx=boot.find(needle)
    if idx<0:raise RuntimeError('Failed to find RX instruction sequence')
    a.rxwait_pc=0x7e00+idx
    for _ in range(1000):
        if a.pc==a.rxwait_pc:break
        a.step()
    else:raise RuntimeError('Did not initialize')
    assert a.io[0xc1]==0x18, a.io
    assert a.io[0xc4]==16, a.io
    assert a.io[0x60]==0x0E,'Bootloader WDT must run reset mode at 1s'
    def send(packet,response):
        ans=a.send(packet)
        assert ans==response, f'packet {packet[:12].hex()} actual {ans.hex()} expected {response.hex()}'
    send(bytes((0x30,0x20)),bytes((0x14,0x10)))
    send(bytes((0x41,0x81,0x20)),bytes((0x14,8,0x10)))
    send(bytes((0x41,0x82,0x20)),bytes((0x14,3,0x10)))
    send(bytes((0x41,0x80,0x20)),bytes((0x14,3,0x10)))
    send(bytes((0x42,))+bytes(20)+(b' '),bytes((0x14,0x10)))
    send(bytes((0x45,))+bytes(5)+(b' '),bytes((0x14,0x10)))
    send(b'P ',bytes((0x14,0x10)))
    send(bytes((0x56,0,0,0,0,0x20)),bytes((0x14,0,0x10)))
    send(bytes((0x75,0x20)),bytes((0x14,0x1e,0x95,0x0f,0x10)))
    data=bytes(random.Random(25+i).randrange(256) for i in range(128))
    send(bytes((0x55,0x00,0x00,0x20)),bytes((0x14,0x10)))
    send(bytes((0x64,0x00,0x80,ord('F')))+data+b' ',bytes((0x14,0x10)))
    assert a.erase_count==1 and a.write_count==1,(a.erase_count,a.write_count)
    assert a.flash[:128]==data,'Flash page programming mismatch'
    send(bytes((0x74,0x00,0x80,ord('F'),0x20)),b'\x14'+data+b'\x10')
    # Second page at address 0x0120, 8 bytes, to test word->byte conversion.
    target=0x0120
    ndata=bytes(range(70,78))
    send(bytes((0x55,(target>>1)&255,(target>>9)&255,0x20)),bytes((0x14,0x10)))
    send(bytes((0x64,0,8,ord('F')))+ndata+b' ',bytes((0x14,0x10)))
    assert a.flash[target:target+8]==ndata
    send(bytes((0x74,0,8,ord('F'),0x20)),b'\x14'+ndata+b'\x10')
    assert a.flash[0x7e00:]==boot,'Boot area unexpectedly modified'
    send(b'Q ',bytes((0x14,0x10)))
    b=AVR(boot,1)
    for _ in range(1000):
        b.step()
        if b.app_started:break
    assert b.app_started,'Power-on reset must launch application'
    w=AVR(boot,8)
    for _ in range(1000):
        w.step()
        if w.app_started:break
    assert w.app_started,'WDRF-only reset must launch application'
    assert a.io[0x60]==0x08,'Leave programming must enable 16ms WDT reset (WDE=1)'
    assert a.reg[1]==0, 'r1 zero register not restored'

    # Corrupt EOP after a complete PROG_PAGE payload. The buffered build must
    # preserve the old page; stream variant loses it because erase is earlier.
    bad=AVR(boot,2)
    bad.rxwait_pc=a.rxwait_pc
    while bad.pc!=bad.rxwait_pc:bad.step()
    page=0x400
    bad.flash[page:page+128]=bytes([0x55]*128)
    bad.send(bytes((0x55,0,2,0x20))) # word address 0x0200 -> byte 0x0400
    bad.rx.extend(bytes((0x64,0,8,ord('F')))+bytes(range(8))+b'\x00')
    err_pc=0x7e00+boot.find(bytes.fromhex('ffcf'))
    assert err_pc>0x7e00
    for i in range(10000):
        bad.step()
        if bad.pc==err_pc:break
    else:raise AssertionError('Bad EOP was not rejected')
    corrupted=bad.flash[page:page+128]!=bytes([0x55]*128)
    if name.endswith('_stream'):
        assert corrupted,'Streaming variant must demonstrate erase-on-invalid-packet risk'
    else:
        assert not corrupted,'Buffered variant must validate EOP before erasing'
    print(f'  malformed PROG_PAGE: rejected; old Flash page '+('ERASED (streaming risk)' if corrupted else 'PRESERVED'))
    print(f'  PASS sync+version+device+universal+signature+program+read+leave+power-on; '
          f'flash_erases={a.erase_count}, flash_writes={a.write_count}, '
          f'AVR instructions={a.cycles}, stack_depth={a.maxstack}')
    return a

if __name__=='__main__':
    for name in ['optiboot_mini328_final','optiboot_mini328_final_tiny']:
        test(name)
