from __future__ import annotations

from pathlib import Path

import numpy as np
from scipy.io import savemat

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class MatFileSink(Block):
    SPEC = BlockSpec(
        type_name="MatFileSink",
        implementation="python",
        inputs=1,
        outputs=1,
        category="sink",
        display_name="MAT File Sink",
        description="Capture a signal stream and export it to a Matlab/Octave MAT file.",
        tags=("matlab", "octave", "file", "export"),
        params=(
            ParamSpec("path", "str", "", "Path to the output MAT file."),
            ParamSpec("variable", "str", "signal", "Variable name stored in the MAT file."),
            ParamSpec("write_on_teardown", "bool", True, "Write the MAT file when teardown runs."),
        ),
    )

    def __init__(
        self,
        path: str,
        variable: str = "signal",
        write_on_teardown: bool = True,
        context: dict | None = None,
    ) -> None:
        super().__init__(path=path, variable=variable, write_on_teardown=write_on_teardown)
        self.path = path
        self.variable = variable
        self.write_on_teardown = bool(write_on_teardown)
        self.context = dict(context or {})
        self._captured: list[np.ndarray] = []

    def process(self, inputs):
        x = np.asarray(inputs[0], dtype=np.float32)
        self._captured.append(x.copy())
        return [x]

    def teardown(self):
        if self.write_on_teardown:
            self.flush()

    def flush(self) -> None:
        path = self._resolve_path(self.path)
        path.parent.mkdir(parents=True, exist_ok=True)
        signal = np.concatenate(self._captured, axis=0) if self._captured else np.zeros((0, self.ch), dtype=np.float32)
        savemat(
            path,
            {
                self.variable: signal,
                "sample_rate": np.array([[self.sr]], dtype=np.float32),
                "channels": np.array([[self.ch]], dtype=np.int32),
                "block_size": np.array([[self.bs]], dtype=np.int32),
            },
        )

    def _resolve_path(self, raw_path: str) -> Path:
        path = Path(raw_path)
        pipeline_dir = self.context.get("pipeline_dir")
        if not path.is_absolute() and pipeline_dir:
            path = Path(pipeline_dir) / path
        return path.resolve()
