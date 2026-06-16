from __future__ import annotations

import numpy as np

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class Sine(Block):
    SPEC = BlockSpec(
        type_name="Sine",
        implementation="python",
        inputs=0,
        outputs=1,
        description="Sine wave generator.",
        params=(
            ParamSpec("freq", "float", 1000.0, "Tone frequency in Hz."),
            ParamSpec("amp", "float", 0.7, "Peak amplitude."),
        ),
    )

    def __init__(self, freq=1000.0, amp=0.7):
        super().__init__(freq=freq, amp=amp)
        self.freq = float(freq)
        self.amp = float(amp)
        self._phase = 0.0

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)

    def process(self, inputs):
        t = np.arange(self.bs, dtype=np.float32) / self.sr
        sig = self.amp * np.sin((2 * np.pi * self.freq * t) + self._phase)
        self._phase = (self._phase + ((2 * np.pi * self.freq * self.bs) / self.sr)) % (2 * np.pi)
        y = np.repeat(sig[:, None], self.ch, axis=1).astype(np.float32)
        y = np.clip(y, -1.0, 1.0)
        return [y]
