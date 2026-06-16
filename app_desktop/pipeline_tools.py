from __future__ import annotations

from copy import deepcopy
from pathlib import Path
from typing import Any

from simdsp_blocks import AWGN, FFTMag, ScopeTap, Sine
from simdsp_io import PIPELINE_SCHEMA_VERSION, load_pipeline, pipeline_to_engine

BLOCK_REGISTRY = {
    "Sine": Sine,
    "AWGN": AWGN,
    "FFTMag": FFTMag,
    "ScopeTap": ScopeTap,
}


def create_block(block_type: str, params: dict[str, Any]) -> object:
    if block_type not in BLOCK_REGISTRY:
        raise ValueError(f"Unknown block type '{block_type}'.")
    return BLOCK_REGISTRY[block_type](**params)


def default_pipeline(
    sample_rate: float = 48_000.0,
    block_size: int = 1024,
    channels: int = 1,
) -> dict[str, Any]:
    return {
        "schema_version": PIPELINE_SCHEMA_VERSION,
        "sample_rate": float(sample_rate),
        "block_size": int(block_size),
        "channels": int(channels),
        "nodes": [
            {"id": "gen", "type": "Sine", "params": {"freq": 1000.0, "amp": 0.8}},
            {"id": "awgn", "type": "AWGN", "params": {"snr_db": 30.0, "seed": 1}},
            {"id": "scope", "type": "ScopeTap", "params": {}},
            {"id": "fft", "type": "FFTMag", "params": {"window": "hann"}},
        ],
        "edges": [
            {"from": "gen", "to": "awgn"},
            {"from": "awgn", "to": "scope"},
            {"from": "awgn", "to": "fft"},
        ],
    }


def engine_from_pipeline(pipeline: dict[str, Any]):
    return pipeline_to_engine(pipeline, create_block)


def engine_from_pipeline_path(path: str | Path):
    return engine_from_pipeline(load_pipeline(path))


def sync_pipeline_from_engine(pipeline: dict[str, Any], engine) -> dict[str, Any]:
    snapshot = deepcopy(pipeline)
    snapshot["sample_rate"] = float(engine.sr)
    snapshot["block_size"] = int(engine.bs)
    snapshot["channels"] = int(engine.ch)

    for node in snapshot["nodes"]:
        runtime_node = engine.nodes.get(node["id"])
        if runtime_node is None:
            continue
        params = node.setdefault("params", {})
        for param_name in list(params.keys()):
            if hasattr(runtime_node.block, param_name):
                params[param_name] = getattr(runtime_node.block, param_name)

    return snapshot
