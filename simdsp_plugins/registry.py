from __future__ import annotations

import importlib.util
from pathlib import Path
from typing import Iterable

from simdsp_core import BlockSpec
from simdsp_core.registry import BlockRegistry
from simdsp_native import NativeBlockAdapter
from simdsp_plugins.manifest import SimDSPPluginManifest, discover_plugin_manifests



def _load_module_from_path(path: Path):
    module_name = f"simdsp_plugin_{path.stem}_{abs(hash(str(path)))}"
    spec = importlib.util.spec_from_file_location(module_name, path)
    if spec is None or spec.loader is None:
        raise ValueError(f"Could not load plugin Python module from path: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module



def register_plugin_manifest(
    registry: BlockRegistry,
    manifest: SimDSPPluginManifest,
) -> BlockRegistry:
    for block in manifest.blocks:
        module_path = (manifest.root_dir / block.path).resolve()
        if not module_path.exists():
            raise ValueError(
                f"Plugin '{manifest.plugin_id}' references missing module file '{module_path}'."
            )
        module = _load_module_from_path(module_path)
        if block.kind == "python-file":
            block_cls = getattr(module, block.class_name)
            registry.register_class(
                block_cls,
                type_name=block.type_name,
                replace_existing=block.replace_existing,
            )
            continue

        backend_cls = getattr(module, block.backend_class)
        spec = block.spec
        if spec is None:
            raise ValueError(
                f"Plugin '{manifest.plugin_id}' native block '{block.backend_class}' is missing spec metadata."
            )
        if block.type_name and block.type_name != spec.type_name:
            spec = BlockSpec(
                type_name=block.type_name,
                implementation=spec.implementation,
                inputs=spec.inputs,
                outputs=spec.outputs,
                category=spec.category,
                display_name=spec.display_name,
                description=spec.description,
                tags=spec.tags,
                params=spec.params,
                accepts_extra_params=spec.accepts_extra_params,
            )
        registry.register(
            spec,
            lambda params, context=None, backend_cls=backend_cls, spec=spec: NativeBlockAdapter(backend_cls, spec, **params),
            replace_existing=block.replace_existing,
        )
    return registry



def register_plugins_from_dirs(
    search_dirs: Iterable[str | Path],
    registry: BlockRegistry | None = None,
) -> tuple[BlockRegistry, list[SimDSPPluginManifest]]:
    registry = registry or BlockRegistry()
    manifests = discover_plugin_manifests(search_dirs)
    for manifest in manifests:
        register_plugin_manifest(registry, manifest)
    return registry, manifests
