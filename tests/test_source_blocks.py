from __future__ import annotations

import math
import wave
from pathlib import Path

import numpy as np

from app_desktop.pipeline_tools import DEFAULT_BLOCK_REGISTRY, engine_from_pipeline_path
from simdsp_blocks.audio_input import AudioInput
from simdsp_blocks.wav_file_source import WavFileSource


class _FakeInputStream:
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

    def push(self, data):
        self.callback(data, data.shape[0], {}, False)


class _FakeSDInput:
    def __init__(self):
        self.last_stream = None

    def InputStream(self, **kwargs):
        stream = _FakeInputStream(**kwargs)
        self.last_stream = stream
        return stream


def _write_wave(path: Path, samples: np.ndarray, sample_rate: int = 48000) -> None:
    pcm = np.clip(samples, -1.0, 1.0)
    ints = (pcm * 32767.0).astype(np.int16)
    with wave.open(str(path), 'wb') as wav_file:
        wav_file.setnchannels(samples.shape[1])
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(ints.tobytes())


def test_wav_file_source_reads_wav_blocks_from_pipeline_context(tmp_path):
    wav_path = tmp_path / 'tone.wav'
    t = np.arange(0, 512, dtype=np.float32) / 48000.0
    samples = (0.5 * np.sin(2 * math.pi * 1000.0 * t)).astype(np.float32)[:, None]
    _write_wave(wav_path, samples)

    pipeline_path = tmp_path / 'pipeline.json'
    pipeline_path.write_text(
        '{\n'
        '  "schema_version": 1,\n'
        '  "sample_rate": 48000,\n'
        '  "block_size": 256,\n'
        '  "channels": 1,\n'
        '  "nodes": [\n'
        '    {"id": "src", "type": "WavFileSource", "params": {"path": "tone.wav", "loop": false}},\n'
        '    {"id": "scope", "type": "ScopeTap", "params": {}}\n'
        '  ],\n'
        '  "edges": [\n'
        '    {"from": "src", "to": "scope"}\n'
        '  ]\n'
        '}\n'
    )

    engine = engine_from_pipeline_path(pipeline_path)
    first = engine.run_once()['src'][0]
    second = engine.run_once()['src'][0]
    third = engine.run_once()['src'][0]

    assert np.max(np.abs(first)) > 0.0
    assert np.max(np.abs(second)) > 0.0
    assert np.allclose(third, 0.0)


def test_audio_input_captures_blocks_from_backend_context():
    fake_sd = _FakeSDInput()
    block = AudioInput(context={'sounddevice_backend': fake_sd})
    block.init(48_000, 8, 1)

    pushed = np.linspace(-0.5, 0.5, 8, dtype=np.float32)[:, None]
    fake_sd.last_stream.push(pushed)
    out = block.process([])[0]

    assert np.allclose(out[:8, :1], pushed)
    block.teardown()
    assert fake_sd.last_stream.closed is True


def test_registry_can_create_wav_file_source_with_context(tmp_path):
    wav_path = tmp_path / 'tone.wav'
    samples = np.zeros((32, 1), dtype=np.float32)
    _write_wave(wav_path, samples)

    block = DEFAULT_BLOCK_REGISTRY.create('WavFileSource', {'path': 'tone.wav'}, {'pipeline_dir': str(tmp_path)})
    assert isinstance(block, WavFileSource)
    block.init(48_000, 16, 1)
    out = block.process([])[0]
    assert out.shape == (16, 1)
    block.teardown()
