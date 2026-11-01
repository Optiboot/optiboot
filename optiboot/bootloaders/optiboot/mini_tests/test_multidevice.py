#!/usr/bin/env python3
"""Static, build and boundary checks for experimental multi-MCU Optiboot Mini ports."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
import build_mini as build

BASELINE='be531a3f0b7294c6e9f00f4d02a8c5e7ff28d8fb2b7c90cb6501d0294cfae00b'

def cli(*args,expect=0):
    p=subprocess.run([sys.executable,str(ROOT/'build_mini.py'),*map(str,args)],capture_output=True,text=True)
    if expect==0:
        assert p.returncode==0, p.stderr
    else:
        assert p.returncode!=0,p.stdout
    return p

class BuildTest(unittest.TestCase):
    def test_baseline_bit_identical(self):
        with tempfile.TemporaryDirectory() as t:
            cli('--output-dir',t,'--verify-baseline')
            data=(Path(t)/'atmega328p.bin').read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(),BASELINE)
            self.assertEqual(len(data),512)
    def test_all_devices_assemble(self):
        with tempfile.TemporaryDirectory() as t:
            for chip,dev in build.DEVICES.items():
                with self.subTest(chip=chip):
                    folder=Path(t)/chip
                    frequency = 8000000 if chip in ('attiny84','attiny85') else 12000000 if chip=='attiny1634' else 16000000
                    cli('--mcu',chip,'--f-cpu',frequency,'--output-dir',folder)
                    report=json.loads((folder/(chip+'.json')).read_text())
                    b=(folder/(chip+'.bin')).read_bytes()
                    self.assertEqual(report['flash_bytes'],dev['flash'])
                    self.assertEqual(report['flash_page_bytes'],dev['page'])
                    self.assertEqual(report['eeprom_bytes'],dev['eeprom'])
                    self.assertEqual(report['boot_bytes'],len(b))
                    self.assertEqual(int(report['boot_start_hex'],16),dev['flash']-len(b))
                    self.assertEqual(b[-12:-9],bytes(dev['sig']))
                    self.assertEqual(b[-9:-2],b'AVR STK')
                    self.assertEqual(b[-2:],b'\x03\x08')
                    self.assertLessEqual(report['code_bytes']+12,len(b))
                    self.assertEqual(report['boot_bytes']%dev['page'],0)
                    self.assertFalse(report['silicon_tested'])
                    hx=(folder/(chip+'.hex')).read_text().strip().splitlines()
                    decoded=[]; base=[]; upper=0
                    for rec in hx:
                        if rec==':00000001FF':break
                        bs=bytes.fromhex(rec[1:]);self.assertEqual(sum(bs)&255,0)
                        addr=(bs[1]<<8)|bs[2]
                        if bs[3]==4:
                            self.assertEqual(bs[0],2)
                            upper=(bs[4]<<8)|bs[5]
                            continue
                        self.assertEqual(bs[3],0)
                        decoded.extend(bs[4:-1]);base.extend(range((upper<<16)|addr,((upper<<16)|addr)+bs[0]))
                    self.assertEqual(bytes(decoded),b)
                    self.assertEqual(base[0],dev['flash']-len(b))
                    self.assertEqual(base[-1],dev['flash']-1)
    def test_legacy_512_boot_and_mega64_1024(self):
        with tempfile.TemporaryDirectory() as d:
            for chip in ('atmega16','atmega32a'):
                out=cli('--mcu',chip,'--output-dir',Path(d)/chip)
                self.assertIn('512 bytes',out.stdout)
                report=json.loads((Path(d)/chip/(chip+'.json')).read_text())
                self.assertEqual(report['code_bytes'],500)
                self.assertEqual(report['boot_bytes'],512)
            out=cli('--mcu','atmega64a','--output-dir',Path(d)/'mega64')
            self.assertIn('1024 bytes',out.stdout)
            cli('--mcu','atmega64a','--boot-bytes','512','--output-dir',Path(d)/'bad64',expect=1)
    def test_clock_matrix_and_uart_divisor(self):
        freqs=(1,2,4,8,10,12,16,20,24,25,28)
        for mhz in freqs:
            with self.subTest(mhz=mhz):
                f=mhz*1_000_000
                clock='internal_rc' if mhz<=8 else 'external_clock'
                with tempfile.TemporaryDirectory() as out:
                    a=['--f-cpu',f,'--clock-source',clock,'--output-dir',out]
                    if mhz>20:a+=['--allow-overclock']
                    cli(*a)
                    metadata=json.loads((Path(out)/'atmega328p.json').read_text())
                    self.assertEqual(metadata['f_cpu_hz'],f)
                    self.assertLess(abs(metadata['baud_error_percent']),2.5)
                    self.assertEqual(metadata['overclock'],mhz>20)
    def test_overclock_rejected_by_default(self):
        cli('--f-cpu',24_000_000,'--output-dir','/tmp/optiboot-invalid-test',expect=1)
        cli('--mcu','atmega16','--f-cpu',20_000_000,'--output-dir','/tmp/optiboot-invalid-test',expect=1)
        cli('--mcu','atmega328p','--f-cpu',28_000_000,'--output-dir','/tmp/optiboot-invalid-test',expect=1)
    def test_rc_mismatch_rejected(self):
        cli('--clock-source','internal_rc','--f-cpu',16_000_000,expect=1)
        cli('--clock-source','internal_rc','--f-cpu',10_000_000,expect=1)
    def test_baud_error_protected(self):
        cli('--f-cpu',8_000_000,'--baud','115200',expect=1)
    def test_128kb_boot_minimum_is_1024(self):
        for chip in ('atmega128a','atmega1284','atmega1284p'):
            with self.subTest(chip=chip):
                with tempfile.TemporaryDirectory() as out:
                    cli('--mcu',chip,'--output-dir',out)
                    report=json.loads((Path(out)/(chip+'.json')).read_text())
                    self.assertEqual(report['boot_bytes'],1024)
                    self.assertGreater(report['code_bytes'],512)
                    self.assertLess(report['code_bytes']+12,1024)
                    self.assertEqual(report['boot_start_hex'],'0x1FC00')
                    records=(Path(out)/(chip+'.hex')).read_text().splitlines()
                    self.assertEqual(records[0],':020000040001F9')
                cli('--mcu',chip,'--boot-bytes',512,expect=1)
        cli('--mcu','atmega1284a',expect=1)
        cli('--mcu','atmega128',expect=1)
    def test_clock_conversion(self):
        for f,b in ((16_000_000,115200),(8_000_000,38400),(12_000_000,115200)):
            q=build.best_baud(f,b)
            self.assertLess(abs(q['baud_error_percent']),2.5)
        q=build.best_baud(16_000_000,115200)
        self.assertEqual((q['u2x'],q['ubrr']),(1,16))
    def test_device_selection_from_compiler(self):
        txt=(ROOT/'devices'/'select.inc').read_text()
        for mcu in build.DEVICES:
            self.assertIn('#include "'+mcu+'.inc"',txt)

if __name__=='__main__':unittest.main(verbosity=2)
