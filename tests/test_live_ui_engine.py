from app_desktop.live_ui import SimDSPWindow, build_default_engine
from app_desktop.pipeline_tools import default_pipeline, list_block_specs, sync_pipeline_from_engine


class _FakeCombo:
    def __init__(self, data):
        self._data = data

    def currentData(self):
        return self._data


class _FakeCheck:
    def __init__(self, checked):
        self._checked = checked

    def isChecked(self):
        return self._checked


class _FakeSpin:
    def __init__(self, value):
        self._value = value

    def value(self):
        return self._value


class _FakeLine:
    def __init__(self, text):
        self._text = text

    def text(self):
        return self._text


def test_build_default_engine_graph_contains_scope_and_fft_nodes():
    eng = build_default_engine(sample_rate=48_000, block_size=512, channels=1)

    assert "gen" in eng.nodes
    assert "awgn" in eng.nodes
    assert "scope" in eng.nodes
    assert "fft" in eng.nodes
    assert eng.nodes["scope"].inputs == ["awgn"]
    assert eng.nodes["fft"].inputs == ["awgn"]


def test_default_pipeline_contains_expected_nodes():
    pipeline = default_pipeline(sample_rate=44_100, block_size=256, channels=1)

    assert pipeline["sample_rate"] == 44_100.0
    assert pipeline["block_size"] == 256
    assert pipeline["channels"] == 1
    assert [node["id"] for node in pipeline["nodes"]] == ["gen", "awgn", "scope", "fft"]


def test_sync_pipeline_from_engine_updates_runtime_params():
    pipeline = default_pipeline(sample_rate=48_000, block_size=512, channels=1)
    eng = build_default_engine(sample_rate=48_000, block_size=512, channels=1)
    eng.nodes["gen"].block.freq = 1750.0

    snapshot = sync_pipeline_from_engine(pipeline, eng)

    gen = next(node for node in snapshot["nodes"] if node["id"] == "gen")
    assert gen["params"]["freq"] == 1750.0


def test_block_catalog_exposes_metadata_for_gui():
    specs = list_block_specs()
    names = {spec.type_name for spec in specs}

    assert "Sine" in names
    assert "AudioInput" in names
    assert "NativeGain" in names


def test_block_spec_formatter_contains_category_and_params():
    spec = next(spec for spec in list_block_specs() if spec.type_name == "Sine")
    formatted = SimDSPWindow._format_block_spec(None, spec)

    assert "Category: generator" in formatted
    assert "freq" in formatted
    assert "unit=Hz" in formatted


def test_read_editor_value_respects_param_types():
    spec = next(spec for spec in list_block_specs() if spec.type_name == "Square")
    params = {param.name: param for param in spec.params}
    fake = type('FakeWindow', (), {'_current_param_spec_by_name': params})()

    assert SimDSPWindow._read_editor_value(fake, 'duty', _FakeSpin(0.25)) == 0.25
    assert SimDSPWindow._read_editor_value(fake, 'freq', _FakeSpin(1234.0)) == 1234.0

    fft = next(spec for spec in list_block_specs() if spec.type_name == "FFTMag")
    fake._current_param_spec_by_name = {param.name: param for param in fft.params}
    assert SimDSPWindow._read_editor_value(fake, 'window', _FakeCombo('rect')) == 'rect'

    audio = next(spec for spec in list_block_specs() if spec.type_name == "AudioInput")
    fake._current_param_spec_by_name = {param.name: param for param in audio.params}
    assert SimDSPWindow._read_editor_value(fake, 'device', _FakeLine('hw:0')) == 'hw:0'


def test_change_selected_node_type_replaces_params_with_spec_defaults():
    specs = {spec.type_name: spec for spec in list_block_specs()}
    fake = type('FakeWindow', (), {})()
    fake._selected_node_id = 'gen'
    fake._selected_spec = specs['Square']
    fake.current_pipeline_path = None
    fake.current_pipeline = default_pipeline(sample_rate=48_000, block_size=512, channels=1)
    captured = {}

    def _replace_pipeline(pipeline, path):
        captured['pipeline'] = pipeline
        captured['path'] = path

    fake._replace_pipeline = _replace_pipeline

    SimDSPWindow.change_selected_node_type(fake)

    node = next(node for node in captured['pipeline']['nodes'] if node['id'] == 'gen')
    assert node['type'] == 'Square'
    assert node['params']['freq'] == 1000.0
    assert node['params']['duty'] == 0.5


def test_make_unique_node_id_appends_suffix_when_needed():
    fake = type('FakeWindow', (), {})()
    fake.current_pipeline = {'nodes': [{'id': 'sine'}, {'id': 'sine2'}]}

    assert SimDSPWindow._make_unique_node_id(fake, 'Sine') == 'sine3'


def test_add_selected_block_as_node_uses_spec_defaults():
    specs = {spec.type_name: spec for spec in list_block_specs()}
    fake = type('FakeWindow', (), {})()
    fake._selected_spec = specs['Triangle']
    fake.current_pipeline_path = None
    fake.current_pipeline = default_pipeline(sample_rate=48_000, block_size=512, channels=1)
    fake._make_unique_node_id = lambda type_name: 'triangle2'
    captured = {}

    def _replace_pipeline(pipeline, path):
        captured['pipeline'] = pipeline

    fake._replace_pipeline = _replace_pipeline

    SimDSPWindow.add_selected_block_as_node(fake)

    node = next(node for node in captured['pipeline']['nodes'] if node['id'] == 'triangle2')
    assert node['type'] == 'Triangle'
    assert node['params']['amp'] == 0.7


def test_delete_selected_node_removes_node_and_related_edges():
    fake = type('FakeWindow', (), {})()
    fake._selected_node_id = 'awgn'
    fake.window = None
    fake._QMessageBox = type('Msg', (), {'warning': staticmethod(lambda *args, **kwargs: None)})
    fake.current_pipeline_path = None
    fake.current_pipeline = default_pipeline(sample_rate=48_000, block_size=512, channels=1)
    captured = {}

    def _replace_pipeline(pipeline, path):
        captured['pipeline'] = pipeline

    fake._replace_pipeline = _replace_pipeline

    SimDSPWindow.delete_selected_node(fake)

    ids = {node['id'] for node in captured['pipeline']['nodes']}
    assert 'awgn' not in ids
    assert all(edge['from'] != 'awgn' and edge['to'] != 'awgn' for edge in captured['pipeline']['edges'])
