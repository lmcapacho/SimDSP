from __future__ import annotations

import argparse
import queue
import sys
from copy import deepcopy
from pathlib import Path
from typing import Sequence

import numpy as np

from app_desktop.pipeline_tools import default_pipeline, engine_from_pipeline, sync_pipeline_from_engine
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
            QFileDialog,
            QHBoxLayout,
            QLabel,
            QMainWindow,
            QMessageBox,
            QPushButton,
            QSpinBox,
            QVBoxLayout,
            QWidget,
        )

        self._pg = pg
        self._QMainWindow = QMainWindow
        self._QTimer = QTimer
        self._QWidget = QWidget
        self._QVBoxLayout = QVBoxLayout
        self._QHBoxLayout = QHBoxLayout
        self._QPushButton = QPushButton
        self._QFileDialog = QFileDialog
        self._QLabel = QLabel
        self._QMessageBox = QMessageBox
        self._QSpinBox = QSpinBox

        self.window = QMainWindow()
        self.window.resize(1200, 750)

        self.current_pipeline_path: Path | None = None
        self.current_pipeline = default_pipeline(sample_rate, block_size, channels)
        if pipeline_path:
            self.current_pipeline = load_pipeline(pipeline_path)
            self.current_pipeline_path = Path(pipeline_path)

        self.engine = engine_from_pipeline(self.current_pipeline, self.current_pipeline_path)
        self._running = False

        root = QWidget()
        self.window.setCentralWidget(root)
        main_layout = QVBoxLayout(root)

        controls = QHBoxLayout()
        self.start_btn = QPushButton("Start")
        self.stop_btn = QPushButton("Stop")
        self.stop_btn.setEnabled(False)
        controls.addWidget(self.start_btn)
        controls.addWidget(self.stop_btn)
        controls.addSpacing(12)
        self.open_btn = QPushButton("Open")
        self.save_btn = QPushButton("Save")
        self.save_as_btn = QPushButton("Save As")
        self.default_btn = QPushButton("Default")
        controls.addWidget(self.open_btn)
        controls.addWidget(self.save_btn)
        controls.addWidget(self.save_as_btn)
        controls.addWidget(self.default_btn)
        controls.addSpacing(20)
        controls.addWidget(QLabel("Sine Freq (Hz):"))
        self.freq_spin = QSpinBox()
        self.freq_spin.setRange(10, int(self.engine.sr // 2))
        controls.addWidget(self.freq_spin)
        self.pipeline_label = QLabel()
        controls.addSpacing(20)
        controls.addWidget(self.pipeline_label)
        controls.addStretch(1)
        main_layout.addLayout(controls)

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

        main_layout.addWidget(self.scope_plot, stretch=1)
        main_layout.addWidget(self.fft_plot, stretch=1)

        self.timer = QTimer(self.window)
        self.timer.setInterval(30)
        self.timer.timeout.connect(self._poll_engine_queues)

        self.start_btn.clicked.connect(self.start_engine)
        self.stop_btn.clicked.connect(self.stop_engine)
        self.open_btn.clicked.connect(self.open_pipeline_dialog)
        self.save_btn.clicked.connect(self.save_pipeline_file)
        self.save_as_btn.clicked.connect(self.save_pipeline_as_dialog)
        self.default_btn.clicked.connect(self.load_default_pipeline)
        self.freq_spin.valueChanged.connect(self._set_frequency)

        self._sync_controls_from_engine()
        self._update_window_state()

    def _set_frequency(self, value: int) -> None:
        gen = self.engine.nodes.get("gen")
        if gen is not None and hasattr(gen.block, "freq"):
            gen.block.freq = float(value)

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
        self._replace_pipeline(
            default_pipeline(self.engine.sr, self.engine.bs, self.engine.ch),
            path=None,
        )

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
        self._sync_controls_from_engine()
        self._update_window_state()
        if was_running:
            self.start_engine()

    def _sync_controls_from_engine(self) -> None:
        gen = self.engine.nodes.get("gen")
        has_freq = gen is not None and hasattr(gen.block, "freq")
        self.freq_spin.blockSignals(True)
        self.freq_spin.setEnabled(has_freq)
        self.freq_spin.setRange(10, max(10, int(self.engine.sr // 2)))
        if has_freq:
            self.freq_spin.setValue(int(round(float(gen.block.freq))))
        else:
            self.freq_spin.setValue(10)
        self.freq_spin.blockSignals(False)

    def _update_window_state(self) -> None:
        name = self.current_pipeline_path.name if self.current_pipeline_path else "unsaved default pipeline"
        self.pipeline_label.setText(name)
        self.window.setWindowTitle(f"SimDSP 2.0 - Live Scope/FFT - {name}")

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
