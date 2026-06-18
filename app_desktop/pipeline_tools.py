from __future__ import annotations

from copy import deepcopy
from pathlib import Path
from typing import Any

from simdsp_blocks import register_builtin_blocks
from simdsp_core.registry import BlockRegistry
from simdsp_io import PIPELINE_SCHEMA_VERSION, load_pipeline, pipeline_to_engine

DEFAULT_BLOCK_REGISTRY = register_builtin_blocks(BlockRegistry())
SOURCE_ROLE = "source"
NOISE_ROLE = "noise"
GAIN_ROLE = "gain"
SCOPE_ROLE = "scope"
FFT_ROLE = "fft"


def create_block(block_type: str, params: dict[str, Any], context: dict[str, Any] | None = None) -> object:
    return DEFAULT_BLOCK_REGISTRY.create(block_type, params, context)


def list_block_specs():
    return DEFAULT_BLOCK_REGISTRY.list_specs()


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
            {"id": "gen", "role": SOURCE_ROLE, "type": "Sine", "params": {"freq": 1000.0, "amp": 0.8}},
            {"id": "awgn", "role": NOISE_ROLE, "type": "AWGN", "params": {"snr_db": 30.0, "seed": 1}},
            {"id": "gain", "role": GAIN_ROLE, "type": "Gain", "params": {"gain": 1.0}},
            {"id": "scope", "role": SCOPE_ROLE, "type": "ScopeTap", "params": {}},
            {"id": "fft", "role": FFT_ROLE, "type": "FFTMag", "params": {"window": "hann"}},
        ],
        "edges": [
            {"from": "gen", "to": "awgn"},
            {"from": "awgn", "to": "gain"},
            {"from": "gain", "to": "scope"},
            {"from": "gain", "to": "fft"},
        ],
    }


def block_defaults(type_name: str) -> dict[str, Any]:
    spec = DEFAULT_BLOCK_REGISTRY.spec(type_name)
    return {param.name: deepcopy(param.default) for param in spec.params}


def find_node_by_role(pipeline: dict[str, Any], role: str) -> dict[str, Any] | None:
    for node in pipeline.get("nodes", []):
        if node.get("role") == role:
            return node
    return None


def apply_quick_experiment(
    pipeline: dict[str, Any],
    *,
    source_type: str,
    source_params: dict[str, Any] | None = None,
    snr_db: float | None = None,
    gain: float | None = None,
) -> dict[str, Any]:
    updated = deepcopy(pipeline)

    source_node = find_node_by_role(updated, SOURCE_ROLE)
    noise_node = find_node_by_role(updated, NOISE_ROLE)
    gain_node = find_node_by_role(updated, GAIN_ROLE)

    if source_node is None or noise_node is None or gain_node is None:
        raise ValueError("Quick experiment requires source, noise, and gain roles in the pipeline.")

    source_node["type"] = source_type
    source_node["params"] = block_defaults(source_type)
    source_node["params"].update(source_params or {})

    if snr_db is not None:
        noise_node.setdefault("params", {})
        noise_node["params"]["snr_db"] = float(snr_db)

    if gain is not None:
        gain_node["type"] = "Gain"
        gain_node["params"] = {"gain": float(gain)}

    return updated


def engine_from_pipeline(pipeline: dict[str, Any], pipeline_path: str | Path | None = None):
    context = {}
    if pipeline_path is not None:
        resolved = Path(pipeline_path).resolve()
        context = {"pipeline_path": str(resolved), "pipeline_dir": str(resolved.parent)}
    return pipeline_to_engine(pipeline, create_block, context=context)


def engine_from_pipeline_path(path: str | Path):
    return engine_from_pipeline(load_pipeline(path), pipeline_path=path)


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
