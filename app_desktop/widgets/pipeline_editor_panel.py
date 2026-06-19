from __future__ import annotations


class PipelineEditorPanel:
    def __init__(self, widgets) -> None:
        QWidget = widgets["QWidget"]
        QLabel = widgets["QLabel"]
        QListWidget = widgets["QListWidget"]
        QPushButton = widgets["QPushButton"]
        QGroupBox = widgets["QGroupBox"]
        QHBoxLayout = widgets["QHBoxLayout"]
        QVBoxLayout = widgets["QVBoxLayout"]
        QScrollArea = widgets["QScrollArea"]
        QFormLayout = widgets["QFormLayout"]
        QAbstractItemView = widgets["QAbstractItemView"]

        self.widget = QWidget()
        self.layout = QVBoxLayout(self.widget)

        self.graph_group = QGroupBox("Pipeline Structure")
        graph_layout = QHBoxLayout(self.graph_group)

        nodes_panel = QWidget()
        nodes_layout = QVBoxLayout(nodes_panel)
        nodes_layout.addWidget(QLabel("Pipeline Nodes"))
        self.node_list = QListWidget()
        self.node_list.setSelectionMode(QAbstractItemView.ExtendedSelection)
        nodes_layout.addWidget(self.node_list, stretch=2)
        graph_layout.addWidget(nodes_panel)

        edges_panel = QWidget()
        edges_layout = QVBoxLayout(edges_panel)
        edges_layout.addWidget(QLabel("Pipeline Edges"))
        self.edge_list = QListWidget()
        edges_layout.addWidget(self.edge_list, stretch=2)
        edge_btns = QHBoxLayout()
        self.add_edge_btn = QPushButton("Add Edge")
        self.add_edge_btn.setEnabled(False)
        self.delete_edge_btn = QPushButton("Delete Edge")
        self.delete_edge_btn.setEnabled(False)
        edge_btns.addWidget(self.add_edge_btn)
        edge_btns.addWidget(self.delete_edge_btn)
        edges_layout.addLayout(edge_btns)
        graph_layout.addWidget(edges_panel)

        self.layout.addWidget(self.graph_group, stretch=1)

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
        self.layout.addWidget(self.param_group, stretch=1)
