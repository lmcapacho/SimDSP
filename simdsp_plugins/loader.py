from __future__ import annotations

from dataclasses import dataclass, field
import importlib.util
import json
from pathlib import Path
from typing import Any, Iterable

from simdsp_core import BlockSpec, ParamSpec
from simdsp_core.registry import BlockRegistry
from simdsp_native import NativeBlockAdapter

MANIFEST_FILENAME = "simdsp_plugin.json"


@dataclass(frozen=True)
class PluginBlockDefinition:
    kind: str
    path: str
    class_name: str | None = None
    backend_class: str | None = None
    type_name: str | None = None
    replace_existing: bool = False
    spec: BlockSpec | None = None


@dataclass(frozen=True)
class SimDSPPluginManifest:
    plugin_id: str
    name: str
    version: str
    root_dir: Path
    manifest_path: Path
    blocks: tuple[PluginBlockDefinition, ...] = field(default_factory=tuple)



def _param_spec_from_dict(data: dict[str, Any]) -> ParamSpec:
    return ParamSpec(
        name=data["name"],
        param_type=data["param_type"],
        default=data.get("default"),
        description=data.get("description", ""),
        min_value=data.get("min_value"),
        max_value=data.get("max_value"),
        step=data.get("step"),
        unit=data.get("unit", ""),
        choices=tuple(data.get("choices", ())),
    )



def _block_spec_from_dict(data: dict[str, Any]) -> BlockSpec:
    return BlockSpec(
        type_name=data["type_name"],
        implementation=data["implementation"],
        inputs=data["inputs"],
        outputs=data["outputs"],
        category=data.get("category", "misc"),
        display_name=data.get("display_name", ""),
        description=data.get("description", ""),
        tags=tuple(data.get("tags", ())),
        params=tuple(_param_spec_from_dict(param) for param in data.get("params", ())),
        accepts_extra_params=bool(data.get("accepts_extra_params", False)),
    )



def load_plugin_manifest(path: str | Path) -> SimDSPPluginManifest:
    manifest_path = Path(path).resolve()
    root_dir = manifest_path.parent
    payload = json.loads(manifest_path.read_text(encoding="utf-8"))

    for field_name in ("plugin_id", "name", "version", "blocks"):
        if field_name not in payload:
            raise ValueError(f"Plugin manifest '{manifest_path}' is missing required field '{field_name}'.")

    blocks = []
    for entry in payload["blocks"]:
        kind = entry.get("kind")
        if kind not in {"python-file", "native-python"}:
            raise ValueError(
                f"Plugin manifest '{manifest_path}' contains unsupported block kind '{kind}'."
            )
        if "path" not in entry:
            raise ValueError(f"Plugin manifest '{manifest_path}' block entry is missing 'path'.")
        if kind == "python-file" and "class_name" not in entry:
            raise ValueError(
                f"Plugin manifest '{manifest_path}' python-file block is missing 'class_name'."
            )
        if kind == "native-python" and "backend_class" not in entry:
            raise ValueError(
                f"Plugin manifest '{manifest_path}' native-python block is missing 'backend_class'."
            )
        spec = None
        if kind == "native-python":
            if "spec" not in entry:
                raise ValueError(
                    f"Plugin manifest '{manifest_path}' native-python block is missing 'spec'."
                )
            spec = _block_spec_from_dict(entry["spec"])
        blocks.append(
            PluginBlockDefinition(
                kind=kind,
                path=entry["path"],
                class_name=entry.get("class_name"),
                backend_class=entry.get("backend_class"),
                type_name=entry.get("type_name"),
                replace_existing=bool(entry.get("replace_existing", False)),
                spec=spec,
            )
        )

    return SimDSPPluginManifest(
        plugin_id=payload["plugin_id"],
        name=payload["name"],
        version=payload["version"],
        root_dir=root_dir,
        manifest_path=manifest_path,
        blocks=tuple(blocks),
    )



def discover_plugin_manifests(search_dirs: Iterable[str | Path]) -> list[SimDSPPluginManifest]:
    manifests: list[SimDSPPluginManifest] = []
    for raw_dir in search_dirs:
        base_dir = Path(raw_dir).resolve()
        if not base_dir.exists():
            continue
        for manifest_path in sorted(base_dir.rglob(MANIFEST_FILENAME)):
            manifests.append(load_plugin_manifest(manifest_path))
    return manifests



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
