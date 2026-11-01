#!/usr/bin/env python3
"""Software instruction-level STK500v1 tests for 17-bit ATmega128A/1284(P).

Models ELPM Z+ carry into RAMPZ, SPM with RAMPZ, bank boundary crossing,
physical boot protection, 4 KiB EEPROM, and each chip's actual USART address.
Not a physical AVR or real avrdude test; timing assumptions are not verified.
"""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
sys.path.insert(0,str(HERE))
from avr_base_sim import AVR
from test_legacy_uart import symaddr

CHIPS=(('atmega128a', b'\x1e\x97\x02', True),
       ('atmega1284', b'\x1e\x97\x06', False),
       ('atmega1284p', b'\x1e\x97\x05', False))

class AVR128(AVR):
    def __init__(self, boot: bytes, legacy_usart: bool, mcusr=2):
        super().__init__(boot[:512],mcusr=mcusr)
        assert len(boot)==1024
        self.flash=bytearray(b'\xff'*(0x20000-1024)+boot)
        self.boot_start=0x1fc00
        self.pc=self.boot_start
        self.flash_size=0x20000
        self.mem=bytearray(0x4100)
        self.io={0x54:mcusr,0x5b:0,0x57:0,0x68:0,0x60:0,0xc0:0,0x3c:0,0x3f:0}
        self.legacy_usart=legacy_usart
        self.spmcsr=0x68 if legacy_usart else 0x57
        self.eecr=0x3c if legacy_usart else 0x3f
        self.eedr=0x3d if legacy_usart else 0x40
        self.eearl=0x3e if legacy_usart else 0x41
        self.eearh=0x3f if legacy_usart else 0x42
        self.eeprom=bytearray(b'\xff'*4096)
        self.eempe_armed=False
        self.wdt_writes=[]
        self.spm_places=[]
        self.sig=b''
        self.fuse=(0x62,0xff,0xff,0xd9)
    def io_read(self,addr):
        if self.legacy_usart and addr==0x2b:
            return 0x20|(0x80 if self.rx else 0)
        if self.legacy_usart and addr==0x2c:
            if not self.rx:raise AssertionError('Read USART0 without RXC')
            return self.rx.popleft()
        if addr in (self.eedr,self.eearl,self.eearh):
            return self.io.get(addr,0)
        return super().io_read(addr)
    def io_write(self,addr,val):
        val&=255
        if self.legacy_usart and addr==0x2c:
            self.tx.append(val)
        if addr==self.eecr:
            if val&0x04:self.eempe_armed=True
            if val&0x02:
                if not self.eempe_armed:raise AssertionError('EEPROM write without EEMPE')
                offset=self.io.get(self.eearl,0)|(self.io.get(self.eearh,0)<<8)
                assert offset<len(self.eeprom)
                self.eeprom[offset]=self.io.get(self.eedr,0)
                self.eempe_armed=False
            if val&0x01:
                offset=self.io.get(self.eearl,0)|(self.io.get(self.eearh,0)<<8)
                assert offset<len(self.eeprom)
                self.io[self.eedr]=self.eeprom[offset]
            self.io[addr]=0 # Model EEPROM operation completion immediately
            return
        if addr == (0x41 if self.legacy_usart else 0x60):
            self.wdt_writes.append(val)
        super().io_write(addr,val)
    def spm(self):
        cmd=self.io[self.spmcsr]
        addr=((self.io.get(0x5b,0)&1)<<16)|self.z()
        page=addr&~0xff
        if cmd==1:
            self.page_buf[addr&0xff]=self.reg[0]
            self.page_buf[(addr+1)&0xff]=self.reg[1]
            self.spm_places.append(('fill',addr))
        elif cmd==3:
            if page>=self.boot_start:raise AssertionError(f'Attempted to erase protected boot: {page:05X}')
            self.flash[page:page+256]=b'\xff'*256
            self.erase_count+=1
            self.spm_places.append(('erase',page))
        elif cmd==5:
            if page>=self.boot_start:raise AssertionError(f'Attempted to write protected boot: {page:05X}')
            if not self.page_buf:raise AssertionError('Empty Flash page buffer')
            for offset,value in self.page_buf.items():
                self.flash[page+offset]=value
            self.page_buf={}
            self.write_count+=1
            self.spm_places.append(('write',page))
        elif cmd==0x11:
            pass
        else:
            raise AssertionError(f'Unexpected SPMCSR {cmd:02X}')
        self.io[self.spmcsr]=0
    def step(self):
        op=self.u16(self.pc)
        if op&0xfe0f==0x9004 and self.io[self.spmcsr]==9:
            self.reg[(op>>4)&31]=self.fuse[self.z()&3]
            self.io[self.spmcsr]=0
            self.pc=(self.pc+2)%len(self.flash)
            self.cycles+=1
            return
        super().step()
    def wait_ready(self):
        for _ in range(500):
            if self.pc==self.rxwait_pc:return
            self.step()
        raise AssertionError(f'Boot never reached receive loop: {self.pc:05X}')
    def send(self,command,limit=300000):
        self.rx.extend(command)
        tx_before=len(self.tx)
        for n in range(limit):
            if n and not self.rx and self.pc==self.rxwait_pc:
                return bytes(self.tx[tx_before:])
            self.step()
        raise AssertionError(f'Serial transaction timed out: PC {self.pc:05X}')
    def execute_until_error(self,packet,error_pc,limit=50000):
        self.rx.extend(packet)
        for _ in range(limit):
            if self.pc==error_pc:return
            self.step()
        raise AssertionError('Expected a protocol_error loop')
    def load_address(self,byte_address):
        assert byte_address%2==0
        word=byte_address>>1
        return self.send(bytes((0x55,word&255,(word>>8)&255,0x20)))
    def read_flash(self,n):
        return self.send(bytes((0x74,(n>>8)&255,n&255,ord('F'),0x20)))
    def write_flash(self,payload):
        return self.send(bytes((0x64,(len(payload)>>8)&255,len(payload)&255,ord('F')))+payload+b' ')

class TestExtendedAVR(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.images={}
        cls.temp=tempfile.TemporaryDirectory()
        for chip,sig,isold in CHIPS:
            output=Path(cls.temp.name)/chip
            subprocess.run([sys.executable,str(ROOT/'build_mini.py'), '--mcu',chip,'--output-dir',str(output)],check=True,capture_output=True,text=True)
            cls.images[chip]=(output/(chip+'.bin')).read_bytes(), output/(chip+'.o'),sig,isold
    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()
    def cpu(self,chip,mcusr=2):
        boot,obj,sig,old=self.images[chip]
        c=AVR128(boot,old,mcusr)
        c.rxwait_pc=c.boot_start+symaddr(obj,'rx_wait')
        c.error_pc=c.boot_start+symaddr(obj,'error_loop')
        c.wait_ready()
        return c
    def test_signature_signon_and_uart_setup(self):
        for chip,(boot,obj,sig,old) in self.images.items():
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                self.assertEqual(c.io[0x5b],0)
                self.assertEqual(c.send(b'0 '),b'\x14\x10')
                self.assertEqual(c.send(b'1 '),b'\x14AVR STK\x10')
                self.assertEqual(c.send(b'u '),b'\x14'+sig+b'\x10')
                self.assertEqual(c.send(b'A\x81 '),b'\x14\x08\x10')
                self.assertEqual(c.send(bytes((0x56,0x50,0,0,0,0x20))),b'\x14\x62\x10')
                self.assertIn(0x0e,c.wdt_writes)
                if old:
                    self.assertEqual(c.io[0x29],16)
                    self.assertEqual(c.io[0x2a],0x18)
                    self.assertEqual(c.io[0x95],0x06)
                else:
                    self.assertEqual(c.io[0xc4],16)
                    self.assertEqual(c.io[0xc1],0x18)
                    self.assertEqual(c.io[0xc2],0x06)
    def test_program_read_both_flash_banks(self):
        for chip in self.images:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                for address in (0x0200,0x10000,0x1fa00,0x1fb00):
                    payload=bytes((i*13+address//256)&255 for i in range(256))
                    self.assertEqual(c.load_address(address),b'\x14\x10')
                    self.assertEqual(c.write_flash(payload),b'\x14\x10')
                    self.assertEqual(c.flash[address:address+256],payload)
                    self.assertEqual(c.load_address(address),b'\x14\x10')
                    self.assertEqual(c.read_flash(256),b'\x14'+payload+b'\x10')
                    self.assertIn(('erase',address),c.spm_places)
                self.assertEqual(c.flash[c.boot_start:],self.images[chip][0])
    def test_elpm_crosses_64k_boundary_and_resets_bank_on_repeat(self):
        for chip in self.images:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                address=0xff80
                payload=bytes((i*3+7)&255 for i in range(256))
                c.flash[address:address+256]=payload
                self.assertEqual(c.load_address(address),b'\x14\x10')
                self.assertEqual(c.read_flash(256),b'\x14'+payload+b'\x10')
                self.assertEqual(c.io[0x5b],1)
                self.assertEqual(c.read_flash(256),b'\x14'+payload+b'\x10')
                self.assertEqual(c.send(b'1 '),b'\x14AVR STK\x10')
                self.assertEqual(c.load_address(address),b'\x14\x10')
                self.assertEqual(c.read_flash(256),b'\x14'+payload+b'\x10')
    def test_eeprom_4k_and_high_bank_rejection(self):
        for chip in self.images:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                offset=4096-16
                data=bytes(range(16))
                self.assertEqual(c.load_address(offset),b'\x14\x10')
                packet=bytes((0x64,0,16,ord('E')))+data+b' '
                self.assertEqual(c.send(packet),b'\x14\x10')
                self.assertEqual(c.eeprom[offset:],data)
                self.assertEqual(c.send(bytes((0x74,0,16,ord('E'),0x20))),b'\x14'+data+b'\x10')
                bad=self.cpu(chip)
                bad.load_address(0x10000)
                bad.execute_until_error(bytes((0x74,0,16,ord('E'),0x20)),bad.error_pc)
                self.assertEqual(bad.eeprom,b'\xff'*4096)
    def test_boot_write_and_flash_end_overrun_rejected(self):
        for chip in self.images:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                c.load_address(0x1fc00)
                c.execute_until_error(bytes((0x64,0,8,ord('F')))+bytes(range(8))+b' ',c.error_pc)
                self.assertEqual(c.erase_count,0)
                self.assertEqual(c.flash[c.boot_start:],self.images[chip][0])
                c2=self.cpu(chip)
                c2.load_address(0x1ff80)
                c2.execute_until_error(bytes((0x74,1,0,ord('F'),0x20)),c2.error_pc)
    def test_bad_packet_does_not_erase_and_invalid_memory_is_rejected(self):
        for chip in self.images:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                address=0x10100
                old=bytes((0xA5,))*256
                c.flash[address:address+256]=old
                self.assertEqual(c.load_address(address),b'\x14\x10')
                c.execute_until_error(bytes((0x64,0,8,ord('F')))+bytes(range(8))+b'\x00',c.error_pc)
                self.assertEqual(c.flash[address:address+256],old)
                self.assertEqual(c.erase_count,0)
                c2=self.cpu(chip)
                c2.load_address(0x10000)
                self.assertEqual(c2.send(bytes((0x74,0,4,ord('X'),0x20))),b'\x14\x11')
                self.assertEqual(c2.erase_count,0)

    def test_1024_2048_4096_8192_boot_options(self):
        # Hardware BOOTSZ fuses may choose any of these four sizes.
        for chip in ('atmega128a','atmega1284p'):
            for capacity in (1024,2048,4096,8192):
                with self.subTest(chip=chip,boot_bytes=capacity):
                    with tempfile.TemporaryDirectory() as t:
                        subprocess.run([sys.executable,str(ROOT/'build_mini.py'),'--mcu',chip,
                            '--boot-bytes',str(capacity),'--output-dir',t],
                            check=True,stdout=subprocess.DEVNULL)
                        binary=(Path(t)/(chip+'.bin')).read_bytes()
                        self.assertEqual(len(binary),capacity)
                        self.assertEqual(binary[-9:-2],b'AVR STK')

    def test_app_reset_vectors(self):
        for chip in self.images:
            for flag in (1,8):
                with self.subTest(chip=chip,flag=flag):
                    boot,obj,sig,old=self.images[chip]
                    c=AVR128(boot,old,mcusr=flag)
                    for _ in range(100):
                        if c.app_started:break
                        c.step()
                    self.assertTrue(c.app_started)

if __name__=='__main__':unittest.main(verbosity=2)
