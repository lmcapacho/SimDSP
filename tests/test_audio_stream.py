from __future__ import annotations

import numpy as np

from simdsp_blocks.sine import Sine
from simdsp_core.engine import Engine
from simdsp_io.audio_stream import AudioEngineStream


class _FakeStream:
    def __init__(self, callback, channels, blocksize, dtype, **kwargs):
        self.callback = callback
        self.channels = channels
        self.blocksize = blocksize
        self.dtype = dtype
        self.started = False
        self.closed = False

    def start(self):
        self.started = True

    def stop(self):
        self.started = False

    def close(self):
        self.closed = True

    def trigger(self, status=False):
        indata = np.zeros((self.blocksize, self.channels), dtype=np.float32)
        outdata = np.zeros((self.blocksize, self.channels), dtype=np.float32)
        self.callback(indata, outdata, self.blocksize, {}, status)
        return outdata


class _FakeSD:
    def __init__(self):
        self.last_stream = None

    def Stream(self, **kwargs):
        stream = _FakeStream(**kwargs)
        self.last_stream = stream
        return stream


def test_audio_engine_stream_start_and_stop():
    fake_sd = _FakeSD()
    eng = Engine(sample_rate=48_000, block_size=512, channels=1)
    eng.add_node("gen", Sine(freq=1000, amp=0.8), inputs=[])

    stream = AudioEngineStream(eng, output_node="gen", sd_backend=fake_sd)
    stream.start()

    assert fake_sd.last_stream is not None
    assert fake_sd.last_stream.started is True

    stream.stop()
    assert fake_sd.last_stream.closed is True


def test_audio_callback_emits_engine_output_and_tracks_stats():
    fake_sd = _FakeSD()
    eng = Engine(sample_rate=48_000, block_size=256, channels=1)
    eng.add_node("gen", Sine(freq=500, amp=1.0), inputs=[])

    stream = AudioEngineStream(eng, output_node="gen", sd_backend=fake_sd)
    stream.start()

    out = fake_sd.last_stream.trigger(status=False)
    stats = stream.stats()
    assert stats.callbacks == 1
    assert stats.xruns == 0
    assert np.max(np.abs(out)) > 0.0
    assert np.max(np.abs(out)) <= 1.0

    fake_sd.last_stream.trigger(status=True)
    stats = stream.stats()
    assert stats.callbacks == 2
    assert stats.xruns == 1

    stream.stop()
