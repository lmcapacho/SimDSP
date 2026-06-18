from __future__ import annotations

import numpy as np

from simdsp_core import NativeBlockCapabilities


class PluginNativeGainBackend:
    CAPABILITIES = NativeBlockCapabilities(
        backend_name="example_plugin_native_gain",
        language="c++-style-backend",
        supported_platforms=("linux", "darwin", "win32"),
        requires_compiler=False,
        auto_build=False,
        notes="Example backend that follows the native plugin contract without loading a shared library.",
    )

    def __init__(self, params: dict):
        self.gain = float(params.get("gain", 1.0))

    def init(self, sample_rate: float, block_size: int, channels: int) -> None:
        self.bs = int(block_size)
        self.ch = int(channels)

    def process(self, inputs: list[np.ndarray]) -> list[np.ndarray]:
        x = inputs[0].astype(np.float32, copy=False)
        return [np.clip(x * self.gain, -1.0, 1.0)]

    def teardown(self) -> None:
        return
