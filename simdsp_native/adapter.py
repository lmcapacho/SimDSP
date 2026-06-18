from __future__ import annotations

from typing import Callable

import numpy as np

from simdsp_core.block_api import Block, BlockSpec, NativeBlockBackend, NativeBlockCapabilities


class NativeBlockAdapter(Block):
    def __init__(self, backend_factory: Callable[[dict], NativeBlockBackend], spec: BlockSpec, **params):
        super().__init__(**params)
        self._backend_factory = backend_factory
        self._backend: NativeBlockBackend | None = None
        self._spec = spec
        self._capabilities = getattr(backend_factory, 'CAPABILITIES', None)
        for name, value in params.items():
            setattr(self, name, value)

    @property
    def spec(self) -> BlockSpec:
        return self._spec

    @property
    def capabilities(self) -> NativeBlockCapabilities | None:
        return self._capabilities

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)
        self._backend = self._backend_factory(dict(self.params))
        self._capabilities = getattr(self._backend, 'CAPABILITIES', self._capabilities)
        try:
            self._backend.init(self.sr, self.bs, self.ch)
        except Exception as exc:
            self._backend = None
            raise RuntimeError(
                f"Failed to initialize native block '{self._spec.type_name}': {exc}"
            ) from exc

    def process(self, inputs):
        if self._backend is None:
            raise RuntimeError("Native backend has not been initialized.")
        try:
            outputs = self._backend.process(inputs)
        except Exception as exc:
            raise RuntimeError(
                f"Native block '{self._spec.type_name}' failed during processing: {exc}"
            ) from exc
        return self._validate_outputs(outputs)

    def teardown(self):
        if self._backend is not None:
            self._backend.teardown()
            self._backend = None

    def _validate_outputs(self, outputs):
        if not isinstance(outputs, list):
            raise RuntimeError(
                f"Native block '{self._spec.type_name}' must return a list of numpy arrays."
            )
        if len(outputs) != self._spec.outputs:
            raise RuntimeError(
                f"Native block '{self._spec.type_name}' returned {len(outputs)} output(s) but spec declares {self._spec.outputs}."
            )
        validated = []
        for idx, out in enumerate(outputs):
            arr = np.asarray(out, dtype=np.float32)
            if arr.ndim != 2:
                raise RuntimeError(
                    f"Native block '{self._spec.type_name}' output {idx} must be a 2D array shaped (frames, channels)."
                )
            if arr.shape != (self.bs, self.ch):
                raise RuntimeError(
                    f"Native block '{self._spec.type_name}' output {idx} has shape {arr.shape}, expected {(self.bs, self.ch)}."
                )
            validated.append(arr)
        return validated
