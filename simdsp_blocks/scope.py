from __future__ import annotations

from simdsp_core.block_api import Block, BlockSpec


class ScopeTap(Block):
    SPEC = BlockSpec(
        type_name="ScopeTap",
        implementation="python",
        inputs=1,
        outputs=1,
        description="Pass-through tap for time-domain visualization.",
    )

    def process(self, inputs):
        return inputs
