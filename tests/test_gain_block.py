from __future__ import annotations

import numpy as np

from simdsp_blocks.gain import Gain


def test_gain_scales_and_clips_signal():
    block = Gain(gain=2.0)
    block.init(48_000, 4, 1)

    x = np.array([[0.2], [0.4], [0.8], [-0.8]], dtype=np.float32)
    y = block.process([x])[0]

    assert np.allclose(y[:, 0], [0.4, 0.8, 1.0, -1.0])
