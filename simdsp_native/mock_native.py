from __future__ import annotations

import numpy as np

from simdsp_core.block_api import BlockSpec, ParamSpec


GAIN_NATIVE_SPEC = BlockSpec(
    type_name="NativeGain",
    implementation="native",
    inputs=1,
    outputs=1,
    description="Example native-style gain block behind an adapter contract.",
    params=(ParamSpec("gain", "float", 1.0, "Linear gain factor."),),
)


class GainBackend:
    def __init__(self, params: dict):
        self.gain = float(params.get("gain", 1.0))

    def init(self, sample_rate: float, block_size: int, channels: int) -> None:
        self.sr = float(sample_rate)
        self.bs = int(block_size)
        self.ch = int(channels)

    def process(self, inputs: list[np.ndarray]) -> list[np.ndarray]:
        x = inputs[0].astype(np.float32, copy=False)
        return [np.clip(x * self.gain, -1.0, 1.0)]

    def teardown(self) -> None:
        return
