"""Virtual Boot patcher correctness and refusal tests; host-only, not silicon."""
import sys,unittest,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tiny'))
from patch_vectors import read_hex,make_hex,rjmp,decode_rjmp,patch_application,TARGETS

class TestTinyVectorPatch(unittest.TestCase):
    def test_patch_and_roundtrip(self):
        for name,t in TARGETS.items():
            with self.subTest(name=name):
                original_start=0x100
                original_reset=rjmp(0,original_start//2,t['flash']//2)
                memory={0:original_reset&255,1:original_reset>>8,0x100:0x78,0x101:0x94,
                        t['vector']*2:255,t['vector']*2+1:255, t['flash']-t['boot']-1:0x55}
                patched,meta=patch_application(memory,name)
                self.assertEqual(decode_rjmp(0,patched[0]|patched[1]<<8,t['flash']//2),
                                 (t['flash']-t['boot'])//2)
                i=t['vector']
                self.assertEqual(decode_rjmp(i,patched[i*2]|patched[i*2+1]<<8,t['flash']//2),
                                 original_start//2)
                self.assertEqual(patched[0x100],0x78)
                with tempfile.TemporaryDirectory() as td:
                    path=Path(td)/'app.hex';path.write_text(make_hex(patched))
                    self.assertEqual(read_hex(path),patched)
    def test_reject_app_overlap_boot(self):
        for name,t in TARGETS.items():
            mem={0:0x7f,1:0xc0,t['flash']-t['boot']:0}
            with self.assertRaisesRegex(ValueError,'overlaps'):
                patch_application(mem,name)
    def test_reject_non_rjmp_reset(self):
        with self.assertRaisesRegex(ValueError,'not RJMP'):
            patch_application({0:0x0c,1:0x94},'attiny1634')
    def test_reject_used_vector_by_default(self):
        with self.assertRaisesRegex(ValueError,'occupied'):
            patch_application({0:0x7f,1:0xc0,52:0xff,53:0xcf},'attiny1634')
        self.assertEqual((patch_application({0:0x7f,1:0xc0,52:0xff,53:0xcf},'attiny1634',True)[0][52+1] & 0xf0),0xc0)
    def test_reject_vector_out_of_range(self):
        # A reset RJMP of -2048 points near the end of the app, but
        # relocating it 26 words forward exceeds the -2048 reach.
        with self.assertRaisesRegex(ValueError,'out of reach'):
            patch_application({0:0x00,1:0xc8},'attiny1634')
    def test_reject_corrupt_hex_checksum(self):
        with tempfile.TemporaryDirectory() as td:
            f=Path(td)/'a.hex'; f.write_text(':0200000000C03F\n:00000001FF\n')
            with self.assertRaisesRegex(ValueError,'checksum'):read_hex(f)
if __name__=='__main__': unittest.main()
