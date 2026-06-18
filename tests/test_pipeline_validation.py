from __future__ import annotations

import pytest

from simdsp_blocks.catalog import create_block, lookup_block_spec
from simdsp_io.pipeline_json import PIPELINE_SCHEMA_VERSION, pipeline_to_engine, validate_pipeline


def _base_pipeline():
    return {
        "schema_version": PIPELINE_SCHEMA_VERSION,
        "sample_rate": 48_000,
        "block_size": 256,
        "channels": 1,
        "nodes": [
            {"id": "gen", "role": "source", "type": "Sine", "params": {"freq": 1000, "amp": 0.8}},
            {"id": "gain", "role": "gain", "type": "Gain", "params": {"gain": 1.0}},
            {"id": "scope", "role": "scope", "type": "ScopeTap", "params": {}},
        ],
        "edges": [
            {"from": "gen", "to": "gain"},
            {"from": "gain", "to": "scope"},
        ],
    }


def test_validate_pipeline_accepts_semantically_valid_pipeline():
    validate_pipeline(_base_pipeline(), spec_lookup=lookup_block_spec)


def test_validate_pipeline_rejects_wrong_input_count():
    pipeline = _base_pipeline()
    pipeline["edges"] = [{"from": "gen", "to": "scope"}]

    with pytest.raises(ValueError, match=r"expects 1 input\(s\) but pipeline provides 0"):
        validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_validate_pipeline_rejects_incompatible_reserved_role():
    pipeline = _base_pipeline()
    pipeline["nodes"][0]["role"] = "gain"

    with pytest.raises(ValueError, match="not compatible with reserved role 'gain'"):
        validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_validate_pipeline_rejects_duplicate_reserved_role():
    pipeline = _base_pipeline()
    pipeline["nodes"].append({"id": "gain2", "role": "gain", "type": "Gain", "params": {"gain": 0.5}})
    pipeline["edges"].append({"from": "gen", "to": "gain2"})

    with pytest.raises(ValueError, match="Reserved role 'gain' must appear at most once"):
        validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_validate_pipeline_rejects_duplicate_edges():
    pipeline = _base_pipeline()
    pipeline["edges"].append({"from": "gen", "to": "gain"})

    with pytest.raises(ValueError, match="duplicate edge"):
        validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_validate_pipeline_rejects_self_loop():
    pipeline = _base_pipeline()
    pipeline["edges"] = [{"from": "gen", "to": "gen"}]

    with pytest.raises(ValueError, match="cannot connect a node to itself"):
        validate_pipeline(pipeline, spec_lookup=lookup_block_spec)


def test_pipeline_to_engine_rejects_unknown_block_type_early():
    pipeline = _base_pipeline()
    pipeline["nodes"][1]["type"] = "MissingBlock"

    with pytest.raises(ValueError, match="Unknown block type 'MissingBlock'"):
        pipeline_to_engine(pipeline, create_block, spec_lookup=lookup_block_spec)
