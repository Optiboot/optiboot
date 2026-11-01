#!/usr/bin/env python3
"""Instruction-level smoke tests for legacy USART and 512-byte boot images.

This is deliberately not a physical-chip timing emulator. It decodes AVR
instructions, executes complete STK500v1 transactions and models the old
USART/EEPROM/SPM I/O register addresses independently of the source macros.
"""
from collections import deque
from pathlib import Path
import random
import re
import subprocess
import sys
import unittest

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
sys.path.insert(0, str(HERE))
from avr_base_sim import AVR


def symaddr(obj, name):
    s = subprocess.check_output(['llvm-objdump', '-t', str(obj)], text=True)
    hits = re.findall(r'^([0-9a-f]+)\s+\S+\s+\.text\s+\S+\s+' + re.escape(name) + r'$', s, re.M)
    if len(hits) != 1:
        raise AssertionError(f'Missing unique symbol {name} in {obj}')
    return int(hits[0], 16)


class LegacyAVR(AVR):
    def __init__(self, boot, flash_size, sig, mcusr=2):
        super().__init__(boot[-512:], mcusr=mcusr)  # initialize common state, then use real image
        self.boot_start = flash_size - len(boot)
        self.flash_size = flash_size
        self.flash = bytearray(b'\xff' * self.boot_start + boot)
        self.pc = self.boot_start
        self.io = {0x54: mcusr, 0x57: 0, 0x41: 0, 0x3C: 0, 0x40: 0, 0x2B: 0}
        self.eeprom = bytearray(b'\xff' * (512 if flash_size == 16384 else 1024))
        self.sig = sig
        self.last_fe = False
        self.sp_writes = []
        self.ucsrc = None
        self.ubrrh = None
        self.ubrrl = None
        self.wdt_settings = []
        self.eempe_armed = False
        self.ee_sequence = []
        self.fuse = (0x62, 0xFF, 0xFF, 0xD9)

    def io_read(self, addr):
        if addr == 0x2B:  # UCSRA: RXC, FE, UDRE
            return 0x20 | (0x80 if self.rx else 0) | (0x10 if self.last_fe and self.rx else 0)
        if addr == 0x2C:  # UDR
            if not self.rx:
                raise RuntimeError('Received without RXC')
            return self.rx.popleft()
        if addr == 0x3D:
            return self.io.get(addr, 0)
        return super().io_read(addr)

    def io_write(self, addr, val):
        val &= 255
        if addr == 0x2C:
            self.tx.append(val)
        elif addr == 0x40:  # UCSRC/UBRRH share I/O 0x20, URSEL disambiguates writes
            if val & 0x80:
                self.ucsrc = val
            else:
                self.ubrrh = val & 15
        elif addr == 0x29:
            self.ubrrl = val
        elif addr in (0x5D, 0x5E):
            self.sp_writes.append((addr, val))
        elif addr == 0x3C:  # EECR: EERE, EEPE, EEMPE
            if val & 0x04:
                self.eempe_armed = True
                self.ee_sequence.append('EEMPE')
            if val & 0x02:
                if not self.eempe_armed:
                    raise AssertionError('EEPROM write without EEMPE')
                self.eempe_armed = False
                i = self.io.get(0x3E, 0) | (self.io.get(0x3F, 0) << 8)
                self.eeprom[i] = self.io.get(0x3D, 0)
                self.ee_sequence.append('EEPE')
            if val & 0x01:
                i = self.io.get(0x3E, 0) | (self.io.get(0x3F, 0) << 8)
                self.io[0x3D] = self.eeprom[i]
            self.io[addr] = 0  # hardware clears flags after instant simulated completion
            return
        elif addr == 0x41:
            self.wdt_settings.append(val)
        self.io[addr] = val

    def step(self):
        # The imported 328P smoke simulator does not implement SBIC, used
        # by our legacy EEPE wait loop; emulate its exact skip semantics.
        op = self.u16(self.pc)
        if op & 0xFF00 == 0x9900:
            addr = 0x20 + ((op >> 3) & 0x1F)
            bit = op & 7
            skip = not (self.io_read(addr) & (1 << bit))
            self.pc = (self.pc + (4 if skip else 2)) % self.flash_size
            self.cycles += 1
            return
        # Model read-only fuse/lock access via SPMCSR.BLSET + LPM.
        if op & 0xFE0F == 0x9004 and self.io.get(0x57) == 9:  # LPM Rd,Z (r16)
            self.reg[16] = self.fuse[self.z() & 3]
            self.io[0x57] = 0
            self.pc = (self.pc + 2) % self.flash_size
            self.cycles += 1
            return
        if self.pc >= self.flash_size:
            self.pc = 0
        super().step()
        self.pc %= self.flash_size

    def wait_ready(self, max_steps=200):
        for _ in range(max_steps):
            if self.pc == self.rxwait_pc:
                return
            self.step()
        raise AssertionError('Boot did not reach UART RX wait')

    def send(self, command, limit=1000000):
        self.rx.extend(command)
        start = len(self.tx)
        for n in range(limit):
            if n and not self.rx and self.pc == self.rxwait_pc:
                return bytes(self.tx[start:])
            self.step()
        raise AssertionError('Transaction timed out')


class LegacyUARTTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.outputs = {}
        for chip, size, sig in [('atmega16', 16384, (0x1E, 0x94, 0x03)),
                                ('atmega32a', 32768, (0x1E, 0x95, 0x02))]:
            d = ROOT / 'output' / chip
            d.mkdir(parents=True, exist_ok=True)
            builder = ROOT / 'build_mini.py' if (ROOT / 'build_mini.py').exists() else ROOT / 'build.py'
            subprocess.run([sys.executable, str(builder), '--mcu', chip,
                            '--output-dir', str(d), '--boot-bytes', '512'],
                           check=True, capture_output=True, text=True)
            cls.outputs[chip] = (d / (chip + '.bin'), d / (chip + '.o'), size, sig)

    def make_cpu(self, chip, mcusr=2):
        binpath, obj, flash_size, sig = self.outputs[chip]
        img = binpath.read_bytes()
        cpu = LegacyAVR(img, flash_size, sig, mcusr)
        cpu.rxwait_pc = cpu.boot_start + symaddr(obj, 'rx_wait')
        cpu.wait_ready()
        return cpu

    def test_512_byte_address_and_uart_setup(self):
        for chip in self.outputs:
            with self.subTest(chip=chip):
                c = self.make_cpu(chip)
                self.assertEqual(c.flash_size - c.boot_start, 512)
                self.assertEqual(c.ucsrc, 0x86)
                self.assertEqual(c.ubrrh, 0)
                self.assertEqual(c.ubrrl, 16)
                self.assertIn(0x0E, c.wdt_settings)
                self.assertEqual(c.sp_writes[:2], [(0x5D, 0x5F), (0x5E, 0x04 if chip == 'atmega16' else 0x08)])

    def test_sync_version_signature_and_signon(self):
        for chip, (_,_,size,sig) in self.outputs.items():
            with self.subTest(chip=chip):
                c = self.make_cpu(chip)
                self.assertEqual(c.send(b'0 '), b'\x14\x10')
                self.assertEqual(c.send(b'1 '), b'\x14AVR STK\x10')
                self.assertEqual(c.send(b'A\x81 '), b'\x14\x08\x10')
                self.assertEqual(c.send(b'A\x82 '), b'\x14\x03\x10')
                self.assertEqual(c.send(b'u '), b'\x14'+bytes(sig)+b'\x10')

    def test_universal_and_eeprom_interleave(self):
        for chip in self.outputs:
            with self.subTest(chip=chip):
                c=self.make_cpu(chip)
                self.assertEqual(c.send(bytes((0x56,0x50,0,0,0,0x20))), b'\x14\x62\x10')
                self.assertEqual(c.send(bytes((0x56,0x58,8,0,0,0x20))), b'\x14\xd9\x10')
                offset=len(c.eeprom)-16
                self.assertEqual(c.send(bytes((0x55,offset//2 & 255,offset//512,0x20))), b'\x14\x10')
                payload=bytes(range(1,17))
                self.assertEqual(c.send(bytes((0x64,0,16,0x45))+payload+b' '),b'\x14\x10')
                self.assertEqual(c.eeprom[offset:],payload)
                self.assertEqual(c.send(bytes((0x74,0,16,0x45,0x20))),b'\x14'+payload+b'\x10')
                self.assertEqual(len(c.ee_sequence),32)
                self.assertEqual(c.send(b'1 '),b'\x14AVR STK\x10')

    def test_flash_write_read_last_user_page(self):
        for chip in self.outputs:
            with self.subTest(chip=chip):
                c=self.make_cpu(chip)
                page=c.boot_start-128
                self.assertEqual(c.send(bytes((0x55,page//2 & 255,page//512,0x20))),b'\x14\x10')
                b=bytes(random.Random(page).randrange(0,256) for _ in range(128))
                self.assertEqual(c.send(bytes((0x64,0,128,ord('F')))+b+b' '),b'\x14\x10')
                self.assertEqual(c.flash[page:page+128],b)
                self.assertEqual(c.send(bytes((0x74,0,128,ord('F'),0x20))),b'\x14'+b+b'\x10')

    def test_rx_fe_does_not_refresh_wdt(self):
        for chip in self.outputs:
            with self.subTest(chip=chip):
                c=self.make_cpu(chip)
                start=c.wdr_count
                c.last_fe=True
                self.assertEqual(c.send(b'0 '),b'\x14\x10')
                self.assertEqual(c.wdr_count,start)
                c.last_fe=False
                self.assertEqual(c.send(b'0 '),b'\x14\x10')
                self.assertGreater(c.wdr_count,start)

    def test_old_1024_vs_new_512_response_equivalence(self):
        """Same valid avrdude packets produce identical UART and memory results."""
        older = HERE / 'fixtures'
        for chip, (_, _, size, sig) in self.outputs.items():
            with self.subTest(chip=chip):
                new = self.make_cpu(chip)
                oldobj = older / (chip + '.o')
                oldimg = (older / (chip + '.bin')).read_bytes()
                old = LegacyAVR(oldimg, size, sig)
                old.rxwait_pc = old.boot_start + symaddr(oldobj, 'rx_wait')
                old.wait_ready()
                page = 0x0400
                ep = 0x40
                flash_payload = bytes((i * 11 + 23) & 255 for i in range(128))
                ep_payload = bytes((i * 7 + 1) & 255 for i in range(16))
                cmds = [
                    b'0 ', b'1 ', b'A\x81 ', b'A\x82 ',
                    bytes((0x42,)) + bytes(range(20)) + b' ',
                    bytes((0x45,)) + bytes(range(5)) + b' ',
                    b'u ', bytes((0x56,0x50,0,0,0,0x20)),
                    bytes((0x55, page >> 1 & 255, page >> 9 & 255, 0x20)),
                    bytes((0x64, 0, 128, ord('F'))) + flash_payload + b' ',
                    bytes((0x74, 0, 128, ord('F'), 0x20)),
                    bytes((0x55, ep >> 1, 0, 0x20)),
                    bytes((0x64, 0, 16, ord('E'))) + ep_payload + b' ',
                    bytes((0x74, 0, 16, ord('E'), 0x20)), b'1 ',
                ]
                for command in cmds:
                    expected = old.send(command)
                    self.assertEqual(new.send(command), expected, (chip, command[:8]))
                self.assertEqual(old.flash[page:page+128], new.flash[page:page+128])
                self.assertEqual(old.eeprom, new.eeprom)

    def test_invalid_eop_does_not_erase_flash(self):
        for chip in self.outputs:
            with self.subTest(chip=chip):
                c = self.make_cpu(chip)
                page=0x400
                c.flash[page:page+128]=bytes((0xA5,))*128
                c.send(bytes((0x55, page>>1 & 255, page>>9, 0x20)))
                c.rx.extend(bytes((0x64,0,8,ord('F')))+bytes(range(8))+b'\x00')
                for _ in range(3000):
                    c.step()
                self.assertEqual(c.flash[page:page+128],bytes((0xA5,))*128)
                self.assertEqual(c.erase_count,0)

    def test_reset_to_app(self):
        for chip, (bf, ob, fs, sig) in self.outputs.items():
            with self.subTest(chip=chip):
                c=LegacyAVR(bf.read_bytes(),fs,sig,mcusr=1)
                c.rxwait_pc=c.boot_start+symaddr(ob,'rx_wait')
                for _ in range(150):
                    if c.app_started:
                        break
                    c.step()
                self.assertTrue(c.app_started)


if __name__ == '__main__':
    unittest.main(verbosity=2)
