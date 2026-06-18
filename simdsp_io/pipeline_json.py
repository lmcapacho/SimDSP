from __future__ import annotations

import json
from pathlib import Path
from typing import Callable, Dict, Any

from simdsp_core.block_api import BlockSpec
from simdsp_io.pipeline_engine import pipeline_to_engine
from simdsp_io.pipeline_validation import PIPELINE_SCHEMA_VERSION, validate_pipeline



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


__all__ = ["PIPELINE_SCHEMA_VERSION", "load_pipeline", "pipeline_to_engine", "save_pipeline", "validate_pipeline"]
