from __future__ import annotations

import numpy as np

from simdsp_core import Block, BlockSpec, ParamSpec


class PluginGain(Block):
    SPEC = BlockSpec(
        type_name="PluginGain",
        implementation="python-plugin",
        inputs=1,
        outputs=1,
        category="effect",
        display_name="Plugin Gain",
        description="Example gain block shipped as a SimDSP plugin.",
        tags=("plugin", "gain"),
        params=(ParamSpec("gain", "float", 1.0, "Linear gain factor."),),
    )

    def __init__(self, gain: float = 1.0) -> None:
        super().__init__(gain=gain)
        self.gain = float(gain)

    def process(self, inputs):
        x = inputs[0].astype(np.float32, copy=False)
        return [np.clip(x * self.gain, -1.0, 1.0)]
