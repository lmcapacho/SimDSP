from __future__ import annotations

from dataclasses import dataclass
from typing import Any
import numpy as np

from simdsp_core.engine import Engine


@dataclass
class AudioStats:
    callbacks: int = 0
    xruns: int = 0


class AudioEngineStream:
    """Bridge between sounddevice callback I/O and the DSP Engine."""

    def __init__(
        self,
        engine: Engine,
        output_node: str | None = None,
        device: int | str | None = None,
        dtype: str = "float32",
        latency: str | float = "low",
        sd_backend: Any | None = None,
    ) -> None:
        self.engine = engine
        self.output_node = output_node
        self.device = device
        self.dtype = dtype
        self.latency = latency
        self._sd = sd_backend if sd_backend is not None else self._import_sounddevice()
        self._stream = None
        self._stats = AudioStats()
        self._initialized = False

    def _import_sounddevice(self):
        try:
            import sounddevice as sd
        except ImportError as exc:
            raise RuntimeError(
                "sounddevice is required for audio streaming. Install project dependencies first."
            ) from exc
        return sd

    def _resolve_output_node(self) -> str:
        if self.output_node is not None:
            if self.output_node not in self.engine.nodes:
                raise ValueError(f"Unknown output node '{self.output_node}'.")
            return self.output_node

        if not self.engine._order:
            self.engine._order = self.engine._topological_order()
        return self.engine._order[-1]

    def _audio_callback(self, indata, outdata, frames, time_info, status) -> None:
        self._stats.callbacks += 1
        if status:
            self._stats.xruns += 1

        buffers = self.engine.run_once()
        out_node = self._resolve_output_node()
        y = np.asarray(buffers[out_node][0], dtype=np.float32)

        # Graceful adaptation if host callback uses a different frame count.
        outdata.fill(0.0)
        n = min(frames, y.shape[0])
        c = min(outdata.shape[1], y.shape[1])
        outdata[:n, :c] = np.clip(y[:n, :c], -1.0, 1.0)

    def start(self) -> None:
        if self._stream is not None:
            return
        if not self._initialized:
            self.engine.init()
            self._initialized = True

        self._stream = self._sd.Stream(
            samplerate=self.engine.sr,
            blocksize=self.engine.bs,
            channels=self.engine.ch,
            dtype=self.dtype,
            device=self.device,
            latency=self.latency,
            callback=self._audio_callback,
        )
        self._stream.start()

    def stop(self) -> None:
        if self._stream is None:
            return
        self._stream.stop()
        self._stream.close()
        self._stream = None
        for node in self.engine.nodes.values():
            try:
                node.block.teardown()
            except Exception:
                pass

    def stats(self) -> AudioStats:
        return AudioStats(callbacks=self._stats.callbacks, xruns=self._stats.xruns)
