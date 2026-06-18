from __future__ import annotations

import numpy as np

from simdsp_blocks.catalog import DEFAULT_BLOCK_REGISTRY
from simdsp_core import BlockRegistry
from simdsp_native import NATIVE_GAIN_SPEC, NativeGainBackend, NativeBlockAdapter


def test_default_registry_contains_builtin_blocks():
    specs = {spec.type_name: spec for spec in DEFAULT_BLOCK_REGISTRY.list_specs()}

    assert {"Sine", "Square", "Triangle", "AWGN", "Gain", "FFTMag", "ScopeTap", "NativeGain"}.issubset(specs.keys())
    assert specs["Sine"].implementation == "python"
    assert specs["FFTMag"].inputs == 1


def test_registry_can_register_native_adapter_block():
    registry = BlockRegistry()
    registry.register(
        NATIVE_GAIN_SPEC,
        lambda params: NativeBlockAdapter(NativeGainBackend, NATIVE_GAIN_SPEC, **params),
    )

    block = registry.create("NativeGain", {"gain": 0.5})
    block.init(48_000, 8, 1)
    x = np.ones((8, 1), dtype=np.float32)
    y = block.process([x])[0]

    assert np.allclose(y, 0.5)
