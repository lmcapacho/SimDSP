from __future__ import annotations
from dataclasses import dataclass
from typing import Dict, List
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
        self._order: List[str] = []
        self._run = False
        self._th: threading.Thread | None = None
        # queues for UI taps
        self.scope_q: "queue.Queue[np.ndarray]" = queue.Queue(maxsize=8)
        self.fft_q: "queue.Queue[np.ndarray]" = queue.Queue(maxsize=8)

    def add_node(self, node_id: str, block, inputs: List[str], outputs: int = 1) -> None:
        if node_id in self.nodes:
            raise ValueError(f"Node '{node_id}' already exists.")
        self.nodes[node_id] = Node(node_id, block, inputs, outputs)
        self._order = []

    def _topological_order(self) -> List[str]:
        indegree: Dict[str, int] = {node_id: 0 for node_id in self.nodes}
        downstream: Dict[str, List[str]] = {node_id: [] for node_id in self.nodes}

        for node_id, node in self.nodes.items():
            for upstream_id in node.inputs:
                if upstream_id not in self.nodes:
                    raise ValueError(
                        f"Node '{node_id}' depends on missing input node '{upstream_id}'."
                    )
                indegree[node_id] += 1
                downstream[upstream_id].append(node_id)

        order: List[str] = []
        ready = [node_id for node_id, degree in indegree.items() if degree == 0]

        while ready:
            current = ready.pop(0)
            order.append(current)
            for node_id in downstream[current]:
                indegree[node_id] -= 1
                if indegree[node_id] == 0:
                    ready.append(node_id)

        if len(order) != len(self.nodes):
            raise ValueError("Cycle detected in DSP graph. The graph must be acyclic.")

        return order

    def init(self) -> None:
        self._order = self._topological_order()
        for n in self.nodes.values():
            n.block.init(self.sr, self.bs, self.ch)

    def run_once(self) -> Dict[str, List[np.ndarray]]:
        if not self._order:
            self._order = self._topological_order()

        buffers: Dict[str, List[np.ndarray]] = {}
        for node_id in self._order:
            node = self.nodes[node_id]
            if node.inputs:
                ins: List[np.ndarray] = []
                for upstream_id in node.inputs:
                    ins.extend(buffers[upstream_id])
            else:
                ins = [np.zeros((self.bs, self.ch), dtype=np.float32)]
            outs = node.block.process(ins)
            buffers[node_id] = outs

        # publish to UI (optional taps)
        if "scope" in buffers:
            try:
                self.scope_q.put_nowait(buffers["scope"][0])
            except queue.Full:
                pass
        if "fft" in buffers:
            try:
                self.fft_q.put_nowait(buffers["fft"][0])
            except queue.Full:
                pass

        return buffers

    def start(self) -> None:
        if self._run:
            return
        if not self._order:
            self._order = self._topological_order()
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
        while self._run:
            self.run_once()
            time.sleep(self.bs / self.sr)
