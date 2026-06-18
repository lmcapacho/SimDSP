from __future__ import annotations

from copy import deepcopy
import json
from pathlib import Path
from typing import Callable, Dict, List, Any
import inspect

from simdsp_core.block_api import BlockSpec
from simdsp_core.engine import Engine

PIPELINE_SCHEMA_VERSION = 1
RESERVED_ROLES = {"source", "noise", "gain", "scope", "fft"}


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

    if float(pipeline["sample_rate"]) <= 0:
        raise ValueError("Pipeline sample_rate must be > 0.")
    if int(pipeline["block_size"]) <= 0:
        raise ValueError("Pipeline block_size must be > 0.")
    if int(pipeline["channels"]) <= 0:
        raise ValueError("Pipeline channels must be > 0.")

    node_ids: List[str] = []
    for node in pipeline["nodes"]:
        if "id" not in node or "type" not in node:
            raise ValueError("Each node must include 'id' and 'type'.")
        node_ids.append(node["id"])

    if len(node_ids) != len(set(node_ids)):
        raise ValueError("Pipeline nodes contain duplicate ids.")

    node_set = set(node_ids)
    seen_edges: set[tuple[str, str]] = set()
    for edge in pipeline.get("edges", []):
        if "from" not in edge or "to" not in edge:
            raise ValueError("Each edge must include 'from' and 'to'.")
        if edge["from"] not in node_set or edge["to"] not in node_set:
            raise ValueError("Each edge must reference existing nodes.")
        if edge["from"] == edge["to"]:
            raise ValueError("A pipeline edge cannot connect a node to itself.")
        edge_key = (edge["from"], edge["to"])
        if edge_key in seen_edges:
            raise ValueError(f"Pipeline contains duplicate edge '{edge['from']} -> {edge['to']}'.")
        seen_edges.add(edge_key)


def _inputs_from_pipeline(pipeline: Dict[str, Any]) -> Dict[str, List[str]]:
    if "edges" in pipeline:
        inputs: Dict[str, List[str]] = {node["id"]: [] for node in pipeline["nodes"]}
        for edge in pipeline["edges"]:
            inputs[edge["to"]].append(edge["from"])
        return inputs
    return {node["id"]: list(node.get("inputs", [])) for node in pipeline["nodes"]}


def _role_matches_spec(role: str, spec: BlockSpec) -> bool:
    if role == "source":
        return spec.inputs == 0 and spec.category in {"generator", "source"}
    if role == "noise":
        return spec.inputs == 1 and "noise" in spec.tags
    if role == "gain":
        return spec.inputs == 1 and "gain" in spec.tags
    if role == "scope":
        return spec.type_name == "ScopeTap" or (spec.inputs == 1 and "scope" in spec.tags)
    if role == "fft":
        return spec.type_name == "FFTMag" or (spec.inputs == 1 and "spectrum" in spec.tags)
    return True


def validate_pipeline(
    pipeline: Dict[str, Any],
    spec_lookup: Callable[[str], BlockSpec] | None = None,
) -> None:
    _validate_pipeline_dict(pipeline)

    inputs_by_node = _inputs_from_pipeline(pipeline)
    role_counts: dict[str, int] = {}

    for node in pipeline["nodes"]:
        node_id = node["id"]
        role = node.get("role")
        if role is not None:
            role_counts[role] = role_counts.get(role, 0) + 1
        if spec_lookup is None:
            continue

        spec = spec_lookup(node["type"])
        actual_inputs = len(inputs_by_node.get(node_id, []))
        if spec.inputs is not None and actual_inputs != spec.inputs:
            raise ValueError(
                f"Node '{node_id}' of type '{spec.type_name}' expects {spec.inputs} input(s) but pipeline provides {actual_inputs}."
            )
        if role is not None and role in RESERVED_ROLES and not _role_matches_spec(role, spec):
            raise ValueError(
                f"Node '{node_id}' of type '{spec.type_name}' is not compatible with reserved role '{role}'."
            )

    for role, count in role_counts.items():
        if role in RESERVED_ROLES and count > 1:
            raise ValueError(f"Reserved role '{role}' must appear at most once in a pipeline.")



def save_pipeline(
    path: str | Path,
    pipeline: Dict[str, Any],
    spec_lookup: Callable[[str], BlockSpec] | None = None,
) -> None:
    validate_pipeline(pipeline, spec_lookup=spec_lookup)
    p = Path(path)
    p.parent.mkdir(parents=True, exist_ok=True)
    with p.open("w", encoding="utf-8") as f:
        json.dump(pipeline, f, indent=2)



def load_pipeline(path: str | Path) -> Dict[str, Any]:
    p = Path(path)
    with p.open("r", encoding="utf-8") as f:
        pipeline = json.load(f)
    validate_pipeline(pipeline)
    return pipeline



def pipeline_to_engine(
    pipeline: Dict[str, Any],
    block_factory: Callable[[str, Dict[str, Any]], object],
    context: Dict[str, Any] | None = None,
    spec_lookup: Callable[[str], BlockSpec] | None = None,
) -> Engine:
    validate_pipeline(pipeline, spec_lookup=spec_lookup)
    inputs_by_node = _inputs_from_pipeline(pipeline)

    engine = Engine(
        sample_rate=float(pipeline["sample_rate"]),
        block_size=int(pipeline["block_size"]),
        channels=int(pipeline["channels"]),
    )

    factory_context = dict(context or {})

    for node in pipeline["nodes"]:
        params = deepcopy(node.get("params", {}))
        if len(inspect.signature(block_factory).parameters) >= 3:
            block = block_factory(node["type"], params, factory_context)
        else:
            block = block_factory(node["type"], params)
        engine.add_node(node["id"], block, inputs_by_node[node["id"]])

    engine.init()
    return engine
