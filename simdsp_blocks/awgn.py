import numpy as np


class AWGN:
    def __init__(self, snr_db=40.0, seed=None):
        self.snr_db = float(snr_db)
        self.seed = seed

    def init(self, sample_rate, block_size, channels):
        self.sr = float(sample_rate)
        self.bs = int(block_size)
        self.ch = int(channels)
        self._rng = np.random.default_rng(self.seed)

    def process(self, inputs):
        x = inputs[0].astype(np.float32, copy=False)
        signal_power = float(np.mean(x * x))
        if signal_power <= 0.0:
            return [x.copy()]

        noise_power = signal_power / (10.0 ** (self.snr_db / 10.0))
        noise = self._rng.normal(0.0, np.sqrt(noise_power), size=x.shape).astype(np.float32)
        y = np.clip(x + noise, -1.0, 1.0)
        return [y]

    def teardown(self):
        pass
