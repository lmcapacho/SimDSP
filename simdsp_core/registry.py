from __future__ import annotations

from dataclasses import replace
from typing import Callable

from simdsp_core.block_api import BlockSpec


BlockFactory = Callable[[dict], object]


class BlockRegistry:
    def __init__(self) -> None:
        self._specs: dict[str, BlockSpec] = {}
        self._factories: dict[str, BlockFactory] = {}

    def register(self, spec: BlockSpec, factory: BlockFactory, *, replace_existing: bool = False) -> None:
        if spec.type_name in self._specs and not replace_existing:
            raise ValueError(f"Block type '{spec.type_name}' is already registered.")
        self._specs[spec.type_name] = replace(spec)
        self._factories[spec.type_name] = factory

    def register_class(self, block_cls, *, type_name: str | None = None, replace_existing: bool = False) -> None:
        spec = getattr(block_cls, "SPEC", None)
        if spec is None:
            raise ValueError(f"Block class '{block_cls.__name__}' does not define SPEC.")
        if type_name and type_name != spec.type_name:
            spec = replace(spec, type_name=type_name)
        self.register(spec, lambda params: block_cls(**params), replace_existing=replace_existing)

    def create(self, type_name: str, params: dict | None = None):
        if type_name not in self._factories:
            raise ValueError(f"Unknown block type '{type_name}'.")
        return self._factories[type_name](dict(params or {}))

    def spec(self, type_name: str) -> BlockSpec:
        if type_name not in self._specs:
            raise ValueError(f"Unknown block type '{type_name}'.")
        return self._specs[type_name]

    def list_specs(self) -> list[BlockSpec]:
        return [self._specs[name] for name in sorted(self._specs.keys())]

    def is_registered(self, type_name: str) -> bool:
        return type_name in self._specs
