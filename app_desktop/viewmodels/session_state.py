from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Literal

ViewMode = Literal["lab", "pipeline"]


@dataclass
class SessionState:
    current_pipeline_path: Path | None = None
    running: bool = False
    selected_node_id: str | None = None
    selected_edge_index: int | None = None
    selected_block_type: str | None = None
    view_mode: ViewMode = "lab"
