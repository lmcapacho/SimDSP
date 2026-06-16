from __future__ import annotations

import numpy as np

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class Square(Block):
    SPEC = BlockSpec(
        type_name="Square",
        implementation="python",
        inputs=0,
        outputs=1,
        category="generator",
        display_name="Square",
        description="Square wave generator.",
        tags=("signal", "tone", "legacy"),
        params=(
            ParamSpec("freq", "float", 1000.0, "Tone frequency in Hz.", min_value=0.0, max_value=96_000.0, step=1.0, unit="Hz"),
            ParamSpec("amp", "float", 0.7, "Peak amplitude.", min_value=0.0, max_value=1.0, step=0.01),
            ParamSpec("duty", "float", 0.5, "Duty cycle between 0 and 1.", min_value=0.0, max_value=1.0, step=0.01),
        ),
    )

    def __init__(self, freq=1000.0, amp=0.7, duty=0.5):
        super().__init__(freq=freq, amp=amp, duty=duty)
        self.freq = float(freq)
        self.amp = float(amp)
        self.duty = float(duty)
        self._phase = 0.0

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)
        self.duty = min(1.0, max(0.0, self.duty))

    def process(self, inputs):
        t = np.arange(self.bs, dtype=np.float32) / self.sr
        phase = ((self.freq * t) + self._phase) % 1.0
        sig = np.where(phase < self.duty, self.amp, -self.amp).astype(np.float32)
        self._phase = (self._phase + (self.freq * self.bs / self.sr)) % 1.0
        y = np.repeat(sig[:, None], self.ch, axis=1)
        return [np.clip(y, -1.0, 1.0)]
