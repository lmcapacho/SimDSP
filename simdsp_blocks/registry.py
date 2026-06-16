from __future__ import annotations

from simdsp_blocks.awgn import AWGN
from simdsp_blocks.fft import FFTMag
from simdsp_blocks.python_module import PythonModuleBlock
from simdsp_blocks.scope import ScopeTap
from simdsp_blocks.sine import Sine
from simdsp_core.registry import BlockRegistry


BUILTIN_BLOCKS = (Sine, AWGN, FFTMag, ScopeTap, PythonModuleBlock)


def register_builtin_blocks(registry: BlockRegistry | None = None) -> BlockRegistry:
    registry = registry or BlockRegistry()
    for block_cls in BUILTIN_BLOCKS:
        if not registry.is_registered(block_cls.SPEC.type_name):
            registry.register_class(block_cls)
    return registry
