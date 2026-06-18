from __future__ import annotations

import numpy as np

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class Gain(Block):
    SPEC = BlockSpec(
        type_name="Gain",
        implementation="python",
        inputs=1,
        outputs=1,
        category="effect",
        display_name="Gain",
        description="Linear gain stage.",
        tags=("gain", "amplitude", "legacy"),
        params=(
            ParamSpec("gain", "float", 1.0, "Linear amplitude scale factor.", min_value=0.0, max_value=100.0, step=0.1),
        ),
    )

    def __init__(self, gain: float = 1.0) -> None:
        super().__init__(gain=gain)
        self.gain = float(gain)

    def process(self, inputs):
        x = inputs[0].astype(np.float32, copy=False)
        return [np.clip(x * self.gain, -1.0, 1.0)]
