from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Protocol

import numpy as np


@dataclass(frozen=True)
class ParamSpec:
    name: str
    param_type: str
    default: Any
    description: str = ""
    min_value: float | None = None
    max_value: float | None = None
    step: float | None = None
    unit: str = ""
    choices: tuple[Any, ...] = field(default_factory=tuple)


@dataclass(frozen=True)
class BlockSpec:
    type_name: str
    implementation: str
    inputs: int | None
    outputs: int
    category: str = "misc"
    display_name: str = ""
    description: str = ""
    tags: tuple[str, ...] = field(default_factory=tuple)
    params: tuple[ParamSpec, ...] = field(default_factory=tuple)


class Block:
    SPEC = BlockSpec(
        type_name="Block",
        implementation="python",
        inputs=None,
        outputs=1,
        category="misc",
        display_name="Block",
        description="Base DSP block.",
        tags=("base",),
    )

    def __init__(self, **params: Any) -> None:
        self.params = dict(params)
        self.sr = 0.0
        self.bs = 0
        self.ch = 0

    def init(self, sample_rate: float, block_size: int, channels: int) -> None:
        self.sr = float(sample_rate)
        self.bs = int(block_size)
        self.ch = int(channels)

    def process(self, inputs: list[np.ndarray]) -> list[np.ndarray]:
        raise NotImplementedError

    def teardown(self) -> None:
        return


class NativeBlockBackend(Protocol):
    def init(self, sample_rate: float, block_size: int, channels: int) -> None: ...

    def process(self, inputs: list[np.ndarray]) -> list[np.ndarray]: ...

    def teardown(self) -> None: ...
