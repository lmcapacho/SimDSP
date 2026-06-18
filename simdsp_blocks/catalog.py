from __future__ import annotations

from copy import deepcopy
from pathlib import Path
from typing import Any

from simdsp_blocks import register_builtin_blocks
from simdsp_core.registry import BlockRegistry
from simdsp_plugins import register_plugins_from_dirs



def create_registry(plugin_dirs: list[str | Path] | None = None) -> BlockRegistry:
    registry = register_builtin_blocks(BlockRegistry())
    if plugin_dirs:
        register_plugins_from_dirs(plugin_dirs, registry)
    return registry


DEFAULT_BLOCK_REGISTRY = create_registry()



def create_block(block_type: str, params: dict[str, Any], context: dict[str, Any] | None = None) -> object:
    return DEFAULT_BLOCK_REGISTRY.create(block_type, params, context)



def list_block_specs():
    return DEFAULT_BLOCK_REGISTRY.list_specs()



def lookup_block_spec(type_name: str):
    return DEFAULT_BLOCK_REGISTRY.spec(type_name)



def block_defaults(type_name: str) -> dict[str, Any]:
    spec = DEFAULT_BLOCK_REGISTRY.spec(type_name)
    return {param.name: deepcopy(param.default) for param in spec.params}
