from __future__ import annotations

import numpy as np

from app_desktop.pipeline_tools import create_block
from simdsp_io.pipeline_json import PIPELINE_SCHEMA_VERSION, load_pipeline, pipeline_to_engine, save_pipeline


def test_python_module_block_can_load_user_block_by_path(tmp_path):
    user_block = tmp_path / "user_gain.py"
    user_block.write_text(
        "from simdsp_core import Block\n"
        "import numpy as np\n\n"
        "class UserBlock(Block):\n"
        "    def __init__(self, gain=0.5):\n"
        "        super().__init__(gain=gain)\n"
        "        self.gain = float(gain)\n\n"
        "    def process(self, inputs):\n"
        "        x = inputs[0].astype(np.float32, copy=False)\n"
        "        return [np.clip(x * self.gain, -1.0, 1.0)]\n"
    )

    pipeline = {
        "schema_version": PIPELINE_SCHEMA_VERSION,
        "sample_rate": 48_000,
        "block_size": 256,
        "channels": 1,
        "nodes": [
            {"id": "gen", "type": "Sine", "params": {"freq": 1000, "amp": 0.8}},
            {"id": "gain", "type": "PythonModule", "params": {"path": "user_gain.py", "gain": 0.25}},
        ],
        "edges": [
            {"from": "gen", "to": "gain"},
        ],
    }

    pipeline_path = tmp_path / "pipeline.json"
    save_pipeline(pipeline_path, pipeline)
    loaded = load_pipeline(pipeline_path)
    engine = pipeline_to_engine(
        loaded,
        create_block,
        context={"pipeline_path": str(pipeline_path), "pipeline_dir": str(tmp_path)},
    )
    buffers = engine.run_once()

    out = buffers["gain"][0]
    assert np.max(np.abs(out)) <= 0.25 + 1e-3
