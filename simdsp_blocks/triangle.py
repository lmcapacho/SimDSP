from __future__ import annotations

import numpy as np

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class Triangle(Block):
    SPEC = BlockSpec(
        type_name="Triangle",
        implementation="python",
        inputs=0,
        outputs=1,
        category="generator",
        display_name="Triangle",
        description="Triangle wave generator.",
        tags=("signal", "tone", "legacy"),
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
        phase = ((self.freq * t) + self._phase) % 1.0
        sig = (4.0 * np.abs(phase - 0.5) - 1.0) * self.amp
        self._phase = (self._phase + (self.freq * self.bs / self.sr)) % 1.0
        y = np.repeat(sig[:, None].astype(np.float32), self.ch, axis=1)
        return [np.clip(y, -1.0, 1.0)]
