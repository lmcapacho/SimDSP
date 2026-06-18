from __future__ import annotations

from simdsp_blocks.catalog import DEFAULT_BLOCK_REGISTRY
from simdsp_core import NativeBlockCapabilities
from simdsp_native import NATIVE_GAIN_CAPABILITIES
from simdsp_native.adapter import NativeBlockAdapter
from simdsp_native.mock_native import GAIN_NATIVE_CAPABILITIES, GAIN_NATIVE_SPEC, GainBackend


def test_native_gain_capabilities_are_exported():
    assert isinstance(NATIVE_GAIN_CAPABILITIES, NativeBlockCapabilities)
    assert NATIVE_GAIN_CAPABILITIES.backend_name == 'simdsp_native_gain'
    assert NATIVE_GAIN_CAPABILITIES.language == 'c++17'
    assert NATIVE_GAIN_CAPABILITIES.requires_compiler is True
    assert 'linux' in NATIVE_GAIN_CAPABILITIES.supported_platforms


def test_native_adapter_exposes_backend_capabilities():
    block = NativeBlockAdapter(GainBackend, GAIN_NATIVE_SPEC, gain=0.5)

    assert block.capabilities == GAIN_NATIVE_CAPABILITIES
    block.init(48_000, 16, 1)
    assert block.capabilities == GAIN_NATIVE_CAPABILITIES
    block.teardown()


def test_registry_native_gain_instance_exposes_capabilities():
    block = DEFAULT_BLOCK_REGISTRY.create('NativeGain', {'gain': 0.5})

    assert isinstance(block, NativeBlockAdapter)
    assert block.capabilities is not None
    assert block.capabilities.backend_name == 'simdsp_native_gain'
