from __future__ import annotations
from dataclasses import dataclass
from typing import Dict, List, Tuple
import numpy as np
import threading, time, queue

@dataclass
class Node:
    id: str
    block: object
    inputs: List[str]  # upstream node ids
    outputs: int = 1

class Engine:
    def __init__(self, sample_rate=48000.0, block_size=1024, channels=1):
        self.sr = float(sample_rate)
        self.bs = int(block_size)
        self.ch = int(channels)
        self.nodes: Dict[str, Node] = {}
        self._run = False
        self._th: threading.Thread | None = None
        # queues for UI taps
        self.scope_q: "queue.Queue[np.ndarray]" = queue.Queue(maxsize=8)
        self.fft_q: "queue.Queue[np.ndarray]" = queue.Queue(maxsize=8)

    def add_node(self, node_id: str, block, inputs: List[str], outputs: int = 1) -> None:
        self.nodes[node_id] = Node(node_id, block, inputs, outputs)

    def _topological_order(self) -> List[str]:
        # Simple linear eval by insertion order; later you can compute true topo sort.
        return list(self.nodes.keys())

    def init(self) -> None:
        for n in self.nodes.values():
            n.block.init(self.sr, self.bs, self.ch)

    def start(self) -> None:
        if self._run: return
        self._run = True
        self._th = threading.Thread(target=self._loop, daemon=True)
        self._th.start()

    def stop(self) -> None:
        self._run = False
        if self._th:
            self._th.join(timeout=1.0)
        for n in self.nodes.values():
            try:
                n.block.teardown()
            except Exception:
                pass  # teardown must not crash the engine

    def _loop(self) -> None:
        buffers: Dict[str, List[np.ndarray]] = {}
        order = self._topological_order()
        while self._run:
            for node_id in order:
                node = self.nodes[node_id]
                if node.inputs:
                    ins = buffers[node.inputs[0]]
                else:
                    ins = [np.zeros((self.bs, self.ch), dtype=np.float32)]
                outs = node.block.process(ins)
                buffers[node_id] = outs

            # publish to UI (optional taps)
            if "scope" in buffers:
                try: self.scope_q.put_nowait(buffers["scope"][0])
                except queue.Full: pass
            if "fft" in buffers:
                try: self.fft_q.put_nowait(buffers["fft"][0])
                except queue.Full: pass

            time.sleep(self.bs / self.sr)
