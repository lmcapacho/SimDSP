from __future__ import annotations

from pathlib import Path

import numpy as np
from scipy.io import loadmat

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class MatFileSource(Block):
    SPEC = BlockSpec(
        type_name="MatFileSource",
        implementation="python",
        inputs=0,
        outputs=1,
        category="source",
        display_name="MAT File Source",
        description="Read a signal matrix from a Matlab/Octave MAT file.",
        tags=("matlab", "octave", "file", "legacy"),
        params=(
            ParamSpec("path", "str", "", "Path to a MAT file."),
            ParamSpec("variable", "str", "signal", "Variable name inside the MAT file."),
            ParamSpec("loop", "bool", False, "Loop the signal when the end is reached."),
            ParamSpec("transpose", "bool", False, "Transpose the loaded matrix before use."),
        ),
    )

    def __init__(
        self,
        path: str,
        variable: str = "signal",
        loop: bool = False,
        transpose: bool = False,
        context: dict | None = None,
    ) -> None:
        super().__init__(path=path, variable=variable, loop=loop, transpose=transpose)
        self.path = path
        self.variable = variable
        self.loop = bool(loop)
        self.transpose = bool(transpose)
        self.context = dict(context or {})
        self._data: np.ndarray | None = None
        self._cursor = 0

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)
        self._data = self._load_mat_signal(self.path, self.variable)
        self._cursor = 0

    def process(self, inputs):
        if self._data is None:
            raise RuntimeError("MatFileSource has not been initialized.")
        if self._data.shape[0] == 0:
            return [np.zeros((self.bs, self.ch), dtype=np.float32)]

        start = self._cursor
        end = start + self.bs
        if end <= self._data.shape[0]:
            chunk = self._data[start:end]
            self._cursor = end
        else:
            chunk = np.zeros((self.bs, self._data.shape[1]), dtype=np.float32)
            available = max(0, self._data.shape[0] - start)
            if available > 0:
                chunk[:available] = self._data[start:]
            self._cursor = self._data.shape[0]
            if self.loop:
                remaining = self.bs - available
                cursor = 0
                while remaining > 0:
                    take = min(remaining, self._data.shape[0])
                    offset = self.bs - remaining
                    chunk[offset : offset + take] = self._data[cursor : cursor + take]
                    remaining -= take
                    cursor = (cursor + take) % self._data.shape[0]
                self._cursor = cursor

        if self._cursor >= self._data.shape[0] and self.loop:
            self._cursor %= self._data.shape[0]

        return [self._adapt_channels(chunk)]

    def _load_mat_signal(self, raw_path: str, variable: str) -> np.ndarray:
        path = self._resolve_path(raw_path)
        if not path.exists():
            raise ValueError(f"MatFileSource input file does not exist: {path}")

        data = loadmat(path)
        if variable not in data:
            raise ValueError(f"Variable '{variable}' was not found in MAT file: {path}")

        signal = np.asarray(data[variable], dtype=np.float32)
        if self.transpose:
            signal = signal.T
        if signal.ndim == 1:
            signal = signal[:, None]
        elif signal.ndim != 2:
            raise ValueError("MatFileSource expects a 1D or 2D signal array.")
        return signal.astype(np.float32, copy=False)

    def _resolve_path(self, raw_path: str) -> Path:
        path = Path(raw_path)
        pipeline_dir = self.context.get("pipeline_dir")
        if not path.is_absolute() and pipeline_dir:
            path = Path(pipeline_dir) / path
        return path.resolve()

    def _adapt_channels(self, chunk: np.ndarray) -> np.ndarray:
        if chunk.shape[1] == self.ch:
            return chunk.astype(np.float32, copy=False)
        if chunk.shape[1] == 1 and self.ch > 1:
            return np.repeat(chunk, self.ch, axis=1).astype(np.float32, copy=False)
        if chunk.shape[1] > self.ch:
            return chunk[:, : self.ch].astype(np.float32, copy=False)
        out = np.zeros((chunk.shape[0], self.ch), dtype=np.float32)
        out[:, : chunk.shape[1]] = chunk
        return out
