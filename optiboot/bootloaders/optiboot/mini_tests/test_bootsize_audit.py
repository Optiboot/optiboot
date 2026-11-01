"""Cross-family BOOTSZ and Intel HEX regression checks based on datasheet tables."""
from pathlib import Path
import json, sys, subprocess, tempfile, unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
import build_mini

class TestBootFuseSizes(unittest.TestCase):
    def test_all_known_hardware_ranges(self):
        for chip, profile in build_mini.DEVICES.items():
            with self.subTest(chip=chip):
                flash=profile['flash']
                maxboot=2048 if flash <=16384 else 4096 if flash <=32768 else 8192
                allowed=[maxboot//8, maxboot//4, maxboot//2, maxboot]
                if profile.get('virtual_boot'):
                    # Virtual Boot ATtiny has no BOOTSZ fuses; 1024 is a
                    # reserved Flash region rather than a hardware section.
                    self.assertEqual(profile['default_boot'],1024)
                else:
                    self.assertIn(profile['default_boot'],allowed)
                    self.assertLessEqual(profile['default_boot'],maxboot)
                self.assertEqual(len(allowed),4)
    def test_unsupported_fuse_sizes_rejected(self):
        for chip,bad in [('atmega16',4096),('atmega88',4096),('atmega168',8192),('atmega328p',8192),('atmega32a',8192)]:
            with self.subTest(chip=chip),tempfile.TemporaryDirectory() as t:
                p=subprocess.run([sys.executable,str(ROOT/'build_mini.py'),'--mcu',chip,'--boot-bytes',str(bad),'--output-dir',t],capture_output=True,text=True)
                self.assertNotEqual(p.returncode,0)
                self.assertIn('not allowed',p.stderr)
    def test_1kb_fuse_manifest_for_128k(self):
        for chip in ('atmega128a','atmega1284','atmega1284p'):
            with self.subTest(chip=chip),tempfile.TemporaryDirectory() as t:
                p=subprocess.run([sys.executable,str(ROOT/'build_mini.py'),'--mcu',chip,'--output-dir',t],capture_output=True,text=True)
                self.assertEqual(p.returncode,0,p.stderr)
                manifest=json.loads((Path(t)/(chip+'.json')).read_text())
                self.assertEqual((manifest['boot_bytes'],manifest['boot_size_max_bytes'],manifest['bootsz1_0_bits']), (1024,8192,'11'))
                self.assertEqual(manifest['boot_start_hex'],'0x1FC00')
    def test_source_references_correct_boot_branch(self):
        s=(ROOT/'optiboot_mini.S').read_text()
        self.assertIn('#elif MINI_FLASH_BANKED || (BOOT_BYTES > 4096)',s)

if __name__=='__main__':unittest.main()
