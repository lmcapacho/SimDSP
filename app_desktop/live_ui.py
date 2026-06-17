from __future__ import annotations

import argparse
import queue
import sys
from copy import deepcopy
from pathlib import Path
from typing import Sequence

import numpy as np

from app_desktop.pipeline_tools import default_pipeline, engine_from_pipeline, list_block_specs, sync_pipeline_from_engine
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
        from PySide6.QtCore import QTimer, Qt
        from PySide6.QtWidgets import (
            QCheckBox,
            QComboBox,
            QFileDialog,
            QDoubleSpinBox,
            QFormLayout,
            QGroupBox,
            QHBoxLayout,
            QLabel,
            QLineEdit,
            QListWidget,
            QListWidgetItem,
            QMainWindow,
            QMessageBox,
            QPushButton,
            QScrollArea,
            QSplitter,
            QTextEdit,
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
        self._Qt = Qt

        self.window = QMainWindow()
        self.window.resize(1480, 860)

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
        self._param_editors: dict[str, object] = {}
        self._current_param_spec_by_name: dict[str, object] = {}

        root = QWidget()
        self.window.setCentralWidget(root)
        outer = QVBoxLayout(root)

        controls = QHBoxLayout()
        self.start_btn = QPushButton("Start")
        self.stop_btn = QPushButton("Stop")
        self.stop_btn.setEnabled(False)
        self.open_btn = QPushButton("Open")
        self.save_btn = QPushButton("Save")
        self.save_as_btn = QPushButton("Save As")
        self.default_btn = QPushButton("Default")
        self.apply_btn = QPushButton("Apply Params")
        self.apply_btn.setEnabled(False)
        self.change_type_btn = QPushButton("Use Selected Block Type")
        self.change_type_btn.setEnabled(False)
        self.pipeline_label = QLabel()

        for widget in (
            self.start_btn,
            self.stop_btn,
            self.open_btn,
            self.save_btn,
            self.save_as_btn,
            self.default_btn,
            self.apply_btn,
            self.change_type_btn,
        ):
            controls.addWidget(widget)
        controls.addSpacing(16)
        controls.addWidget(self.pipeline_label)
        controls.addStretch(1)
        outer.addLayout(controls)

        splitter = QSplitter()
        outer.addWidget(splitter, stretch=1)

        left_panel = QWidget()
        left_layout = QVBoxLayout(left_panel)
        left_layout.addWidget(QLabel("Block Catalog"))
        self.catalog_list = QListWidget()
        left_layout.addWidget(self.catalog_list, stretch=2)
        left_layout.addWidget(QLabel("Block Inspector"))
        self.inspector = QTextEdit()
        self.inspector.setReadOnly(True)
        left_layout.addWidget(self.inspector, stretch=2)
        left_layout.addWidget(QLabel("Pipeline Nodes"))
        self.node_list = QListWidget()
        left_layout.addWidget(self.node_list, stretch=2)
        splitter.addWidget(left_panel)

        center_panel = QWidget()
        center_layout = QVBoxLayout(center_panel)

        self.param_group = QGroupBox("Block Parameters")
        param_layout = QVBoxLayout(self.param_group)
        self.param_title = QLabel("Select a node from the current pipeline to edit its parameters.")
        param_layout.addWidget(self.param_title)
        self.param_scroll = QScrollArea()
        self.param_scroll.setWidgetResizable(True)
        self.param_container = QWidget()
        self.param_form = QFormLayout(self.param_container)
        self.param_scroll.setWidget(self.param_container)
        param_layout.addWidget(self.param_scroll)
        center_layout.addWidget(self.param_group, stretch=1)

        self.scope_plot = pg.PlotWidget(title="Scope (Time Domain)")
        self.scope_plot.setLabel("left", "Amplitude")
        self.scope_plot.setLabel("bottom", "Sample")
        self.scope_plot.showGrid(x=True, y=True, alpha=0.2)
        self.scope_curve = self.scope_plot.plot(pen=pg.mkPen(color="#2D7FF9", width=2))

        self.fft_plot = pg.PlotWidget(title="FFT Magnitude (dB)")
        self.fft_plot.setLabel("left", "Magnitude (dB)")
        self.fft_plot.setLabel("bottom", "Frequency (Hz)")
        self.fft_plot.showGrid(x=True, y=True, alpha=0.2)
        self.fft_curve = self.fft_plot.plot(pen=pg.mkPen(color="#E95D0F", width=2))

        center_layout.addWidget(self.scope_plot, stretch=2)
        center_layout.addWidget(self.fft_plot, stretch=2)
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
        self.catalog_list.currentRowChanged.connect(self._on_catalog_selected)
        self.node_list.currentRowChanged.connect(self._on_node_selected)

        self._populate_catalog()
        self._populate_node_list()
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
        self._update_window_state()
        if was_running:
            self.start_engine()

    def _update_window_state(self) -> None:
        name = self.current_pipeline_path.name if self.current_pipeline_path else "unsaved default pipeline"
        self.pipeline_label.setText(name)
        self.window.setWindowTitle(f"SimDSP 2.0 - Live Scope/FFT - {name}")

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

    def _select_first_catalog_item(self) -> None:
        if self.catalog_list.count() > 0:
            self.catalog_list.setCurrentRow(0)

    def _on_catalog_selected(self, row: int) -> None:
        if row < 0 or row >= len(self._block_specs):
            self._selected_spec = None
            self.inspector.clear()
            self.change_type_btn.setEnabled(False)
            return
        self._selected_spec = self._block_specs[row]
        self.inspector.setPlainText(self._format_block_spec(self._selected_spec))
        self.change_type_btn.setEnabled(self._selected_node_id is not None)

    def _on_node_selected(self, row: int) -> None:
        if row < 0 or row >= len(self.current_pipeline.get("nodes", [])):
            self._selected_node_id = None
            self.change_type_btn.setEnabled(False)
            self._clear_param_form()
            return
        node = self.current_pipeline["nodes"][row]
        self._selected_node_id = node['id']
        self.change_type_btn.setEnabled(self._selected_spec is not None)
        spec = self._spec_by_type.get(node['type'])
        self._build_param_form(node, spec)

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
