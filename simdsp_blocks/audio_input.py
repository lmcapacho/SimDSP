from __future__ import annotations

from queue import Empty, Queue

import numpy as np

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class AudioInput(Block):
    SPEC = BlockSpec(
        type_name="AudioInput",
        implementation="python",
        inputs=0,
        outputs=1,
        category="source",
        display_name="Audio Input",
        description="Capture audio from the system input device as a streaming source.",
        tags=("audio", "microphone", "live", "legacy"),
        params=(
            ParamSpec("device", "str|int|null", None, "Input device identifier."),
            ParamSpec("latency", "str|float", "low", "Requested input latency."),
            ParamSpec("dtype", "str", "float32", "Host stream dtype."),
        ),
    )

    def __init__(
        self,
        device: str | int | None = None,
        latency: str | float = "low",
        dtype: str = "float32",
        context: dict | None = None,
    ) -> None:
        super().__init__(device=device, latency=latency, dtype=dtype)
        self.device = device
        self.latency = latency
        self.dtype = dtype
        self.context = dict(context or {})
        self._sd = self.context.get("sounddevice_backend") or self._import_sounddevice()
        self._queue: Queue[np.ndarray] = Queue(maxsize=8)
        self._stream = None

    def _import_sounddevice(self):
        try:
            import sounddevice as sd
        except ImportError as exc:
            raise RuntimeError(
                "sounddevice is required for AudioInput. Install project dependencies first."
            ) from exc
        return sd

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)
        self._stream = self._sd.InputStream(
            samplerate=self.sr,
            blocksize=self.bs,
            channels=self.ch,
            dtype=self.dtype,
            device=self.device,
            latency=self.latency,
            callback=self._audio_callback,
        )
        self._stream.start()

    def process(self, inputs):
        try:
            return [self._queue.get_nowait()]
        except Empty:
            return [np.zeros((self.bs, self.ch), dtype=np.float32)]

    def teardown(self):
        if self._stream is not None:
            self._stream.stop()
            self._stream.close()
            self._stream = None
        while True:
            try:
                self._queue.get_nowait()
            except Empty:
                break

    def _audio_callback(self, indata, frames, time_info, status):
        block = np.asarray(indata, dtype=np.float32)
        out = np.zeros((self.bs, self.ch), dtype=np.float32)
        n = min(frames, block.shape[0], self.bs)
        c = min(block.shape[1], self.ch)
        out[:n, :c] = block[:n, :c]
        if self._queue.full():
            try:
                self._queue.get_nowait()
            except Empty:
                pass
        self._queue.put_nowait(out)
