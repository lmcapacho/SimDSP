from __future__ import annotations


class LeftPanel:
    def __init__(self, widgets, quick_source_types: list[str]) -> None:
        QWidget = widgets["QWidget"]
        QLabel = widgets["QLabel"]
        QListWidget = widgets["QListWidget"]
        QTextEdit = widgets["QTextEdit"]
        QGroupBox = widgets["QGroupBox"]
        QFormLayout = widgets["QFormLayout"]
        QComboBox = widgets["QComboBox"]
        QDoubleSpinBox = widgets["QDoubleSpinBox"]
        QLineEdit = widgets["QLineEdit"]
        QPushButton = widgets["QPushButton"]
        QVBoxLayout = widgets["QVBoxLayout"]

        self.widget = QWidget()
        self.layout = QVBoxLayout(self.widget)
        self.layout.addWidget(QLabel("Block Catalog"))
        self.catalog_list = QListWidget()
        self.layout.addWidget(self.catalog_list, stretch=2)
        self.layout.addWidget(QLabel("Block Inspector"))
        self.inspector = QTextEdit()
        self.inspector.setReadOnly(True)
        self.layout.addWidget(self.inspector, stretch=2)

        self.quick_group = QGroupBox("Quick Experiment")
        quick_layout = QFormLayout(self.quick_group)
        self.quick_source_combo = QComboBox()
        for block_type in quick_source_types:
            self.quick_source_combo.addItem(block_type, block_type)
        self.quick_freq_spin = QDoubleSpinBox()
        self.quick_freq_spin.setDecimals(3)
        self.quick_freq_spin.setRange(0.0, 96000.0)
        self.quick_freq_spin.setSingleStep(10.0)
        self.quick_amp_spin = QDoubleSpinBox()
        self.quick_amp_spin.setDecimals(3)
        self.quick_amp_spin.setRange(0.0, 1.0)
        self.quick_amp_spin.setSingleStep(0.05)
        self.quick_snr_spin = QDoubleSpinBox()
        self.quick_snr_spin.setDecimals(2)
        self.quick_snr_spin.setRange(-20.0, 120.0)
        self.quick_snr_spin.setSingleStep(0.5)
        self.quick_gain_spin = QDoubleSpinBox()
        self.quick_gain_spin.setDecimals(3)
        self.quick_gain_spin.setRange(0.0, 100.0)
        self.quick_gain_spin.setSingleStep(0.1)
        self.quick_path_edit = QLineEdit()
        self.quick_apply_btn = QPushButton("Apply Quick Setup")
        quick_layout.addRow("Source", self.quick_source_combo)
        quick_layout.addRow("Freq (Hz)", self.quick_freq_spin)
        quick_layout.addRow("Amplitude", self.quick_amp_spin)
        quick_layout.addRow("SNR (dB)", self.quick_snr_spin)
        quick_layout.addRow("Gain", self.quick_gain_spin)
        quick_layout.addRow("File/Mat Path", self.quick_path_edit)
        quick_layout.addRow(self.quick_apply_btn)
        self.layout.addWidget(self.quick_group, stretch=0)
