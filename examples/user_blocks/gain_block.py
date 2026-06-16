from __future__ import annotations

import numpy as np

from simdsp_core import Block


class UserBlock(Block):
    def __init__(self, gain: float = 0.5):
        super().__init__(gain=gain)
        self.gain = float(gain)

    def process(self, inputs: list[np.ndarray]) -> list[np.ndarray]:
        x = inputs[0].astype(np.float32, copy=False)
        return [np.clip(x * self.gain, -1.0, 1.0)]
