import numpy as np

class Sine:
    def __init__(self, freq=1000.0, amp=0.7):
        self.freq = float(freq)
        self.amp = float(amp)
        self._phase = 0.0

    def init(self, sample_rate, block_size, channels):
        self.sr = float(sample_rate); self.bs = int(block_size); self.ch = int(channels)

    def process(self, inputs):
        t = (np.arange(self.bs, dtype=np.float32) / self.sr)
        sig = self.amp * np.sin(2*np.pi*self.freq*t + self._phase)
        self._phase = (self._phase + 2*np.pi*self.freq*self.bs/self.sr) % (2*np.pi)
        y = np.repeat(sig[:, None], self.ch, axis=1).astype(np.float32)
        y = np.clip(y, -1.0, 1.0)
        return [y]

    def teardown(self): pass
