from __future__ import annotations


class ControlBar:
    def __init__(self, widgets) -> None:
        QPushButton = widgets["QPushButton"]
        QLabel = widgets["QLabel"]
        QHBoxLayout = widgets["QHBoxLayout"]

        self.layout = QHBoxLayout()
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
        self.add_node_btn = QPushButton("Add Node")
        self.add_node_btn.setEnabled(False)
        self.delete_node_btn = QPushButton("Delete Node")
        self.delete_node_btn.setEnabled(False)
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
            self.add_node_btn,
            self.delete_node_btn,
        ):
            self.layout.addWidget(widget)
        self.layout.addSpacing(16)
        self.layout.addWidget(self.pipeline_label)
        self.layout.addStretch(1)
