from __future__ import annotations

import numpy as np

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class FFTMag(Block):
    SPEC = BlockSpec(
        type_name="FFTMag",
        implementation="python",
        inputs=1,
        outputs=1,
        category="analysis",
        display_name="FFT Magnitude",
        description="FFT magnitude in dB.",
        tags=("spectrum", "visualization", "analysis"),
        params=(ParamSpec("window", "str", "hann", "Window function name.", choices=("hann", "rect")),),
    )

    def __init__(self, window="hann"):
        super().__init__(window=window)
        self.window = window

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)
        self.w = (
            np.hanning(self.bs).astype(np.float32)
            if self.window == "hann"
            else np.ones(self.bs, np.float32)
        )

    def process(self, inputs):
        x = inputs[0] * self.w[:, None]
        X = np.fft.rfft(x, axis=0)
        mag = 20 * np.log10(np.maximum(1e-12, np.abs(X))).astype(np.float32)
        return [mag]
