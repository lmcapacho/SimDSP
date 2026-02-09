import numpy as np

class FFTMag:
    def __init__(self, window="hann"):
        self.window = window

    def init(self, sample_rate, block_size, channels):
        self.sr = float(sample_rate); self.bs = int(block_size); self.ch = int(channels)
        self.w = np.hanning(self.bs).astype(np.float32) if self.window == "hann" else np.ones(self.bs, np.float32)

    def process(self, inputs):
        x = inputs[0] * self.w[:, None]
        X = np.fft.rfft(x, axis=0)
        mag = 20*np.log10(np.maximum(1e-12, np.abs(X))).astype(np.float32)
        return [mag]

    def teardown(self): pass
