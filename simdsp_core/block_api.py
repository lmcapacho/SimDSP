"""
Base interface for DSP blocks.
"""
from typing import Any, List
import numpy as np

class Block:
    """Base interface for DSP blocks."""
    def __init__(self, **params: Any) -> None:
        self.params = params
        self.sr = 0.0
        self.bs = 0
        self.ch = 0

    def init(self, sample_rate: float, block_size: int, channels: int) -> None:
        """Allocate buffers and initialize internal state."""
        self.sr = float(sample_rate)
        self.bs = int(block_size)
        self.ch = int(channels)

    def process(self, inputs: List[np.ndarray]) -> List[np.ndarray]:
        """Core processing. inputs: list of float32 arrays, shape (N, C)."""
        raise NotImplementedError

    def teardown(self) -> None:
        """Release resources (streams, files, threads, native handles)."""
        return
