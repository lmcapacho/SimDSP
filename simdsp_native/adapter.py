from __future__ import annotations

from typing import Callable

from simdsp_core.block_api import Block, BlockSpec, NativeBlockBackend


class NativeBlockAdapter(Block):
    def __init__(self, backend_factory: Callable[[dict], NativeBlockBackend], spec: BlockSpec, **params):
        super().__init__(**params)
        self._backend_factory = backend_factory
        self._backend: NativeBlockBackend | None = None
        self._spec = spec
        for name, value in params.items():
            setattr(self, name, value)

    @property
    def spec(self) -> BlockSpec:
        return self._spec

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)
        self._backend = self._backend_factory(dict(self.params))
        self._backend.init(self.sr, self.bs, self.ch)

    def process(self, inputs):
        if self._backend is None:
            raise RuntimeError("Native backend has not been initialized.")
        return self._backend.process(inputs)

    def teardown(self):
        if self._backend is not None:
            self._backend.teardown()
            self._backend = None
