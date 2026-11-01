"""Dynamic smoke tests for all modern-UART ATmega88/168/164/324/328 variants.

The legacy tests used a 128-byte 328P SPM model; this test uses each chip's
real Flash/EEPROM/page geometry and executes its own compiled image.
"""
import subprocess, sys, tempfile, unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT));sys.path.insert(0,str(HERE))
import build_mini
from test_128k import AVR128
from test_legacy_uart import symaddr

CHIPS=tuple(k for k,v in build_mini.DEVICES.items() if v['profile']=='modern')

class AVRModern(AVR128):
    def __init__(self, boot, flash_size, page_size, eeprom_size, mcusr=2):
        super().__init__(boot.rjust(1024,b'\xff'),False,mcusr)
        self.flash_size=flash_size
        self.boot_start=flash_size-len(boot)
        self.flash=bytearray(b'\xff'*self.boot_start+boot)
        self.pc=self.boot_start
        self.page_size=page_size
        self.eeprom=bytearray(b'\xff'*eeprom_size)
        self.spmcsr=0x57
        self.fuse=(0x62,0xff,0xff,0xd9)
    def spm(self):
        cmd=self.io[self.spmcsr]
        addr=self.z()
        page=addr&~(self.page_size-1)
        if cmd==1:
            self.page_buf[addr&(self.page_size-1)]=self.reg[0]
            self.page_buf[(addr+1)&(self.page_size-1)]=self.reg[1]
        elif cmd==3:
            if page>=self.boot_start:raise AssertionError('Attempted boot erase')
            self.flash[page:page+self.page_size]=b'\xff'*self.page_size
            self.erase_count+=1
        elif cmd==5:
            if page>=self.boot_start:raise AssertionError('Attempted boot write')
            for offset,value in self.page_buf.items():self.flash[page+offset]=value
            self.page_buf={};self.write_count+=1
        elif cmd==0x11:pass
        else:raise AssertionError(f'Bad SPMCSR {cmd:x}')
        self.io[self.spmcsr]=0

class TestModernFamily(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory()
        cls.data={}
        for chip in CHIPS:
            out=Path(cls.tmp.name)/chip
            subprocess.run([sys.executable,str(ROOT/'build_mini.py'),'--mcu',chip,'--output-dir',str(out)],check=True,capture_output=True,text=True)
            cls.data[chip]=((out/(chip+'.bin')).read_bytes(),out/(chip+'.o'),build_mini.DEVICES[chip])
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def cpu(self,chip,mcusr=2):
        boot,obj,dev=self.data[chip]
        c=AVRModern(boot,dev['flash'],dev['page'],dev['eeprom'],mcusr)
        c.rxwait_pc=c.boot_start+symaddr(obj,'rx_wait')
        c.error_pc=c.boot_start+symaddr(obj,'error_loop')
        c.wait_ready()
        return c
    def test_all_modern_devices_flash_and_eeprom(self):
        for chip in CHIPS:
            with self.subTest(chip=chip):
                boot,obj,dev=self.data[chip]
                c=self.cpu(chip)
                self.assertEqual(c.io[0xc1],0x18)
                self.assertEqual(c.io[0xc2],0x06)
                self.assertEqual(c.io[0xc4],16)
                self.assertEqual(c.send(b'1 '),b'\x14AVR STK\x10')
                self.assertEqual(c.send(b'u '),b'\x14'+bytes(dev['sig'])+b'\x10')
                last=c.boot_start-dev['page']
                payload=bytes((i*31+last//256)%256 for i in range(dev['page']))
                self.assertEqual(c.load_address(last),b'\x14\x10')
                self.assertEqual(c.write_flash(payload),b'\x14\x10')
                self.assertEqual(c.flash[last:last+dev['page']],payload)
                self.assertEqual(c.load_address(last),b'\x14\x10')
                self.assertEqual(c.read_flash(dev['page']),b'\x14'+payload+b'\x10')
                offset=dev['eeprom']-16
                ep=bytes((i*7+9)&255 for i in range(16))
                c.load_address(offset)
                self.assertEqual(c.send(bytes((0x64,0,16,ord('E')))+ep+b' '),b'\x14\x10')
                self.assertEqual(c.eeprom[offset:offset+16],ep)
                self.assertEqual(c.send(bytes((0x74,0,16,ord('E'),0x20))),b'\x14'+ep+b'\x10')
                self.assertEqual(c.flash[c.boot_start:],boot)
    def test_bad_packet_and_boot_write_are_rejected(self):
        for chip in CHIPS:
            with self.subTest(chip=chip):
                _,_,dev=self.data[chip]
                c=self.cpu(chip)
                start=c.boot_start-dev['page']
                c.load_address(start)
                pay=bytes(dev['page'])
                c.execute_until_error(bytes((0x64,0,len(pay),ord('F')))+pay+b'!' if len(pay)<256 else bytes((0x64,1,0,ord('F')))+pay+b'!',c.error_pc,limit=150000)
                self.assertEqual(c.erase_count,0)
                self.assertEqual(c.flash[start:start+dev['page']],b'\xff'*dev['page'])

if __name__=='__main__':unittest.main()
