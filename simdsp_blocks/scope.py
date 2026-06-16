from __future__ import annotations

from simdsp_core.block_api import Block, BlockSpec


class ScopeTap(Block):
    SPEC = BlockSpec(
        type_name="ScopeTap",
        implementation="python",
        inputs=1,
        outputs=1,
        category="analysis",
        display_name="Scope Tap",
        description="Pass-through tap for time-domain visualization.",
        tags=("scope", "visualization", "tap"),
    )

    def process(self, inputs):
        return inputs
