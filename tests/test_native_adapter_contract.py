from __future__ import annotations

import numpy as np
import pytest

from simdsp_core.block_api import BlockSpec
from simdsp_native.adapter import NativeBlockAdapter


class _BackendWrongCount:
    def __init__(self, params):
        self.params = params

    def init(self, sample_rate, block_size, channels):
        self.bs = int(block_size)
        self.ch = int(channels)

    def process(self, inputs):
        return [np.zeros((self.bs, self.ch), dtype=np.float32), np.zeros((self.bs, self.ch), dtype=np.float32)]

    def teardown(self):
        return


class _BackendWrongShape:
    def __init__(self, params):
        self.params = params

    def init(self, sample_rate, block_size, channels):
        self.bs = int(block_size)
        self.ch = int(channels)

    def process(self, inputs):
        return [np.zeros((self.bs,), dtype=np.float32)]

    def teardown(self):
        return


class _BackendInitFails:
    def __init__(self, params):
        self.params = params

    def init(self, sample_rate, block_size, channels):
        raise RuntimeError('compiler missing')

    def process(self, inputs):
        return []

    def teardown(self):
        return


SPEC = BlockSpec(type_name='TestNative', implementation='native-test', inputs=1, outputs=1)


def test_native_adapter_wraps_backend_init_errors():
    block = NativeBlockAdapter(_BackendInitFails, SPEC)

    with pytest.raises(RuntimeError, match="Failed to initialize native block 'TestNative': compiler missing"):
        block.init(48_000, 8, 1)


def test_native_adapter_rejects_wrong_output_count():
    block = NativeBlockAdapter(_BackendWrongCount, SPEC)
    block.init(48_000, 8, 1)
    x = np.zeros((8, 1), dtype=np.float32)

    with pytest.raises(RuntimeError, match='returned 2 output'):
        block.process([x])


def test_native_adapter_rejects_wrong_output_shape():
    block = NativeBlockAdapter(_BackendWrongShape, SPEC)
    block.init(48_000, 8, 1)
    x = np.zeros((8, 1), dtype=np.float32)

    with pytest.raises(RuntimeError, match='must be a 2D array'):
        block.process([x])
