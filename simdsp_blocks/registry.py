from __future__ import annotations

from simdsp_blocks.audio_input import AudioInput
from simdsp_blocks.awgn import AWGN
from simdsp_blocks.fft import FFTMag
from simdsp_blocks.gain import Gain
from simdsp_blocks.mat_file_sink import MatFileSink
from simdsp_blocks.mat_file_source import MatFileSource
from simdsp_blocks.wav_file_source import WavFileSource
from simdsp_blocks.python_module import PythonModuleBlock
from simdsp_blocks.scope import ScopeTap
from simdsp_blocks.square import Square
from simdsp_blocks.triangle import Triangle
from simdsp_blocks.sine import Sine
from simdsp_core.registry import BlockRegistry
from simdsp_native import NATIVE_GAIN_SPEC, NativeGainBackend, NativeBlockAdapter


BUILTIN_BLOCKS = (Sine, Square, Triangle, AWGN, Gain, FFTMag, ScopeTap, WavFileSource, AudioInput, MatFileSource, MatFileSink, PythonModuleBlock)


def register_builtin_blocks(registry: BlockRegistry | None = None) -> BlockRegistry:
    registry = registry or BlockRegistry()
    for block_cls in BUILTIN_BLOCKS:
        if not registry.is_registered(block_cls.SPEC.type_name):
            registry.register_class(block_cls)

    if not registry.is_registered(NATIVE_GAIN_SPEC.type_name):
        registry.register(
            NATIVE_GAIN_SPEC,
            lambda params, context=None: NativeBlockAdapter(NativeGainBackend, NATIVE_GAIN_SPEC, **params),
        )
    return registry
