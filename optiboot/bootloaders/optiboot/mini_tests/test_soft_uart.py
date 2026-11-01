"""Cycle-accounted AVR instruction emulator and software serial line model.

Models pin-level start/data/stop waveforms at CPU-cycle resolution (approximated
per AVR ISA), not physical oscillator tolerances or analog rise time.
"""
import sys,unittest,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HERE=Path(__file__).resolve().parent
sys.path[:0]=[str(ROOT),str(HERE)]
import build_mini
from test_tiny_virtual import TinyAVR
from test_legacy_uart import symaddr
sys.path.insert(0,str(ROOT/'tiny'))
from patch_vectors import rjmp


class SoftAVR(TinyAVR):
    def __init__(self,blob,dev,mcusr=2,baud=19200,f_cpu=8000000):
        super().__init__(blob,dev,mcusr)
        self.uart_status=-1
        self.uart_data=-2
        self.ccp_addr=-3
        self.wdt_addr=-4
        self.wdt_writes=[]
        self.io[0x41]=0                 # tiny85/84 WDTCR/CSR memory address
        self.pin_addr=0x36 if dev['profile']=='soft85' else 0x39
        self.ddr_addr=0x37 if dev['profile']=='soft85' else 0x3a
        self.port_addr=0x38 if dev['profile']=='soft85' else 0x3b
        self.rxbit=1 if dev['profile']=='soft85' else 2
        self.txbit=0 if dev['profile']=='soft85' else 1
        self.portlevel=None
        self.tx_edges=[]
        self.scheduled=[]
        self.f_cpu=f_cpu
        self.baud=baud
        self.bitcycles=f_cpu/baud
        self.input_bit_factor=1.0
        self.rx_end=0
    def rx_level(self):
        t=self.cycles
        for start,payload in self.scheduled:
            n=int((t-start)//(self.bitcycles*self.input_bit_factor))
            if t<start or n>=10*len(payload):continue
            c=payload[n//10];bit=n%10
            return 0 if bit==0 else 1 if bit==9 else (c>>(bit-1))&1
        return 1
    def io_read(self,addr):
        if addr==self.pin_addr:
            return (super().io_read(addr)&~(1<<self.rxbit))|(self.rx_level()<<self.rxbit)
        return super().io_read(addr)
    def io_write(self,addr,val):
        if addr==0x41:
            self.wdt_writes.append(val&255)
        # bypass TinyAVR CCP handling; old-AVR WDCE unlock applies
        # to WDTCSR on soft85/84.
        if addr in (self.ddr_addr,self.port_addr):
            super().io_write(addr,val)
            if self.io.get(self.ddr_addr,0)&(1<<self.txbit):
                level=(self.io.get(self.port_addr,0)>>self.txbit)&1
                if level != self.portlevel:
                    self.tx_edges.append((self.cycles,level))
                    self.portlevel=level
            return
        super().io_write(addr,val)
    def step(self):
        pc=self.pc;op=self.u16(pc)
        super().step()
        # Core emulator's cycles counter assumes 1 cycle/op. Adjust to AVR
        # documented costs for all opcodes encountered on the serial path.
        c=1
        if op&0xf000==0xc000:c=2
        elif op&0xf000==0xd000:c=3
        elif op==0x9508:c=4
        elif op&0xff00 in (0x9a00,0x9800):c=2 # SBI/CBI
        elif op&0xff00 in (0x9900,0x9b00):
            skipped=(self.pc-(pc+2))%len(self.flash)
            c=1 if skipped==0 else 2 if skipped==2 else 3
        elif op&0xfc00 in (0xf000,0xf400):
            c=1 if self.pc==(pc+2)%len(self.flash) else 2
        elif op&0xfe0f in (0x9000,0x9200,0x900d,0x920d,0x900f,0x920f):c=2
        elif op&0xff00 in (0x9600,0x9700):c=2
        elif op&0xfe0f in (0x9005,0x9007):c=3
        elif op==0x95e8:c=4
        elif op&0xfe08 in (0xfc00,0xfe00):
            skipped=(self.pc-(pc+2))%len(self.flash)
            c=1 if skipped==0 else 2 if skipped==2 else 3
        self.cycles+=c-1
    def wait_ready(self):
        for _ in range(400):
            if self.pc==self.rxwait_pc:return
            self.step()
        raise AssertionError('Bootloader did not initialize')
    def send_wave(self,data,max_instructions=1000000):
        start=max(self.cycles+int(self.bitcycles*5),self.rx_end+int(self.bitcycles*5))
        self.scheduled.append((start,bytes(data)))
        self.rx_end=start+int(len(data)*10*self.bitcycles*self.input_bit_factor)
        tx0=len(self.tx_edges)
        for n in range(max_instructions):
            if self.cycles>self.rx_end+int(12*self.bitcycles) and self.pc==self.rxwait_pc:
                return self.tx_edges[tx0:]
            self.step()
        raise AssertionError(f'No serial response: pc={self.pc:04x} cycles={self.cycles} rx_end={self.rx_end}')
    def decoded_transmit(self):
        edges=self.tx_edges
        bit=self.bitcycles
        def level_at(t):
            v=1
            for et,ev in edges:
                if et>t:break
                v=ev
            return v
        out=[];last=-1e100
        previous=1
        for t,level in edges:
            if previous==1 and level==0 and t>=last+9.75*bit:
                data=0
                for j in range(8):
                    data|=level_at(t+(1.5+j)*bit)<<j
                if level_at(t+9.5*bit)!=1:raise AssertionError(f'TX stop bit low at t={t}')
                out.append(data)
                last=t
            previous=level
        return bytes(out)


class SoftUARTTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory()
        cls.images={}
        for chip in ('attiny85','attiny84'):
            out=Path(cls.tmp.name)/chip
            subprocess.run([sys.executable,str(ROOT/'build_mini.py'),'--mcu',chip,
                '--f-cpu','8000000','--baud','19200','--clock-source','internal_rc',
                '--output-dir',str(out)],check=True,capture_output=True,text=True)
            cls.images[chip]=((out/(chip+'.bin')).read_bytes(),out/(chip+'.o'),build_mini.DEVICES[chip])
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def cpu(self,chip,mcusr=2):
        boot,obj,dev=self.images[chip]
        c=SoftAVR(boot,dev,mcusr)
        c.rxwait_pc=c.boot_start+symaddr(obj,'rx_wait')
        c.wait_ready()
        return c
    def test_simple_sync_and_signature_8n1(self):
        for chip in self.images:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                self.assertTrue(c.io.get(c.ddr_addr,0)&(1<<c.txbit))
                self.assertFalse(c.io.get(c.ddr_addr,0)&(1<<c.rxbit))
                c.send_wave(bytes([0x30,0x20]))
                self.assertEqual(c.decoded_transmit(),bytes([0x14,0x10]))
                c.send_wave(bytes([0x75,0x20]))
                self.assertEqual(c.decoded_transmit()[-5:],bytes([0x14,*self.images[chip][2]['sig'],0x10]))
    def test_program_page_64_and_readback(self):
        for chip in ('attiny85', 'attiny84'):
            with self.subTest(chip=chip):
                self._program_page_64_and_readback(chip)
    def _program_page_64_and_readback(self,chip):
        c=self.cpu(chip)
        target=c.boot_start-64
        word=target//2
        c.send_wave(bytes((0x55,word&255,word>>8,0x20)))
        self.assertEqual(c.decoded_transmit(),b'\x14\x10')
        payload=bytes((n*11+7)&255 for n in range(64))
        c.send_wave(bytes((0x64,0,64,ord('F')))+payload+b'\x20',max_instructions=3500000)
        self.assertEqual(c.flash[target:target+64],payload)
        self.assertEqual(c.decoded_transmit()[-2:],b'\x14\x10')
        c.send_wave(bytes((0x55,word&255,word>>8,0x20)))
        c.send_wave(bytes((0x74,0,64,ord('F'),0x20)),max_instructions=2500000)
        self.assertEqual(c.decoded_transmit()[-66:],b'\x14'+payload+b'\x10')
    def test_16_byte_eeprom_write_and_read(self):
        for chip in self.images:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                addr=480;word=addr//2
                c.send_wave(bytes((0x55,word&255,word>>8,0x20)))
                payload=bytes((i*17+13)&255 for i in range(16))
                c.send_wave(bytes((0x64,0,16,ord('E')))+payload+b'\x20')
                self.assertEqual(c.eeprom[addr:addr+16],payload)
                c.send_wave(bytes((0x74,0,16,ord('E'),0x20)))
                self.assertEqual(c.decoded_transmit()[-18:],b'\x14'+payload+b'\x10')
    def test_rx_host_bit_timing_plus_minus_1_5_percent(self):
        for chip in self.images:
            for drift in (-0.015,0.015):
                with self.subTest(chip=chip,drift=drift):
                    c=self.cpu(chip)
                    c.input_bit_factor=1+drift
                    c.send_wave(b'\x30\x20')
                    self.assertEqual(c.decoded_transmit(),b'\x14\x10')
    def test_1mhz_and_16mhz_baud_profiles(self):
        for clock,baud,source in ((1000000,9600,'internal_rc'),(16000000,38400,'external_clock')):
            with self.subTest(clock=clock):
                chip='attiny85'; dev=build_mini.DEVICES[chip]
                out=Path(self.tmp.name)/f'{chip}_{clock}'
                subprocess.run([sys.executable,str(ROOT/'build_mini.py'),'--mcu',chip,
                    '--f-cpu',str(clock),'--baud',str(baud),'--clock-source',source,
                    '--output-dir',str(out)],check=True,capture_output=True,text=True)
                c=SoftAVR((out/(chip+'.bin')).read_bytes(),dev,baud=baud,f_cpu=clock)
                c.rxwait_pc=c.boot_start+symaddr(out/(chip+'.o'),'rx_wait')
                c.wait_ready()
                c.send_wave(b'\x30\x20')
                self.assertEqual(c.decoded_transmit(),b'\x14\x10')
    def test_read_only_boot_and_bad_packet_eop_no_flash_erase(self):
        chip='attiny85';c=self.cpu(chip)
        address=c.boot_start-64;w=address//2
        c.send_wave(bytes((0x55,w&255,w>>8,0x20)))
        invalid=bytes((0x64,0,64,ord('F')))+bytes([0x7a]*64)+b'!'
        start=max(c.cycles+int(c.bitcycles*5),c.rx_end+int(c.bitcycles*5))
        c.scheduled.append((start,invalid))
        c.rx_end=start+int(len(invalid)*10*c.bitcycles)
        error_pc=c.boot_start+symaddr(self.images[chip][1],'error_loop')
        for _ in range(600000):
            c.step()
            if c.pc==error_pc:break
        self.assertEqual(c.pc,error_pc)
        self.assertEqual(c.erase_count,0)
        self.assertEqual(c.flash[address:address+64],b'\xff'*64)

    def test_transmit_alternating_bits_timing(self):
        for chip in self.images:
            with self.subTest(chip=chip):
                c=self.cpu(chip)
                obj=self.images[chip][1]
                c.pc=c.boot_start+symaddr(obj,'soft_uart_tx')
                c.reg[16]=0x55
                c.stack.append(c.rxwait_pc)
                old=len(c.tx_edges)
                for _ in range(5000):
                    if c.pc==c.rxwait_pc:break
                    c.step()
                edges=c.tx_edges[old:]
                self.assertEqual([x[1] for x in edges], [0,1,0,1,0,1,0,1,0,1])
                gaps=[edges[j][0]-edges[j-1][0] for j in range(1,len(edges))]
                # Per-edge timing includes branch-balance jitter and 2-cycle
                # SBI/CBI GPIO instruction latency, not analog pin rise time.
                self.assertTrue(all(abs(g-c.bitcycles) <= c.bitcycles*0.02 for g in gaps),gaps)

    def test_virtual_boot_application_on_power_on(self):
        for chip in self.images:
            with self.subTest(chip=chip):
                boot,obj,dev=self.images[chip]
                c=SoftAVR(boot,dev,mcusr=1)
                vector=6 if chip=='attiny85' else 14
                app=0x0100
                op=rjmp(vector,app//2,dev['flash']//2)
                c.flash[vector*2:vector*2+2]=bytes((op&255,op>>8))
                for i in range(150):
                    if c.pc==app:break
                    c.step()
                self.assertEqual(c.pc,app)

if __name__=='__main__':unittest.main()
