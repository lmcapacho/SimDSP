from __future__ import annotations

from copy import deepcopy
import inspect
from typing import Any, Callable, Dict

from simdsp_core.block_api import BlockSpec
from simdsp_core.engine import Engine
from simdsp_io.pipeline_validation import inputs_from_pipeline, validate_pipeline



def pipeline_to_engine(
    pipeline: Dict[str, Any],
    block_factory: Callable[[str, Dict[str, Any]], object],
    context: Dict[str, Any] | None = None,
    spec_lookup: Callable[[str], BlockSpec] | None = None,
) -> Engine:
    validate_pipeline(pipeline, spec_lookup=spec_lookup)
    inputs_by_node = inputs_from_pipeline(pipeline)

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
