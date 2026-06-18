from __future__ import annotations

import pytest

from simdsp_blocks.catalog import create_block, lookup_block_spec
from simdsp_io.pipeline_json import PIPELINE_SCHEMA_VERSION, pipeline_to_engine, validate_pipeline


def _pipeline_with_node(node: dict):
    return {
        "schema_version": PIPELINE_SCHEMA_VERSION,
        "sample_rate": 48_000,
        "block_size": 256,
        "channels": 1,
        "nodes": [
            node,
            {"id": "scope", "type": "ScopeTap", "params": {}},
        ],
        "edges": [{"from": node["id"], "to": "scope"}],
    }


def test_validate_pipeline_rejects_unknown_param_for_strict_block():
    pipeline = _pipeline_with_node({"id": "gen", "type": "Sine", "params": {"freq": 1000, "amp": 0.8, "phase": 0.0}})

    with pytest.raises(ValueError, match=r"unknown parameter\(s\): phase"):
        validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_validate_pipeline_rejects_wrong_param_type():
    pipeline = _pipeline_with_node({"id": "gen", "type": "Sine", "params": {"freq": "fast", "amp": 0.8}})

    with pytest.raises(ValueError, match="parameter 'freq'.*must match type 'float'"):
        validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_validate_pipeline_rejects_param_below_minimum():
    pipeline = _pipeline_with_node({"id": "gen", "type": "Sine", "params": {"freq": -10.0, "amp": 0.8}})

    with pytest.raises(ValueError, match=r"parameter 'freq'.*must be >= 0.0"):
        validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_validate_pipeline_rejects_choice_outside_catalog():
    pipeline = {
        "schema_version": PIPELINE_SCHEMA_VERSION,
        "sample_rate": 48_000,
        "block_size": 256,
        "channels": 1,
        "nodes": [
            {"id": "gen", "type": "Sine", "params": {"freq": 1000, "amp": 0.8}},
            {"id": "fft", "type": "FFTMag", "params": {"window": "blackman"}},
        ],
        "edges": [{"from": "gen", "to": "fft"}],
    }

    with pytest.raises(ValueError, match="must be one of"):
        validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_validate_pipeline_accepts_nullable_and_union_param_types():
    pipeline = _pipeline_with_node({"id": "src", "type": "AudioInput", "params": {"device": None, "latency": "low", "dtype": "float32"}})

    validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_validate_pipeline_allows_extra_params_for_python_module_block():
    pipeline = _pipeline_with_node({
        "id": "user",
        "type": "PythonModule",
        "params": {"path": "user_gain.py", "class_name": "UserBlock", "gain": 0.25},
    })

    validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_pipeline_to_engine_rejects_invalid_param_before_block_construction():
    pipeline = _pipeline_with_node({"id": "gen", "type": "Sine", "params": {"freq": 1000, "amp": 3.5}})

    with pytest.raises(ValueError, match=r"parameter 'amp'.*must be <= 1.0"):
        pipeline_to_engine(pipeline, create_block, spec_lookup=lookup_block_spec)
