from __future__ import annotations


class WorkspacePanel:
    def __init__(self, widgets, pipeline_panel, plot_panel) -> None:
        QWidget = widgets["QWidget"]
        QVBoxLayout = widgets["QVBoxLayout"]

        self.widget = QWidget()
        self.layout = QVBoxLayout(self.widget)
        self.pipeline_panel = pipeline_panel
        self.plot_panel = plot_panel

        self.layout.addWidget(self.pipeline_panel.widget, stretch=1)
        self.layout.addWidget(self.plot_panel.widget, stretch=2)
        self.set_mode("lab")

    def set_mode(self, mode: str) -> None:
        pipeline_visible = mode == "pipeline"
        self.pipeline_panel.widget.setVisible(pipeline_visible)
