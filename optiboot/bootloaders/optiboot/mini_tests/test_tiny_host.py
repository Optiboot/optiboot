"""Mock protocol-only checks of the tiny custom programmer. No serial hardware."""
import sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tiny'))
from flash_tiny import Stk500Mini,upload
from patch_vectors import rjmp,TARGETS,decode_rjmp
from make_seed import make_seed

class Loopback:
    def __init__(self):self.port=self;self.mem=bytearray(b'\xff'*16384);self.cmds=[];self._buffer=b'';self.addr=0
    def write(self,b):
        self.cmds.append(b)
        a=self.addr
        if b==b'0 ':body=b''
        elif b==b'u ':body=b'\x1e\x94\x12'
        elif b[0]==0x55:
            self.addr=(b[1]|b[2]<<8)*2;body=b''
        elif b[0]==0x64:
            n=(b[1]<<8)|b[2];assert b[-1]==0x20 and len(b)==n+5
            self.mem[a:a+n]=b[4:4+n];body=b''
        elif b[0]==0x74:
            n=(b[1]<<8)|b[2];body=bytes(self.mem[a:a+n])
        else:raise AssertionError('Unexpected '+b.hex())
        self._buffer+=b'\x14'+body+b'\x10'
    def flush(self):pass
    def read(self,n):s,self._buffer=self._buffer[:n],self._buffer[n:];return s

class HostTests(unittest.TestCase):
    def test_grouped_128_byte_transfer(self):
        port=Loopback();link=Stk500Mini(port)
        cfg=TARGETS['attiny1634'];total=cfg['flash']//2;boot=(cfg['flash']-cfg['boot'])//2
        j=rjmp(0,boot,total)
        app={0:j&255,1:j>>8,2*cfg['vector']:0,2*cfg['vector']+1:0,0x83:0x21}
        self.assertEqual(upload(link,app,'attiny1634'),(2,128))
        self.assertEqual(port.mem[0x83],0x21)
        self.assertEqual(len([x for x in port.cmds if x[0]==0x64]),2)
        self.assertTrue(all(x[1:3]==bytes((0,128)) for x in port.cmds if x[0]==0x64))
    def test_isp_seed_has_boot_and_safe_initial_vectors(self):
        for name,t in TARGETS.items():
            with self.subTest(name=name):
                bin=bytes([0xff]*t['boot'])
                img=make_seed(bin,name)
                self.assertEqual(len(img),t['boot']+4)
                self.assertEqual(decode_rjmp(0,img[0]|img[1]<<8,t['flash']//2),
                                 (t['flash']-t['boot'])//2)
                i=t['vector']
                self.assertEqual(decode_rjmp(i,img[2*i]|img[2*i+1]<<8,t['flash']//2),
                                 (t['flash']-t['boot'])//2)
    def test_reject_unpatched_reset(self):
        with self.assertRaisesRegex(ValueError,'not been patched'):
            upload(Stk500Mini(Loopback()),{0:0xff,1:0xcf},'attiny841')
    def test_bad_reply_rejected(self):
        class Bad:
            def write(self,b):pass
            def flush(self):pass
            def read(self,n):return b''
        with self.assertRaisesRegex(RuntimeError,'INSYNC'):Stk500Mini(Bad()).sync()
if __name__=='__main__':unittest.main()
