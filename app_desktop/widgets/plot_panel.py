from __future__ import annotations


class PlotPanel:
    def __init__(self, widgets, pg) -> None:
        QWidget = widgets["QWidget"]
        QVBoxLayout = widgets["QVBoxLayout"]

        self.widget = QWidget()
        self.layout = QVBoxLayout(self.widget)

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

        self.layout.addWidget(self.scope_plot, stretch=2)
        self.layout.addWidget(self.fft_plot, stretch=2)
