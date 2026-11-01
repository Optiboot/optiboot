"""Additional instruction-level validation of ATmega64A (previously only build-tested).

Uses the established AVR128 instruction engine, but with 64KiB flash, 2KiB
EEPROM and ATmega64A memory map. This is NOT a hardware/timing simulation.
"""
from pathlib import Path
import subprocess, sys, tempfile, unittest
HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
sys.path.insert(0,str(HERE))
from test_128k import AVR128
from test_legacy_uart import symaddr

class AVR64(AVR128):
    def __init__(self,boot,mcusr=2):
        super().__init__(boot[:1024], True, mcusr)
        self.flash_size=65536
        self.boot_start=65536-len(boot)
        self.flash=bytearray(b'\xff'*self.boot_start+boot)
        self.pc=self.boot_start
        self.eeprom=bytearray(b'\xff'*2048)
        self.spmcsr=0x68  # ATmega64A SPMCSR at extended data address 0x68

class TestATmega64A(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory()
        cls.out=Path(cls.tmp.name)
        subprocess.run([sys.executable,str(ROOT/'build_mini.py'), '--mcu','atmega64a','--output-dir',str(cls.out)],check=True, capture_output=True,text=True)
        cls.img=(cls.out/'atmega64a.bin').read_bytes()
        cls.obj=cls.out/'atmega64a.o'
    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()
    def cpu(self,mcusr=2):
        c=AVR64(self.img,mcusr)
        c.rxwait_pc=c.boot_start+symaddr(self.obj,'rx_wait')
        c.error_pc=c.boot_start+symaddr(self.obj,'error_loop')
        c.wait_ready()
        return c
    def test_signature_uart_stk_protocol_and_full_256_page(self):
        c=self.cpu()
        self.assertEqual(c.send(b'0 '),b'\x14\x10')
        self.assertEqual(c.send(b'1 '),b'\x14AVR STK\x10')
        self.assertEqual(c.send(b'u '),b'\x14\x1e\x96\x02\x10')
        self.assertEqual(c.io[0x95],6)  # UCSR0C
        self.assertEqual(c.io[0x90],0)  # UBRR0H
        self.assertEqual(c.io[0x29],16) # UBRR0L
        for addr in (0x0000,0x4000,0xFA00,0xFB00):
            data=bytes((i*37+(addr>>8)*7)&255 for i in range(256))
            self.assertEqual(c.load_address(addr),b'\x14\x10')
            self.assertEqual(c.write_flash(data),b'\x14\x10')
            self.assertEqual(c.flash[addr:addr+256],data)
            self.assertEqual(c.load_address(addr),b'\x14\x10')
            self.assertEqual(c.read_flash(256),b'\x14'+data+b'\x10')
        self.assertEqual(c.flash[c.boot_start:],self.img)
    def test_eeprom_last_16_and_protected_boot(self):
        c=self.cpu()
        addr=2048-16
        self.assertEqual(c.load_address(addr),b'\x14\x10')
        payload=bytes(range(16))
        self.assertEqual(c.send(bytes((0x64,0,16,ord('E')))+payload+b' '),b'\x14\x10')
        self.assertEqual(c.eeprom[addr:],payload)
        self.assertEqual(c.send(bytes((0x74,0,16,ord('E'),0x20))),b'\x14'+payload+b'\x10')
        bad=self.cpu()
        bad.load_address(bad.boot_start)
        bad.execute_until_error(bytes((0x64,1,0,ord('F')))+b'\x00'*256+b' ',bad.error_pc,limit=250000)
        self.assertEqual(bad.flash[bad.boot_start:],self.img)
        beyond=self.cpu()
        beyond.load_address(0xff80)
        beyond.execute_until_error(bytes((0x74,1,0,ord('F'),0x20)),beyond.error_pc)
    def test_bad_eop_never_erases_flash(self):
        c=self.cpu()
        addr=0xFA00
        c.load_address(addr)
        data=bytes(range(256))
        c.execute_until_error(bytes((0x64,1,0,ord('F')))+data+b'!',c.error_pc,limit=250000)
        self.assertEqual(c.flash[addr:addr+256],b'\xff'*256)
        self.assertEqual(c.erase_count,0)
    def test_8k_boot_fuse_and_absolute_jmp_zero(self):
        with tempfile.TemporaryDirectory() as folder:
            subprocess.run([sys.executable,str(ROOT/'build_mini.py'), '--mcu','atmega64a',
                            '--boot-bytes','8192','--output-dir',folder],check=True,
                            capture_output=True,text=True)
            data=(Path(folder)/'atmega64a.bin').read_bytes()
            self.assertEqual(len(data),8192)
            c=AVR64(data,mcusr=1)
            for _ in range(100):
                if c.app_started:break
                c.step()
            self.assertTrue(c.app_started)

    def test_wdt_and_reset_to_application(self):
        c=AVR64(self.img,mcusr=1)
        c.rxwait_pc=c.boot_start+symaddr(self.obj,'rx_wait')
        for _ in range(100):
            if c.app_started:break
            c.step()
        self.assertTrue(c.app_started)
        c=self.cpu()
        self.assertIn(0x0e,c.wdt_writes)

if __name__=='__main__':unittest.main()
