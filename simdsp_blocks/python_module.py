from __future__ import annotations

import importlib
import importlib.util
from pathlib import Path
from types import ModuleType
from typing import Any

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class PythonModuleBlock(Block):
    SPEC = BlockSpec(
        type_name="PythonModule",
        implementation="python-loader",
        inputs=None,
        outputs=1,
        description="Load a user-defined DSP block from a Python module or file.",
        params=(
            ParamSpec("path", "str|null", None, "Path to a Python file containing the block class."),
            ParamSpec("module", "str|null", None, "Importable Python module name containing the block class."),
            ParamSpec("class_name", "str", "UserBlock", "Block class name inside the target module."),
        ),
    )

    def __init__(
        self,
        path: str | None = None,
        module: str | None = None,
        class_name: str = "UserBlock",
        context: dict[str, Any] | None = None,
        **block_params: Any,
    ) -> None:
        super().__init__(path=path, module=module, class_name=class_name, **block_params)
        self.path = path
        self.module = module
        self.class_name = class_name
        self.context = dict(context or {})
        self.block_params = dict(block_params)
        self._inner = None

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)
        block_cls = self._resolve_block_class()
        self._inner = block_cls(**self.block_params)
        self._inner.init(self.sr, self.bs, self.ch)

    def process(self, inputs):
        if self._inner is None:
            raise RuntimeError("PythonModule block has not been initialized.")
        return self._inner.process(inputs)

    def teardown(self):
        if self._inner is not None:
            self._inner.teardown()
            self._inner = None

    def _resolve_block_class(self):
        target_module = self._load_module()
        if not hasattr(target_module, self.class_name):
            raise ValueError(
                f"Class '{self.class_name}' was not found in module '{target_module.__name__}'."
            )
        return getattr(target_module, self.class_name)

    def _load_module(self) -> ModuleType:
        if self.path and self.module:
            raise ValueError("PythonModule block expects either 'path' or 'module', not both.")
        if self.path:
            return self._load_module_from_path(self.path)
        if self.module:
            return importlib.import_module(self.module)
        raise ValueError("PythonModule block requires either 'path' or 'module'.")

    def _load_module_from_path(self, raw_path: str) -> ModuleType:
        path = Path(raw_path)
        pipeline_dir = self.context.get("pipeline_dir")
        if not path.is_absolute() and pipeline_dir:
            path = Path(pipeline_dir) / path
        path = path.resolve()
        if not path.exists():
            raise ValueError(f"PythonModule source file does not exist: {path}")

        module_name = f"simdsp_user_{path.stem}_{abs(hash(str(path)))}"
        spec = importlib.util.spec_from_file_location(module_name, path)
        if spec is None or spec.loader is None:
            raise ValueError(f"Could not load Python module from path: {path}")
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return module
