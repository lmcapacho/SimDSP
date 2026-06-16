from app_desktop.live_ui import build_default_engine
from app_desktop.pipeline_tools import default_pipeline, sync_pipeline_from_engine


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
