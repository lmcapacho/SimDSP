from __future__ import annotations

from app_desktop.smoke_audio import main, run_smoke
from simdsp_io import PIPELINE_SCHEMA_VERSION, save_pipeline


def _make_pipeline(path):
    pipeline = {
        "schema_version": PIPELINE_SCHEMA_VERSION,
        "sample_rate": 48_000,
        "block_size": 512,
        "channels": 1,
        "nodes": [
            {"id": "gen", "type": "Sine", "params": {"freq": 1000, "amp": 0.8}},
            {"id": "awgn", "type": "AWGN", "params": {"snr_db": 30, "seed": 1}},
            {"id": "fft", "type": "FFTMag", "params": {}},
        ],
        "edges": [
            {"from": "gen", "to": "awgn"},
            {"from": "awgn", "to": "fft"},
        ],
    }
    save_pipeline(path, pipeline)


def test_run_smoke_dry_run_executes_blocks(tmp_path):
    pipeline_path = tmp_path / "smoke.json"
    _make_pipeline(pipeline_path)

    result = run_smoke(
        pipeline_path=str(pipeline_path),
        seconds=0.1,
        output_node=None,
        dry_run=True,
    )

    assert result["mode"] == "dry-run"
    assert result["blocks"] >= 1
    assert result["xruns"] == 0


def test_cli_main_dry_run_returns_zero(tmp_path, capsys):
    pipeline_path = tmp_path / "smoke.json"
    _make_pipeline(pipeline_path)

    code = main([str(pipeline_path), "--dry-run", "--seconds", "0.05"])
    captured = capsys.readouterr()

    assert code == 0
    assert "mode=dry-run" in captured.out
