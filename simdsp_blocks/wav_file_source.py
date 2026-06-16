from __future__ import annotations

from pathlib import Path
import wave

import numpy as np

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class WavFileSource(Block):
    SPEC = BlockSpec(
        type_name="WavFileSource",
        implementation="python",
        inputs=0,
        outputs=1,
        description="Read PCM WAV audio from a file as a streaming source.",
        params=(
            ParamSpec("path", "str", "", "Path to a WAV file."),
            ParamSpec("loop", "bool", False, "Loop the file when the end is reached."),
            ParamSpec("normalize", "bool", True, "Normalize integer PCM to [-1, 1]."),
        ),
    )

    def __init__(
        self,
        path: str,
        loop: bool = False,
        normalize: bool = True,
        context: dict | None = None,
    ) -> None:
        super().__init__(path=path, loop=loop, normalize=normalize)
        self.path = path
        self.loop = bool(loop)
        self.normalize = bool(normalize)
        self.context = dict(context or {})
        self._data: np.ndarray | None = None
        self._cursor = 0

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)
        self._data = self._load_wave_file(self.path)
        self._cursor = 0

    def process(self, inputs):
        if self._data is None:
            raise RuntimeError("WavFileSource has not been initialized.")

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

    def _load_wave_file(self, raw_path: str) -> np.ndarray:
        path = self._resolve_path(raw_path)
        if not path.exists():
            raise ValueError(f"WavFileSource input file does not exist: {path}")

        with wave.open(str(path), "rb") as wav_file:
            src_channels = wav_file.getnchannels()
            sampwidth = wav_file.getsampwidth()
            frames = wav_file.getnframes()
            raw = wav_file.readframes(frames)

        if sampwidth == 1:
            pcm = np.frombuffer(raw, dtype=np.uint8).astype(np.float32)
            pcm = (pcm - 128.0) / 128.0 if self.normalize else pcm
        elif sampwidth == 2:
            pcm = np.frombuffer(raw, dtype=np.int16).astype(np.float32)
            pcm = pcm / 32768.0 if self.normalize else pcm
        elif sampwidth == 4:
            pcm = np.frombuffer(raw, dtype=np.int32).astype(np.float32)
            pcm = pcm / 2147483648.0 if self.normalize else pcm
        else:
            raise ValueError(f"Unsupported WAV sample width: {sampwidth} bytes")

        return pcm.reshape(-1, src_channels).astype(np.float32, copy=False)

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
