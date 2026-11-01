#!/usr/bin/env python3
"""Executable AVR instruction tests for Oct 30 classic-ATmega ports.

Checks actual opcodes, not just build metadata. This is NOT a physical
cycle-accurate peripheral simulation. In particular, no oscillator drift,
real SPM/EEPROM timing, pin directions, fuse lock bits, or real AVRDUDE tested.
"""
import sys,subprocess,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT));sys.path.insert(0,str(HERE))
import build_mini
from test_128k import AVR128
from test_legacy_uart import LegacyAVR,symaddr
from test_modern_audit import AVRModern

LEGACY=('atmega8','atmega8a','atmega8515','atmega8535','atmega162')
MODERN_64=('atmega644','atmega644p','atmega644a','atmega644pa','atmega640')
B256=('atmega2560','atmega2561')

class LegacyNew(LegacyAVR):
    def __init__(self,img,dev,mcusr=2):
        super().__init__(img,dev['flash'],bytes(dev['sig']),mcusr)
        self.eeprom=bytearray([255]*dev['eeprom'])
        self.page_size=dev['page']
    def spm(self):
        cmd=self.io.get(0x57,0)
        addr=self.z()
        page=addr&~(self.page_size-1)
        if cmd==1:
            self.page_buf[addr&(self.page_size-1)]=self.reg[0]
            self.page_buf[(addr+1)&(self.page_size-1)]=self.reg[1]
        elif cmd==3:
            if page>=self.boot_start:raise AssertionError('Boot erase at '+hex(page))
            self.flash[page:page+self.page_size]=b'\xff'*self.page_size
            self.erase_count+=1
        elif cmd==5:
            if page>=self.boot_start:raise AssertionError('Boot write at '+hex(page))
            for off,v in self.page_buf.items():self.flash[page+off]=v
            self.page_buf={};self.write_count+=1
        elif cmd==0x11:pass
        else:raise AssertionError('Unexpected SPM opcode '+hex(cmd))
        self.io[0x57]=0

class Mega256New(AVR128):
    def __init__(self,boot,mcusr=2):
        super().__init__(boot,False,mcusr)
        self.flash_size=0x40000
        self.boot_start=self.flash_size-len(boot)
        self.flash=bytearray(b'\xff'*self.boot_start+boot)
        self.pc=self.boot_start
        self.io[0x5b]=0
    def spm(self):
        cmd=self.io[self.spmcsr]
        addr=(self.io.get(0x5b,0)<<16)|self.z()
        page=addr&~255
        if cmd==1:
            self.page_buf[addr&255]=self.reg[0]
            self.page_buf[(addr+1)&255]=self.reg[1]
        elif cmd==3:
            if page>=self.boot_start:raise AssertionError('Protected Boot erase at '+hex(page))
            self.flash[page:page+256]=b'\xff'*256;self.erase_count+=1
        elif cmd==5:
            if page>=self.boot_start:raise AssertionError('Protected Boot write at '+hex(page))
            if not self.page_buf:raise AssertionError('SPM wrote empty page')
            for off,v in self.page_buf.items():self.flash[page+off]=v
            self.page_buf={};self.write_count+=1
        elif cmd==0x11:pass
        else:raise AssertionError('SPMCSR opcode '+hex(cmd))
        self.io[self.spmcsr]=0
    def ext_addr(self,ext):
        return self.send(bytes((0x56,0x4d,0,ext,0,0x20)))
    def load_addr(self,byte_addr):
        # Caller must have sent correct LOAD_EXTENDED_ADDRESS bank.
        return self.send(bytes((0x55,(byte_addr>>1)&255,(byte_addr>>9)&255,0x20)))

class ExpandedPorts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory()
        cls.builds={}
        for chip in (*LEGACY,*MODERN_64,*B256):
            out=Path(cls.temp.name)/chip
            cp=subprocess.run([sys.executable,str(ROOT/'build_mini.py'),'--mcu',chip,'--output-dir',str(out)],text=True,capture_output=True)
            if cp.returncode:raise AssertionError(f'{chip} compilation failed: {cp.stderr}')
            cls.builds[chip]=((out/(chip+'.bin')).read_bytes(),out/(chip+'.o'),build_mini.DEVICES[chip])
    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()
    def cpu(self,chip):
        boot,obj,dev=self.builds[chip]
        if chip in LEGACY:c=LegacyNew(boot,dev)
        elif chip in MODERN_64:c=AVRModern(boot,dev['flash'],dev['page'],dev['eeprom'])
        else:c=Mega256New(boot)
        c.rxwait_pc=c.boot_start+symaddr(obj,'rx_wait')
        c.error_pc=c.boot_start+symaddr(obj,'error_loop')
        c.wait_ready()
        return c
    def test_legacy_64_128_page_program_and_eeprom(self):
        for chip in LEGACY:
            with self.subTest(chip=chip):
                boot,obj,d=self.builds[chip]
                c=self.cpu(chip)
                self.assertEqual(c.ucsrc,0x86)
                self.assertEqual(c.ubrrl,16)
                self.assertEqual(c.send(b'u '),b'\x14'+bytes(d['sig'])+b'\x10')
                address=c.boot_start-d['page']
                payload=bytes((i*13+(address>>8))&255 for i in range(d['page']))
                self.assertEqual(c.send(bytes((0x55,(address>>1)&255,(address>>9)&255,0x20))),b'\x14\x10')
                self.assertEqual(c.send(bytes((0x64,0,len(payload),ord('F')))+payload+b' '),b'\x14\x10')
                self.assertEqual(c.flash[address:address+d['page']],payload)
                self.assertEqual(c.send(bytes((0x74,0,len(payload),ord('F'),0x20))),b'\x14'+payload+b'\x10')
                eeprom_base=d['eeprom']-16
                self.assertEqual(c.send(bytes((0x55,(eeprom_base>>1)&255,(eeprom_base>>9)&255,0x20))),b'\x14\x10')
                d8=bytes(range(16))
                self.assertEqual(c.send(bytes((0x64,0,16,ord('E')))+d8+b' '),b'\x14\x10')
                self.assertEqual(c.eeprom[eeprom_base:],d8)
                self.assertEqual(c.send(bytes((0x74,0,16,ord('E'),0x20))),b'\x14'+d8+b'\x10')
                self.assertEqual(c.flash[c.boot_start:],boot)
    def test_64k_modern_256_page_program_and_eeprom(self):
        for chip in MODERN_64:
            with self.subTest(chip=chip):
                boot,obj,d=self.builds[chip]
                c=self.cpu(chip)
                self.assertEqual(c.io[0xc1],0x18)
                self.assertEqual(c.send(b'u '),b'\x14'+bytes(d['sig'])+b'\x10')
                address=c.boot_start-256
                payload=bytes((i*7+83)&255 for i in range(256))
                self.assertEqual(c.load_address(address),b'\x14\x10')
                self.assertEqual(c.write_flash(payload),b'\x14\x10')
                self.assertEqual(c.flash[address:address+256],payload)
                self.assertEqual(c.load_address(address),b'\x14\x10')
                self.assertEqual(c.read_flash(256),b'\x14'+payload+b'\x10')
                eaddr=d['eeprom']-16
                self.assertEqual(c.load_address(eaddr),b'\x14\x10')
                self.assertEqual(c.send(bytes((0x64,0,16,ord('E')))+bytes(range(16))+b' '),b'\x14\x10')
                self.assertEqual(c.eeprom[eaddr:],bytes(range(16)))
    def test_256k_four_bank_program_read_and_boot_protection(self):
        for chip in B256:
            with self.subTest(chip=chip):
                boot,obj,d=self.builds[chip]
                c=self.cpu(chip)
                self.assertEqual(c.send(b'1 '),b'\x14AVR STK\x10')
                self.assertEqual(c.send(b'u '),b'\x14'+bytes(d['sig'])+b'\x10')
                for address in (0x0400,0x10000,0x20000,0x30000,0x3fb00):
                    self.assertEqual(c.ext_addr(address>>17),b'\x14\x00\x10')
                    self.assertEqual(c.load_addr(address),b'\x14\x10')
                    data=bytes((i*5+address//256)&255 for i in range(256))
                    self.assertEqual(c.write_flash(data),b'\x14\x10')
                    self.assertEqual(c.flash[address:address+256],data)
                    self.assertEqual(c.load_addr(address),b'\x14\x10')
                    self.assertEqual(c.read_flash(256),b'\x14'+data+b'\x10')
                self.assertEqual(c.flash[c.boot_start:],boot)
    def test_256k_elpm_read_crosses_two_bank_boundaries(self):
        for chip in B256:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                for addr in (0xff80,0x1ff80,0x2ff80):
                    data=bytes((i*9+addr//256)&255 for i in range(256))
                    c.flash[addr:addr+256]=data
                    self.assertEqual(c.ext_addr(addr>>17),b'\x14\x00\x10')
                    self.assertEqual(c.load_addr(addr),b'\x14\x10')
                    self.assertEqual(c.read_flash(256),b'\x14'+data+b'\x10')
                    self.assertEqual(c.io[0x5b],(addr>>16)+1)
                    self.assertEqual(c.read_flash(256),b'\x14'+data+b'\x10')
    def test_256k_ee_and_rejected_3fc00_and_invalid_bank(self):
        for chip in B256:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                eaddr=4080; data=bytes(range(16))
                self.assertEqual(c.load_addr(eaddr),b'\x14\x10')
                self.assertEqual(c.send(bytes((0x64,0,16,ord('E')))+data+b' '),b'\x14\x10')
                self.assertEqual(c.eeprom[eaddr:],data)
                self.assertEqual(c.ext_addr(1),b'\x14\x00\x10')
                self.assertEqual(c.load_addr(0x3fc00),b'\x14\x10')
                boot,cboot=c.flash[c.boot_start:],c.boot_start
                c.execute_until_error(bytes((0x64,1,0,ord('F')))+bytes(256)+b' ',c.error_pc,limit=50000)
                self.assertEqual(c.flash[cboot:],boot)
                self.assertEqual(c.erase_count,0)
                another=self.cpu(chip)
                another.execute_until_error(bytes((0x56,0x4d,0,2,0,0x20)),another.error_pc)
                self.assertEqual(another.erase_count,0)
    def test_256k_invalid_eop_and_flash_end_read(self):
        for chip in B256:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                c.ext_addr(1);c.load_addr(0x3fb00)
                c.execute_until_error(bytes((0x64,1,0,ord('F')))+bytes(256)+b'!',c.error_pc,limit=50000)
                self.assertEqual(c.erase_count,0)
                another=self.cpu(chip)
                another.ext_addr(1);another.load_addr(0x3ff80)
                another.execute_until_error(bytes((0x74,1,0,ord('F'),0x20)),another.error_pc,limit=10000)

if __name__=='__main__':unittest.main()
