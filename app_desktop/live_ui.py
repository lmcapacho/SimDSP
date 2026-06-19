from __future__ import annotations

import argparse
import queue
import sys
from copy import deepcopy
from pathlib import Path
from typing import Sequence

import numpy as np

from app_desktop.pipeline_tools import apply_quick_experiment, default_pipeline, engine_from_pipeline, sync_pipeline_from_engine
from app_desktop.widgets import ControlBar, LeftPanel, PipelineEditorPanel, PlotPanel
from simdsp_blocks.catalog import list_block_specs
from simdsp_core import find_node_by_role
from simdsp_core.engine import Engine
from simdsp_io import load_pipeline, save_pipeline



def build_default_engine(
    sample_rate: float = 48_000.0,
    block_size: int = 1024,
    channels: int = 1,
) -> Engine:
    return engine_from_pipeline(default_pipeline(sample_rate, block_size, channels))


class SimDSPWindow:
    def __init__(
        self,
        sample_rate: float,
        block_size: int,
        channels: int,
        pipeline_path: str | None = None,
    ) -> None:
        import pyqtgraph as pg
        from PySide6.QtCore import QTimer
        from PySide6.QtWidgets import (
            QAbstractItemView,
            QCheckBox,
            QComboBox,
            QFileDialog,
            QDoubleSpinBox,
            QFormLayout,
            QHBoxLayout,
            QLabel,
            QLineEdit,
            QListWidgetItem,
            QMainWindow,
            QMessageBox,
            QPushButton,
            QSplitter,
            QVBoxLayout,
            QWidget,
        )

        self._pg = pg
        self._QTimer = QTimer
        self._QFileDialog = QFileDialog
        self._QMessageBox = QMessageBox
        self._QListWidgetItem = QListWidgetItem
        self._QLineEdit = QLineEdit
        self._QCheckBox = QCheckBox
        self._QComboBox = QComboBox
        self._QDoubleSpinBox = QDoubleSpinBox

        widget_factory = {
            "QAbstractItemView": QAbstractItemView,
            "QCheckBox": QCheckBox,
            "QComboBox": QComboBox,
            "QDoubleSpinBox": QDoubleSpinBox,
            "QFormLayout": QFormLayout,
            "QHBoxLayout": QHBoxLayout,
            "QLabel": QLabel,
            "QLineEdit": QLineEdit,
            "QListWidget": __import__("PySide6.QtWidgets", fromlist=["QListWidget"]).QListWidget,
            "QPushButton": QPushButton,
            "QGroupBox": __import__("PySide6.QtWidgets", fromlist=["QGroupBox"]).QGroupBox,
            "QScrollArea": __import__("PySide6.QtWidgets", fromlist=["QScrollArea"]).QScrollArea,
            "QTextEdit": __import__("PySide6.QtWidgets", fromlist=["QTextEdit"]).QTextEdit,
            "QVBoxLayout": QVBoxLayout,
            "QWidget": QWidget,
        }

        self.window = QMainWindow()
        self.window.resize(1560, 900)

        self.current_pipeline_path: Path | None = None
        self.current_pipeline = default_pipeline(sample_rate, block_size, channels)
        if pipeline_path:
            self.current_pipeline = load_pipeline(pipeline_path)
            self.current_pipeline_path = Path(pipeline_path)

        self.engine = engine_from_pipeline(self.current_pipeline, self.current_pipeline_path)
        self._running = False
        self._block_specs = list_block_specs()
        self._spec_by_type = {spec.type_name: spec for spec in self._block_specs}
        self._selected_spec = None
        self._selected_node_id: str | None = None
        self._selected_edge_index: int | None = None
        self._param_editors: dict[str, object] = {}
        self._current_param_spec_by_name: dict[str, object] = {}
        self._quick_source_types = ["Sine", "Square", "Triangle", "AudioInput", "WavFileSource", "MatFileSource"]

        root = QWidget()
        self.window.setCentralWidget(root)
        outer = QVBoxLayout(root)

        self.control_bar = ControlBar(widget_factory)
        outer.addLayout(self.control_bar.layout)
        self.start_btn = self.control_bar.start_btn
        self.stop_btn = self.control_bar.stop_btn
        self.open_btn = self.control_bar.open_btn
        self.save_btn = self.control_bar.save_btn
        self.save_as_btn = self.control_bar.save_as_btn
        self.default_btn = self.control_bar.default_btn
        self.apply_btn = self.control_bar.apply_btn
        self.change_type_btn = self.control_bar.change_type_btn
        self.add_node_btn = self.control_bar.add_node_btn
        self.delete_node_btn = self.control_bar.delete_node_btn
        self.pipeline_label = self.control_bar.pipeline_label

        splitter = QSplitter()
        outer.addWidget(splitter, stretch=1)

        self.left_panel = LeftPanel(widget_factory, self._quick_source_types)
        splitter.addWidget(self.left_panel.widget)
        self.catalog_list = self.left_panel.catalog_list
        self.inspector = self.left_panel.inspector
        self.quick_group = self.left_panel.quick_group
        self.quick_source_combo = self.left_panel.quick_source_combo
        self.quick_freq_spin = self.left_panel.quick_freq_spin
        self.quick_amp_spin = self.left_panel.quick_amp_spin
        self.quick_snr_spin = self.left_panel.quick_snr_spin
        self.quick_gain_spin = self.left_panel.quick_gain_spin
        self.quick_path_edit = self.left_panel.quick_path_edit
        self.quick_apply_btn = self.left_panel.quick_apply_btn

        center_panel = QWidget()
        center_layout = QVBoxLayout(center_panel)

        self.pipeline_panel = PipelineEditorPanel(widget_factory)
        center_layout.addWidget(self.pipeline_panel.widget, stretch=1)
        self.node_list = self.pipeline_panel.node_list
        self.edge_list = self.pipeline_panel.edge_list
        self.add_edge_btn = self.pipeline_panel.add_edge_btn
        self.delete_edge_btn = self.pipeline_panel.delete_edge_btn
        self.param_group = self.pipeline_panel.param_group
        self.param_title = self.pipeline_panel.param_title
        self.param_form = self.pipeline_panel.param_form

        self.plot_panel = PlotPanel(widget_factory, pg)
        center_layout.addWidget(self.plot_panel.widget, stretch=2)
        self.scope_plot = self.plot_panel.scope_plot
        self.scope_curve = self.plot_panel.scope_curve
        self.fft_plot = self.plot_panel.fft_plot
        self.fft_curve = self.plot_panel.fft_curve

        splitter.addWidget(center_panel)
        splitter.setStretchFactor(0, 0)
        splitter.setStretchFactor(1, 1)

        self.timer = QTimer(self.window)
        self.timer.setInterval(30)
        self.timer.timeout.connect(self._poll_engine_queues)

        self.start_btn.clicked.connect(self.start_engine)
        self.stop_btn.clicked.connect(self.stop_engine)
        self.open_btn.clicked.connect(self.open_pipeline_dialog)
        self.save_btn.clicked.connect(self.save_pipeline_file)
        self.save_as_btn.clicked.connect(self.save_pipeline_as_dialog)
        self.default_btn.clicked.connect(self.load_default_pipeline)
        self.apply_btn.clicked.connect(self.apply_selected_node_params)
        self.change_type_btn.clicked.connect(self.change_selected_node_type)
        self.add_node_btn.clicked.connect(self.add_selected_block_as_node)
        self.delete_node_btn.clicked.connect(self.delete_selected_node)
        self.add_edge_btn.clicked.connect(self.add_edge_from_selected_nodes)
        self.delete_edge_btn.clicked.connect(self.delete_selected_edge)
        self.quick_apply_btn.clicked.connect(self.apply_quick_experiment_controls)
        self.quick_source_combo.currentIndexChanged.connect(self._update_quick_control_state)
        self.catalog_list.currentRowChanged.connect(self._on_catalog_selected)
        self.node_list.currentRowChanged.connect(self._on_node_selected)
        self.edge_list.currentRowChanged.connect(self._on_edge_selected)

        self._populate_catalog()
        self._populate_node_list()
        self._populate_edge_list()
        self._sync_quick_controls()
        self._update_quick_control_state()
        self._update_window_state()
        self._select_first_catalog_item()

    def start_engine(self) -> None:
        if self._running:
            return
        self.engine.start()
        self.timer.start()
        self._running = True
        self.start_btn.setEnabled(False)
        self.stop_btn.setEnabled(True)

    def stop_engine(self) -> None:
        if not self._running:
            return
        self.timer.stop()
        self.engine.stop()
        self._running = False
        self.start_btn.setEnabled(True)
        self.stop_btn.setEnabled(False)

    def _poll_engine_queues(self) -> None:
        self._update_scope_plot()
        self._update_fft_plot()

    def _update_scope_plot(self) -> None:
        latest = None
        while True:
            try:
                latest = self.engine.scope_q.get_nowait()
            except queue.Empty:
                break
        if latest is None:
            return
        y = np.asarray(latest[:, 0], dtype=np.float32)
        self.scope_curve.setData(np.arange(y.size), y)
        self.scope_plot.setYRange(-1.1, 1.1, padding=0.0)

    def _update_fft_plot(self) -> None:
        latest = None
        while True:
            try:
                latest = self.engine.fft_q.get_nowait()
            except queue.Empty:
                break
        if latest is None:
            return
        mag = np.asarray(latest[:, 0], dtype=np.float32)
        freqs = np.fft.rfftfreq(self.engine.bs, d=1.0 / self.engine.sr)
        self.fft_curve.setData(freqs, mag)
        self.fft_plot.setXRange(0, self.engine.sr / 2, padding=0.0)

    def close(self) -> None:
        self.stop_engine()
        self.window.close()

    def show(self) -> None:
        self.window.show()

    def load_default_pipeline(self) -> None:
        self._replace_pipeline(default_pipeline(self.engine.sr, self.engine.bs, self.engine.ch), path=None)

    def open_pipeline_dialog(self) -> None:
        selected, _ = self._QFileDialog.getOpenFileName(
            self.window,
            "Open Pipeline",
            str(self.current_pipeline_path.parent if self.current_pipeline_path else Path.cwd()),
            "JSON Files (*.json);;All Files (*)",
        )
        if not selected:
            return
        self._replace_pipeline(load_pipeline(selected), path=selected)

    def save_pipeline_file(self) -> None:
        if self.current_pipeline_path is None:
            self.save_pipeline_as_dialog()
            return
        save_pipeline(self.current_pipeline_path, self._snapshot_pipeline())
        self._update_window_state()

    def save_pipeline_as_dialog(self) -> None:
        selected, _ = self._QFileDialog.getSaveFileName(
            self.window,
            "Save Pipeline As",
            str(self.current_pipeline_path or Path.cwd() / "pipeline.json"),
            "JSON Files (*.json);;All Files (*)",
        )
        if not selected:
            return
        self.current_pipeline_path = Path(selected)
        save_pipeline(self.current_pipeline_path, self._snapshot_pipeline())
        self._update_window_state()

    def _snapshot_pipeline(self) -> dict:
        self.current_pipeline = sync_pipeline_from_engine(self.current_pipeline, self.engine)
        return deepcopy(self.current_pipeline)

    def _replace_pipeline(self, pipeline: dict, path: str | Path | None) -> None:
        was_running = self._running
        selected_node = self._selected_node_id
        selected_edge = self._selected_edge_index
        self.stop_engine()
        try:
            self.engine = engine_from_pipeline(pipeline, path)
        except Exception as exc:
            self._QMessageBox.critical(self.window, "Pipeline Error", str(exc))
            if was_running:
                self.start_engine()
            return

        self.current_pipeline = deepcopy(pipeline)
        self.current_pipeline_path = Path(path) if path else None
        self._populate_node_list(selected_node)
        self._populate_edge_list(selected_edge)
        self._sync_quick_controls()
        self._update_quick_control_state()
        self._update_window_state()
        if was_running:
            self.start_engine()

    def _update_window_state(self) -> None:
        name = self.current_pipeline_path.name if self.current_pipeline_path else "unsaved default pipeline"
        self.pipeline_label.setText(name)
        self.window.setWindowTitle(f"SimDSP 2.0 - Live Scope/FFT - {name}")

    def _sync_quick_controls(self) -> None:
        source_node = find_node_by_role(self.current_pipeline, "source")
        noise_node = find_node_by_role(self.current_pipeline, "noise")
        gain_node = find_node_by_role(self.current_pipeline, "gain")
        if source_node is not None:
            idx = self.quick_source_combo.findData(source_node["type"])
            if idx >= 0:
                self.quick_source_combo.setCurrentIndex(idx)
            params = source_node.get("params", {})
            self.quick_freq_spin.setValue(float(params.get("freq", 1000.0) or 0.0))
            self.quick_amp_spin.setValue(float(params.get("amp", 0.7) or 0.0))
            self.quick_path_edit.setText(str(params.get("path", "") or ""))
        if noise_node is not None:
            self.quick_snr_spin.setValue(float(noise_node.get("params", {}).get("snr_db", 30.0)))
        if gain_node is not None:
            self.quick_gain_spin.setValue(float(gain_node.get("params", {}).get("gain", 1.0)))

    def _update_quick_control_state(self) -> None:
        source_type = self.quick_source_combo.currentData()
        generator_source = source_type in {"Sine", "Square", "Triangle"}
        file_source = source_type in {"WavFileSource", "MatFileSource"}
        self.quick_freq_spin.setEnabled(generator_source)
        self.quick_amp_spin.setEnabled(generator_source)
        self.quick_path_edit.setEnabled(file_source)

    def apply_quick_experiment_controls(self) -> None:
        source_type = self.quick_source_combo.currentData()
        source_params = {}
        if source_type in {"Sine", "Square", "Triangle"}:
            source_params["freq"] = float(self.quick_freq_spin.value())
            source_params["amp"] = float(self.quick_amp_spin.value())
        if source_type == "WavFileSource":
            source_params["path"] = self.quick_path_edit.text().strip()
        if source_type == "MatFileSource":
            source_params["path"] = self.quick_path_edit.text().strip()
        updated_pipeline = apply_quick_experiment(
            self.current_pipeline,
            source_type=source_type,
            source_params=source_params,
            snr_db=float(self.quick_snr_spin.value()),
            gain=float(self.quick_gain_spin.value()),
        )
        self._replace_pipeline(updated_pipeline, self.current_pipeline_path)

    def _populate_catalog(self) -> None:
        self.catalog_list.clear()
        for spec in self._block_specs:
            label = f"[{spec.category}] {spec.display_name or spec.type_name}"
            item = self._QListWidgetItem(label)
            item.setData(256, spec.type_name)
            self.catalog_list.addItem(item)

    def _populate_node_list(self, selected_node_id: str | None = None) -> None:
        self.node_list.clear()
        selected_row = 0
        for idx, node in enumerate(self.current_pipeline.get("nodes", [])):
            label = f"{node['id']} [{node['type']}]"
            item = self._QListWidgetItem(label)
            item.setData(256, node['id'])
            self.node_list.addItem(item)
            if selected_node_id is not None and node['id'] == selected_node_id:
                selected_row = idx
        if self.node_list.count() > 0:
            self.node_list.setCurrentRow(min(selected_row, self.node_list.count() - 1))
        else:
            self._clear_param_form()
            self._selected_node_id = None
            self.delete_node_btn.setEnabled(False)
            self.change_type_btn.setEnabled(False)
            self.add_edge_btn.setEnabled(False)

    def _populate_edge_list(self, selected_edge_index: int | None = None) -> None:
        self.edge_list.clear()
        edges = self.current_pipeline.get('edges', [])
        for idx, edge in enumerate(edges):
            item = self._QListWidgetItem(f"{edge['from']} -> {edge['to']}")
            item.setData(256, idx)
            self.edge_list.addItem(item)
        if self.edge_list.count() > 0 and selected_edge_index is not None and selected_edge_index < self.edge_list.count():
            self.edge_list.setCurrentRow(selected_edge_index)
        else:
            self._selected_edge_index = None
            self.delete_edge_btn.setEnabled(False)

    def _select_first_catalog_item(self) -> None:
        if self.catalog_list.count() > 0:
            self.catalog_list.setCurrentRow(0)

    def _on_catalog_selected(self, row: int) -> None:
        if row < 0 or row >= len(self._block_specs):
            self._selected_spec = None
            self.inspector.clear()
            self.change_type_btn.setEnabled(False)
            self.add_node_btn.setEnabled(False)
            return
        self._selected_spec = self._block_specs[row]
        self.inspector.setPlainText(self._format_block_spec(self._selected_spec))
        self.change_type_btn.setEnabled(self._selected_node_id is not None)
        self.add_node_btn.setEnabled(True)

    def _on_node_selected(self, row: int) -> None:
        if row < 0 or row >= len(self.current_pipeline.get("nodes", [])):
            self._selected_node_id = None
            self.change_type_btn.setEnabled(False)
            self.delete_node_btn.setEnabled(False)
            self.add_edge_btn.setEnabled(False)
            self._clear_param_form()
            return
        node = self.current_pipeline["nodes"][row]
        self._selected_node_id = node['id']
        self.change_type_btn.setEnabled(self._selected_spec is not None)
        self.delete_node_btn.setEnabled(True)
        self.add_edge_btn.setEnabled(self.node_list.count() >= 2)
        spec = self._spec_by_type.get(node['type'])
        self._build_param_form(node, spec)

    def _on_edge_selected(self, row: int) -> None:
        if row < 0 or row >= len(self.current_pipeline.get('edges', [])):
            self._selected_edge_index = None
            self.delete_edge_btn.setEnabled(False)
            return
        self._selected_edge_index = row
        self.delete_edge_btn.setEnabled(True)

    def _build_param_form(self, node: dict, spec) -> None:
        self._clear_param_form()
        if spec is None:
            self.param_title.setText(f"No metadata available for node '{node['id']}'.")
            self.apply_btn.setEnabled(False)
            return

        self.param_title.setText(f"Editing node: {node['id']} [{spec.display_name or spec.type_name}]")
        params = node.setdefault('params', {})
        self._current_param_spec_by_name = {param.name: param for param in spec.params}

        for param in spec.params:
            editor = self._create_param_editor(param, params.get(param.name, param.default))
            self._param_editors[param.name] = editor
            label = param.name if not param.unit else f"{param.name} ({param.unit})"
            self.param_form.addRow(label, editor)

        self.apply_btn.setEnabled(bool(spec.params))
        self.change_type_btn.setEnabled(self._selected_spec is not None)
        self.delete_node_btn.setEnabled(self._selected_node_id is not None)

    def _create_param_editor(self, param, value):
        if param.choices:
            editor = self._QComboBox()
            for choice in param.choices:
                editor.addItem(str(choice), choice)
            idx = editor.findData(value)
            if idx >= 0:
                editor.setCurrentIndex(idx)
            return editor

        if param.param_type == 'bool':
            editor = self._QCheckBox()
            editor.setChecked(bool(value))
            return editor

        if param.param_type in {'float', 'int'}:
            editor = self._QDoubleSpinBox()
            editor.setDecimals(6 if param.param_type == 'float' else 0)
            editor.setMinimum(param.min_value if param.min_value is not None else -1e12)
            editor.setMaximum(param.max_value if param.max_value is not None else 1e12)
            editor.setSingleStep(param.step if param.step is not None else (1.0 if param.param_type == 'int' else 0.1))
            editor.setValue(float(value))
            return editor

        editor = self._QLineEdit(str(value) if value is not None else '')
        return editor

    def add_selected_block_as_node(self) -> None:
        if self._selected_spec is None:
            return
        updated_pipeline = deepcopy(self.current_pipeline)
        node_id = self._make_unique_node_id(self._selected_spec.type_name)
        updated_pipeline.setdefault('nodes', []).append({
            'id': node_id,
            'type': self._selected_spec.type_name,
            'params': {param.name: deepcopy(param.default) for param in self._selected_spec.params},
        })
        self._replace_pipeline(updated_pipeline, self.current_pipeline_path)

    def delete_selected_node(self) -> None:
        if self._selected_node_id is None:
            return
        node_ids = {node['id'] for node in self.current_pipeline.get('nodes', [])}
        if len(node_ids) <= 1:
            self._QMessageBox.warning(self.window, 'Delete Node', 'The pipeline must keep at least one node.')
            return

        updated_pipeline = deepcopy(self.current_pipeline)
        updated_pipeline['nodes'] = [node for node in updated_pipeline.get('nodes', []) if node['id'] != self._selected_node_id]
        updated_pipeline['edges'] = [
            edge for edge in updated_pipeline.get('edges', [])
            if edge.get('from') != self._selected_node_id and edge.get('to') != self._selected_node_id
        ]
        self._selected_node_id = None
        self._replace_pipeline(updated_pipeline, self.current_pipeline_path)

    def add_edge_from_selected_nodes(self) -> None:
        selected_rows = self.node_list.selectedIndexes()
        if len(selected_rows) != 2:
            self._QMessageBox.warning(self.window, 'Add Edge', 'Select exactly two nodes: source first, then destination.')
            return
        from_node = self.current_pipeline['nodes'][selected_rows[0].row()]['id']
        to_node = self.current_pipeline['nodes'][selected_rows[1].row()]['id']
        if from_node == to_node:
            self._QMessageBox.warning(self.window, 'Add Edge', 'Cannot connect a node to itself.')
            return
        updated_pipeline = deepcopy(self.current_pipeline)
        edges = updated_pipeline.setdefault('edges', [])
        edge = {'from': from_node, 'to': to_node}
        if edge not in edges:
            edges.append(edge)
        self._replace_pipeline(updated_pipeline, self.current_pipeline_path)

    def delete_selected_edge(self) -> None:
        if self._selected_edge_index is None:
            return
        updated_pipeline = deepcopy(self.current_pipeline)
        edges = updated_pipeline.get('edges', [])
        if 0 <= self._selected_edge_index < len(edges):
            del edges[self._selected_edge_index]
        self._selected_edge_index = None
        self._replace_pipeline(updated_pipeline, self.current_pipeline_path)

    def _make_unique_node_id(self, type_name: str) -> str:
        base = type_name[:1].lower() + type_name[1:]
        existing = {node['id'] for node in self.current_pipeline.get('nodes', [])}
        if base not in existing:
            return base
        suffix = 2
        while f'{base}{suffix}' in existing:
            suffix += 1
        return f'{base}{suffix}'

    def change_selected_node_type(self) -> None:
        if self._selected_node_id is None or self._selected_spec is None:
            return
        row = next((idx for idx, node in enumerate(self.current_pipeline.get('nodes', [])) if node['id'] == self._selected_node_id), None)
        if row is None:
            return

        updated_pipeline = deepcopy(self.current_pipeline)
        node = deepcopy(updated_pipeline['nodes'][row])
        node['type'] = self._selected_spec.type_name
        node['params'] = {param.name: deepcopy(param.default) for param in self._selected_spec.params}
        updated_pipeline['nodes'][row] = node
        self._replace_pipeline(updated_pipeline, self.current_pipeline_path)

    def apply_selected_node_params(self) -> None:
        if self._selected_node_id is None:
            return
        row = next((idx for idx, node in enumerate(self.current_pipeline.get('nodes', [])) if node['id'] == self._selected_node_id), None)
        if row is None:
            return

        node = deepcopy(self.current_pipeline['nodes'][row])
        params = node.setdefault('params', {})
        for name, editor in self._param_editors.items():
            params[name] = self._read_editor_value(name, editor)
        updated_pipeline = deepcopy(self.current_pipeline)
        updated_pipeline['nodes'][row] = node
        self._replace_pipeline(updated_pipeline, self.current_pipeline_path)

    def _read_editor_value(self, name: str, editor):
        param = self._current_param_spec_by_name[name]
        if param.choices:
            return editor.currentData()
        if param.param_type == 'bool':
            return bool(editor.isChecked())
        if param.param_type == 'int':
            return int(round(editor.value()))
        if param.param_type == 'float':
            return float(editor.value())
        text = editor.text()
        if param.param_type.endswith('|null') and text == '':
            return None
        return text

    def _clear_param_form(self) -> None:
        while self.param_form.count():
            item = self.param_form.takeAt(0)
            widget = item.widget()
            if widget is not None:
                widget.deleteLater()
        self._param_editors = {}
        self._current_param_spec_by_name = {}
        self.param_title.setText("Select a node from the current pipeline to edit its parameters.")
        self.apply_btn.setEnabled(False)
        self.change_type_btn.setEnabled(False)
        self.delete_node_btn.setEnabled(False)

    def _format_block_spec(self, spec) -> str:
        lines = [
            f"Name: {spec.display_name or spec.type_name}",
            f"Type: {spec.type_name}",
            f"Category: {spec.category}",
            f"Implementation: {spec.implementation}",
            f"Inputs: {spec.inputs}",
            f"Outputs: {spec.outputs}",
            f"Tags: {', '.join(spec.tags) if spec.tags else '-'}",
            "",
            spec.description or "",
            "",
            "Parameters:",
        ]
        if not spec.params:
            lines.append("  - None")
        else:
            for param in spec.params:
                details = [f"type={param.param_type}", f"default={param.default}"]
                if param.unit:
                    details.append(f"unit={param.unit}")
                if param.min_value is not None or param.max_value is not None:
                    details.append(f"range={param.min_value}..{param.max_value}")
                if param.step is not None:
                    details.append(f"step={param.step}")
                if param.choices:
                    details.append(f"choices={list(param.choices)}")
                lines.append(f"  - {param.name}: {param.description}")
                lines.append(f"    {' | '.join(details)}")
        return "\n".join(lines)

    def install_close_hook(self) -> None:
        original_close = self.window.closeEvent

        def _close_event(event):
            self.stop_engine()
            original_close(event)

        self.window.closeEvent = _close_event



def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Run SimDSP desktop live scope/FFT app.")
    parser.add_argument("--sample-rate", type=float, default=48_000.0)
    parser.add_argument("--block-size", type=int, default=1024)
    parser.add_argument("--channels", type=int, default=1)
    parser.add_argument("--pipeline", default=None, help="Path to a pipeline JSON file.")
    return parser



def main(argv: Sequence[str] | None = None) -> int:
    import pyqtgraph as pg
    from PySide6.QtWidgets import QApplication

    parser = _build_parser()
    args = parser.parse_args(argv)

    app = QApplication(sys.argv if argv is None else ["simdsp-desktop", *argv])
    pg.setConfigOptions(antialias=True)
    win = SimDSPWindow(
        sample_rate=args.sample_rate,
        block_size=args.block_size,
        channels=args.channels,
        pipeline_path=args.pipeline,
    )
    win.install_close_hook()
    win.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
