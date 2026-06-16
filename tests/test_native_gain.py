from __future__ import annotations

import numpy as np

from app_desktop.pipeline_tools import DEFAULT_BLOCK_REGISTRY
from simdsp_blocks.sine import Sine
from simdsp_core.engine import Engine
from simdsp_native.build import build_native_gain, native_gain_library_path


def test_native_gain_library_builds():
    path = build_native_gain(force=True)
    assert path == native_gain_library_path()
    assert path.exists()


def test_native_gain_runs_inside_engine():
    eng = Engine(sample_rate=48_000, block_size=128, channels=1)
    eng.add_node('gen', Sine(freq=1000, amp=0.8), inputs=[])
    eng.add_node('gain', DEFAULT_BLOCK_REGISTRY.create('NativeGain', {'gain': 0.5}), inputs=['gen'])
    eng.init()

    buffers = eng.run_once()
    x = buffers['gen'][0]
    y = buffers['gain'][0]

    assert np.max(np.abs(y)) > 0.0
    assert np.allclose(y, np.clip(x * 0.5, -1.0, 1.0), atol=1e-6)

    eng.stop()
