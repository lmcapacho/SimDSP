from __future__ import annotations

import argparse
import queue
import sys
from typing import Sequence

import numpy as np

from simdsp_blocks.awgn import AWGN
from simdsp_blocks.fft import FFTMag
from simdsp_blocks.scope import ScopeTap
from simdsp_blocks.sine import Sine
from simdsp_core.engine import Engine


def build_default_engine(
    sample_rate: float = 48_000.0,
    block_size: int = 1024,
    channels: int = 1,
) -> Engine:
    eng = Engine(sample_rate=sample_rate, block_size=block_size, channels=channels)
    eng.add_node("gen", Sine(freq=1000, amp=0.8), inputs=[])
    eng.add_node("awgn", AWGN(snr_db=30, seed=1), inputs=["gen"])
    eng.add_node("scope", ScopeTap(), inputs=["awgn"])
    eng.add_node("fft", FFTMag(), inputs=["awgn"])
    eng.init()
    return eng


class SimDSPWindow:
    def __init__(self, sample_rate: float, block_size: int, channels: int) -> None:
        import pyqtgraph as pg
        from PySide6.QtCore import QTimer
        from PySide6.QtWidgets import (
            QHBoxLayout,
            QLabel,
            QMainWindow,
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
        self._QLabel = QLabel
        self._QSpinBox = QSpinBox

        self.window = QMainWindow()
        self.window.setWindowTitle("SimDSP 2.0 - Live Scope/FFT")
        self.window.resize(1200, 750)

        self.engine = build_default_engine(sample_rate, block_size, channels)
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
        controls.addSpacing(20)
        controls.addWidget(QLabel("Sine Freq (Hz):"))
        self.freq_spin = QSpinBox()
        self.freq_spin.setRange(10, int(self.engine.sr // 2))
        self.freq_spin.setValue(1000)
        controls.addWidget(self.freq_spin)
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
        self.freq_spin.valueChanged.connect(self._set_frequency)

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
    )
    win.install_close_hook()
    win.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
