from __future__ import annotations

import numpy as np
import pytest

from simdsp_blocks.awgn import AWGN
from simdsp_blocks.fft import FFTMag
from simdsp_blocks.scope import ScopeTap
from simdsp_blocks.sine import Sine
from simdsp_io.pipeline_json import (
    PIPELINE_SCHEMA_VERSION,
    load_pipeline,
    pipeline_to_engine,
    save_pipeline,
)


def _factory(block_type: str, params: dict):
    registry = {
        "Sine": Sine,
        "AWGN": AWGN,
        "FFTMag": FFTMag,
        "ScopeTap": ScopeTap,
    }
    return registry[block_type](**params)


def test_pipeline_json_roundtrip(tmp_path):
    pipeline = {
        "schema_version": PIPELINE_SCHEMA_VERSION,
        "sample_rate": 48_000,
        "block_size": 2048,
        "channels": 1,
        "nodes": [
            {"id": "gen", "type": "Sine", "params": {"freq": 1200, "amp": 0.7}},
            {"id": "awgn", "type": "AWGN", "params": {"snr_db": 20, "seed": 42}},
            {"id": "fft", "type": "FFTMag", "params": {}},
        ],
        "edges": [
            {"from": "gen", "to": "awgn"},
            {"from": "awgn", "to": "fft"},
        ],
    }

    path = tmp_path / "pipeline.json"
    save_pipeline(path, pipeline)
    loaded = load_pipeline(path)

    assert loaded == pipeline


def test_pipeline_to_engine_runs_chain_sine_awgn_fft():
    pipeline = {
        "schema_version": PIPELINE_SCHEMA_VERSION,
        "sample_rate": 48_000,
        "block_size": 2048,
        "channels": 1,
        "nodes": [
            {"id": "gen", "type": "Sine", "params": {"freq": 1000, "amp": 0.8}},
            {"id": "awgn", "type": "AWGN", "params": {"snr_db": 25, "seed": 7}},
            {"id": "fft", "type": "FFTMag", "params": {}},
        ],
        "edges": [
            {"from": "gen", "to": "awgn"},
            {"from": "awgn", "to": "fft"},
        ],
    }

    engine = pipeline_to_engine(pipeline, _factory)
    buffers = engine.run_once()

    mag = buffers["fft"][0][:, 0]
    freqs = np.fft.rfftfreq(engine.bs, d=1 / engine.sr)
    peak_freq = freqs[np.argmax(mag)]

    assert abs(peak_freq - 1000) <= (engine.sr / engine.bs) * 2


def test_pipeline_to_engine_rejects_cycles():
    pipeline = {
        "schema_version": PIPELINE_SCHEMA_VERSION,
        "sample_rate": 48_000,
        "block_size": 256,
        "channels": 1,
        "nodes": [
            {"id": "a", "type": "ScopeTap", "params": {}},
            {"id": "b", "type": "ScopeTap", "params": {}},
        ],
        "edges": [
            {"from": "a", "to": "b"},
            {"from": "b", "to": "a"},
        ],
    }

    with pytest.raises(ValueError, match="Cycle detected"):
        pipeline_to_engine(pipeline, _factory)
