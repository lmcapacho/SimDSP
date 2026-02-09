from __future__ import annotations

from copy import deepcopy
import json
from pathlib import Path
from typing import Callable, Dict, List, Any

from simdsp_core.engine import Engine

PIPELINE_SCHEMA_VERSION = 1


def _validate_pipeline_dict(pipeline: Dict[str, Any]) -> None:
    required = {"schema_version", "sample_rate", "block_size", "channels", "nodes"}
    missing = required - set(pipeline.keys())
    if missing:
        missing_fields = ", ".join(sorted(missing))
        raise ValueError(f"Pipeline is missing required fields: {missing_fields}.")

    if int(pipeline["schema_version"]) != PIPELINE_SCHEMA_VERSION:
        raise ValueError(
            f"Unsupported schema_version={pipeline['schema_version']}. "
            f"Expected {PIPELINE_SCHEMA_VERSION}."
        )

    node_ids: List[str] = []
    for node in pipeline["nodes"]:
        if "id" not in node or "type" not in node:
            raise ValueError("Each node must include 'id' and 'type'.")
        node_ids.append(node["id"])

    if len(node_ids) != len(set(node_ids)):
        raise ValueError("Pipeline nodes contain duplicate ids.")

    node_set = set(node_ids)
    for edge in pipeline.get("edges", []):
        if "from" not in edge or "to" not in edge:
            raise ValueError("Each edge must include 'from' and 'to'.")
        if edge["from"] not in node_set or edge["to"] not in node_set:
            raise ValueError("Each edge must reference existing nodes.")


def _inputs_from_pipeline(pipeline: Dict[str, Any]) -> Dict[str, List[str]]:
    if "edges" in pipeline:
        inputs: Dict[str, List[str]] = {node["id"]: [] for node in pipeline["nodes"]}
        for edge in pipeline["edges"]:
            inputs[edge["to"]].append(edge["from"])
        return inputs
    return {node["id"]: list(node.get("inputs", [])) for node in pipeline["nodes"]}


def save_pipeline(path: str | Path, pipeline: Dict[str, Any]) -> None:
    _validate_pipeline_dict(pipeline)
    p = Path(path)
    p.parent.mkdir(parents=True, exist_ok=True)
    with p.open("w", encoding="utf-8") as f:
        json.dump(pipeline, f, indent=2)


def load_pipeline(path: str | Path) -> Dict[str, Any]:
    p = Path(path)
    with p.open("r", encoding="utf-8") as f:
        pipeline = json.load(f)
    _validate_pipeline_dict(pipeline)
    return pipeline


def pipeline_to_engine(
    pipeline: Dict[str, Any],
    block_factory: Callable[[str, Dict[str, Any]], object],
) -> Engine:
    _validate_pipeline_dict(pipeline)
    inputs_by_node = _inputs_from_pipeline(pipeline)

    engine = Engine(
        sample_rate=float(pipeline["sample_rate"]),
        block_size=int(pipeline["block_size"]),
        channels=int(pipeline["channels"]),
    )

    for node in pipeline["nodes"]:
        params = deepcopy(node.get("params", {}))
        block = block_factory(node["type"], params)
        engine.add_node(node["id"], block, inputs_by_node[node["id"]])

    engine.init()
    return engine
