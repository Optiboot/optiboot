"""Experimental tiny841/1634 instruction-level checks. Not a silicon timing emulator.

ATtiny1634 uses *four 32-byte pages per erase*, unlike ordinary classic AVR;
model the four-page erased block and validate that all data survive.
"""
import sys,subprocess,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HERE=Path(__file__).resolve().parent
sys.path[:0]=[str(ROOT),str(HERE)]
import build_mini
from test_128k import AVR128
from test_legacy_uart import symaddr
sys.path.insert(0,str(ROOT/"tiny"))
from patch_vectors import rjmp

class TinyAVR(AVR128):
    def __init__(self,boot,dev,mcusr=2):
        super().__init__(boot,False,mcusr)
        self.flash_size=dev['flash']
        self.boot_start=dev['flash']-len(boot)
        self.flash=bytearray(b'\xff'*self.boot_start+boot)
        self.pc=self.boot_start
        self.io={0x54:mcusr, 0x55:mcusr,0x57:0,0x3c:0,0x3f:0,0x30:0,0x41:0}
        self.eeprom=bytearray([255]*dev['eeprom'])
        self.spmcsr=0x57
        self.eecr=0x3c;self.eedr=0x3d;self.eearl=0x3e;self.eearh=0x3f
        self.profile=dev['profile']
        self.page_size=dev['page']
        self.uart_status=0x86 if self.profile=='tiny841' else 0x46
        self.uart_data=0x80 if self.profile=='tiny841' else 0x40
        self.ccp_addr=0x71 if self.profile=='tiny841' else 0x4f
        self.wdt_addr=0x41 if self.profile=='tiny841' else 0x50
        self.ccp_armed=False
        self.ccp_keys=[]
        self.group_erases=[]
        self.page_writes=[]
    def io_read(self,addr):
        if addr==self.uart_status:
            return 0x20|(0x80 if self.rx else 0)
        if addr==self.uart_data:
            if not self.rx:raise AssertionError('UDR read without RXC')
            return self.rx.popleft()
        return super().io_read(addr)
    def io_write(self,addr,val):
        if addr==self.uart_data:
            self.tx.append(val&255)
            self.io[addr]=val&255
            return
        if addr==self.ccp_addr:
            self.ccp_armed=val==0xD8
            self.ccp_keys.append(val)
            self.io[addr]=val
            return
        if addr==self.wdt_addr:
            if not self.ccp_armed:raise AssertionError('WDT change without CCP unlock')
            self.ccp_armed=False
            self.wdt_writes.append(val)
        super().io_write(addr,val)
    def spm(self):
        cmd=self.io[self.spmcsr]
        addr=self.z()
        page=addr&~(self.page_size-1)
        if cmd==1:
            self.page_buf[addr&(self.page_size-1)]=self.reg[0]
            self.page_buf[(addr+1)&(self.page_size-1)]=self.reg[1]
        elif cmd==3:
            pages=4 if self.profile=='tiny1634' else 1
            group=page&~(self.page_size*pages-1)
            if group>=self.boot_start:raise AssertionError('Boot erase')
            self.flash[group:group+self.page_size*pages]=b'\xff'*(self.page_size*pages)
            self.group_erases.append(group)
            self.erase_count+=1
        elif cmd==5:
            if page>=self.boot_start:raise AssertionError('Boot page write')
            if not self.page_buf:raise AssertionError('Empty hardware page buffer')
            for offset,v in self.page_buf.items():self.flash[page+offset]=v
            self.page_buf.clear();self.write_count+=1
            self.page_writes.append(page)
        elif cmd==0x11:
            pass
        else:raise AssertionError(f'Unexpected SPM command {cmd:02X}')
        self.io[self.spmcsr]=0
    def step(self):
        super().step()
        self.pc%=self.flash_size

class TinyVirtualTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory()
        cls.boots={}
        for chip in ('attiny841','attiny1634'):
            o=Path(cls.tmp.name)/chip
            flags=['--mcu',chip,'--output-dir',str(o)]
            if chip=='attiny1634':flags+=['--f-cpu','12000000']
            subprocess.run([sys.executable,str(ROOT/'build_mini.py'),*flags],capture_output=True,text=True,check=True)
            cls.boots[chip]=((o/(chip+'.bin')).read_bytes(),o/(chip+'.o'),build_mini.DEVICES[chip])
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def cpu(self,chip):
        boot,obj,dev=self.boots[chip]
        c=TinyAVR(boot,dev)
        c.rxwait_pc=c.boot_start+symaddr(obj,'rx_wait')
        c.error_pc=c.boot_start+symaddr(obj,'error_loop')
        c.wait_ready()
        return c
    def test_tiny_uart_ccp_and_signature(self):
        for chip in self.boots:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                self.assertIn(0xD8,c.ccp_keys)
                self.assertIn(0x0E,c.wdt_writes)
                self.assertEqual(c.send(b'1 '),b'\x14AVR STK\x10')
                self.assertEqual(c.send(b'u '),b'\x14'+bytes(self.boots[chip][2]['sig'])+b'\x10')
    def test_tiny841_write_and_read_page(self):
        c=self.cpu('attiny841')
        page=c.boot_start-16
        b=bytes((i*3+11)&255 for i in range(16))
        self.assertEqual(c.load_address(page),b'\x14\x10')
        self.assertEqual(c.write_flash(b),b'\x14\x10')
        self.assertEqual(c.flash[page:page+16],b)
        self.assertEqual(c.load_address(page),b'\x14\x10')
        self.assertEqual(c.read_flash(16),b'\x14'+b+b'\x10')
    def test_tiny1634_group_erase_and_four_page_writes(self):
        c=self.cpu('attiny1634')
        group=c.boot_start-128
        payload=bytes((i*17+23)&255 for i in range(128))
        self.assertEqual(c.load_address(group),b'\x14\x10')
        self.assertEqual(c.write_flash(payload),b'\x14\x10')
        self.assertEqual(c.flash[group:group+128],payload)
        self.assertEqual(c.group_erases,[group])
        self.assertEqual(c.page_writes,[group+96,group+64,group+32,group])
        self.assertEqual(c.load_address(group),b'\x14\x10')
        self.assertEqual(c.read_flash(128),b'\x14'+payload+b'\x10')
    def test_power_on_reset_jumps_through_relocated_vector(self):
        for chip,(boot,obj,dev) in self.boots.items():
            with self.subTest(chip=chip):
                c=TinyAVR(boot,dev,mcusr=1)
                i=14 if chip=='attiny841' else 26
                app_start=0x100
                opcode=rjmp(i,app_start//2,dev['flash']//2)
                c.flash[i*2:i*2+2]=bytes((opcode&255,opcode>>8))
                for _ in range(100):
                    if c.pc==app_start:break
                    c.step()
                self.assertEqual(c.pc,app_start)
    def test_eeprom_write_read(self):
        for chip in self.boots:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                offset=len(c.eeprom)-16
                data=bytes((i*13+5)&255 for i in range(16))
                c.load_address(offset)
                self.assertEqual(c.send(bytes((0x64,0,16,ord('E')))+data+b' '),b'\x14\x10')
                self.assertEqual(c.eeprom[offset:offset+16],data)
                self.assertEqual(c.send(bytes((0x74,0,16,ord('E'),0x20))),b'\x14'+data+b'\x10')
    def test_tiny1634_bad_frame_does_not_erase_group(self):
        c=self.cpu('attiny1634')
        group=c.boot_start-128
        c.load_address(group)
        c.execute_until_error(bytes((0x64,0,128,ord('F')))+bytes(128)+b'!',c.error_pc,limit=50000)
        self.assertEqual(c.erase_count,0)
        self.assertEqual(c.flash[group:group+128],b'\xff'*128)
if __name__=='__main__':unittest.main()
