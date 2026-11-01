"""AVRDUDE 2022 STK500v1 EEPROM address-convention regression tests.

Old avrdude sent EEPROM addresses as bytes for ATmega64A/128A; newer
versions use word addresses for all supported classic AVR devices.
The opt-in build variant must not change the stock modern behavior.
This is instruction-level emulation, not a serial/hardware experiment.
"""
from pathlib import Path
import json
import subprocess
import sys
import tempfile
import unittest
HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
sys.path.insert(0,str(HERE))
from test_128k import AVR128
from test_mega64_audit import AVR64
from test_legacy_uart import symaddr

class TestOldAvrdude(unittest.TestCase):
    def build(self,chip,old):
        d=tempfile.TemporaryDirectory()
        cmd=[sys.executable,str(ROOT/'build_mini.py'),'--mcu',chip,'--output-dir',d.name]
        if old:cmd+=['--legacy-eeprom-byte-address']
        r=subprocess.run(cmd,capture_output=True,text=True)
        self.assertEqual(r.returncode,0,r.stderr)
        img=(Path(d.name)/(chip+'.bin')).read_bytes()
        obj=Path(d.name)/(chip+'.o')
        manifest=json.loads((Path(d.name)/(chip+'.json')).read_text())
        self.assertEqual(manifest['eeprom_host_addressing'],'byte' if old else 'word')
        self.assertEqual(len(img),1024)
        sim=AVR64(img) if chip=='atmega64a' else AVR128(img,True)
        sim.rxwait_pc=sim.boot_start+symaddr(obj,'rx_wait')
        sim.error_pc=sim.boot_start+symaddr(obj,'error_loop')
        sim.wait_ready()
        return d,sim,manifest

    def test_byte_protocol_old_client_on_classic_64_128(self):
        for chip,size in (('atmega64a',2048),('atmega128a',4096)):
            with self.subTest(chip=chip):
                folder,c,manifest=self.build(chip,True)
                try:
                    for offset in (1,239,size-16):
                        payload=bytes((i+offset*7)&255 for i in range(16))
                        self.assertEqual(c.send(bytes((0x55,offset&255,offset>>8,0x20))),b'\x14\x10')
                        self.assertEqual(c.send(bytes((0x64,0,16,0x45))+payload+b' '),b'\x14\x10')
                        self.assertEqual(c.eeprom[offset:offset+16],payload)
                        # No second LOAD_ADDRESS: the byte/word translation must not accumulate.
                        self.assertEqual(c.send(bytes((0x74,0,16,0x45,0x20))),b'\x14'+payload+b'\x10')
                        flash_at=2*offset
                        flash_expected=bytes((flash_at+i)&255 for i in range(16))
                        c.flash[flash_at:flash_at+16]=flash_expected
                        self.assertEqual(c.send(bytes((0x74,0,16,0x46,0x20))),b'\x14'+flash_expected+b'\x10')
                    self.assertEqual(c.flash[c.boot_start:],(Path(folder.name)/(chip+'.bin')).read_bytes())
                finally:folder.cleanup()

    def test_default_mode_word_addresses(self):
        for chip,size in (('atmega64a',2048),('atmega128a',4096)):
            with self.subTest(chip=chip):
                folder,c,_=self.build(chip,False)
                try:
                    addr=size-16
                    c.load_address(addr)
                    data=b'abcdefghijklmnop'
                    self.assertEqual(c.send(bytes((0x64,0,16,0x45))+data+b' '),b'\x14\x10')
                    self.assertEqual(c.eeprom[addr:addr+16],data)
                finally:folder.cleanup()

    def test_only_classic_64_128_are_eligible(self):
        with tempfile.TemporaryDirectory() as d:
            r=subprocess.run([sys.executable,str(ROOT/'build_mini.py'),'--mcu','atmega1284p',
                '--legacy-eeprom-byte-address','--output-dir',d],capture_output=True,text=True)
            self.assertNotEqual(r.returncode,0)
            self.assertIn('only supported',r.stderr)

if __name__=='__main__':unittest.main()
