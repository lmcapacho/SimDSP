from __future__ import annotations

from pathlib import Path

import numpy as np
from scipy.io import loadmat, savemat

from app_desktop.pipeline_tools import DEFAULT_BLOCK_REGISTRY, engine_from_pipeline_path
from simdsp_blocks.mat_file_sink import MatFileSink
from simdsp_blocks.mat_file_source import MatFileSource
from simdsp_core.engine import Engine
from simdsp_blocks.sine import Sine


def test_mat_file_source_reads_signal_from_pipeline_context(tmp_path):
    mat_path = tmp_path / 'signal.mat'
    signal = np.linspace(-0.75, 0.75, 512, dtype=np.float32)[:, None]
    savemat(mat_path, {'signal': signal})

    pipeline_path = tmp_path / 'pipeline.json'
    pipeline_path.write_text(
        '{\n'
        '  "schema_version": 1,\n'
        '  "sample_rate": 48000,\n'
        '  "block_size": 128,\n'
        '  "channels": 1,\n'
        '  "nodes": [\n'
        '    {"id": "src", "type": "MatFileSource", "params": {"path": "signal.mat", "variable": "signal", "loop": false}},\n'
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
    fourth = engine.run_once()['src'][0]
    fifth = engine.run_once()['src'][0]

    assert np.max(np.abs(first)) > 0.0
    assert np.max(np.abs(second)) > 0.0
    assert np.max(np.abs(third)) > 0.0
    assert np.max(np.abs(fourth)) > 0.0
    assert np.allclose(fifth, 0.0)


def test_mat_file_sink_writes_captured_signal_on_teardown(tmp_path):
    out_path = tmp_path / 'captured.mat'
    sink = MatFileSink(path='captured.mat', variable='captured', context={'pipeline_dir': str(tmp_path)})

    eng = Engine(sample_rate=48_000, block_size=64, channels=1)
    eng.add_node('gen', Sine(freq=500, amp=0.5), inputs=[])
    eng.add_node('sink', sink, inputs=['gen'])
    eng.init()

    eng.run_once()
    eng.run_once()
    eng.stop()

    saved = loadmat(out_path)
    assert 'captured' in saved
    assert saved['captured'].shape == (128, 1)
    assert float(saved['sample_rate'][0, 0]) == 48_000.0


def test_registry_can_create_mat_blocks_with_context(tmp_path):
    mat_path = tmp_path / 'signal.mat'
    savemat(mat_path, {'signal': np.zeros((32, 1), dtype=np.float32)})

    source = DEFAULT_BLOCK_REGISTRY.create('MatFileSource', {'path': 'signal.mat'}, {'pipeline_dir': str(tmp_path)})
    sink = DEFAULT_BLOCK_REGISTRY.create('MatFileSink', {'path': 'out.mat'}, {'pipeline_dir': str(tmp_path)})

    assert isinstance(source, MatFileSource)
    assert isinstance(sink, MatFileSink)
