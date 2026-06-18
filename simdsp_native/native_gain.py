from __future__ import annotations

import ctypes
import numpy as np

from simdsp_core.block_api import BlockSpec, NativeBlockCapabilities, ParamSpec
from simdsp_native.build import build_native_gain, native_gain_library_path


NATIVE_GAIN_SPEC = BlockSpec(
    type_name="NativeGain",
    implementation="native-cpp",
    inputs=1,
    outputs=1,
    category="native",
    display_name="Native Gain",
    description="Native C++ gain block loaded through a shared library.",
    tags=("c++", "native", "gain"),
    params=(ParamSpec("gain", "float", 1.0, "Linear gain factor."),),
)

NATIVE_GAIN_CAPABILITIES = NativeBlockCapabilities(
    backend_name="simdsp_native_gain",
    language="c++17",
    supported_platforms=("linux", "darwin", "win32"),
    requires_compiler=True,
    auto_build=True,
    shared_library_name=native_gain_library_path().name,
    notes="Auto-build currently works on Linux and macOS. Windows requires a prebuilt DLL.",
)


class NativeGainBackend:
    CAPABILITIES = NATIVE_GAIN_CAPABILITIES

    def __init__(self, params: dict):
        self.gain = float(params.get("gain", 1.0))
        self._lib = None
        self._process = None

    def init(self, sample_rate: float, block_size: int, channels: int) -> None:
        self.sr = float(sample_rate)
        self.bs = int(block_size)
        self.ch = int(channels)
        lib_path = build_native_gain()
        try:
            self._lib = ctypes.CDLL(str(lib_path))
        except OSError as exc:
            raise RuntimeError(f"Could not load native library '{lib_path}': {exc}") from exc
        try:
            self._process = self._lib.simdsp_native_gain_process
        except AttributeError as exc:
            raise RuntimeError(
                f"Native library '{lib_path}' does not export 'simdsp_native_gain_process'."
            ) from exc
        self._process.argtypes = [
            ctypes.POINTER(ctypes.c_float),
            ctypes.POINTER(ctypes.c_float),
            ctypes.c_size_t,
            ctypes.c_size_t,
            ctypes.c_float,
        ]
        self._process.restype = ctypes.c_int

    def process(self, inputs: list[np.ndarray]) -> list[np.ndarray]:
        if self._process is None:
            raise RuntimeError("Native gain backend has not been initialized.")
        x = np.ascontiguousarray(inputs[0], dtype=np.float32)
        y = np.empty_like(x)
        status = self._process(
            x.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
            y.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
            x.shape[0],
            x.shape[1],
            ctypes.c_float(self.gain),
        )
        if status != 0:
            raise RuntimeError(f"Native gain backend returned error code {status}.")
        return [y]

    def teardown(self) -> None:
        self._lib = None
        self._process = None
