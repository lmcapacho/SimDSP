from app_desktop.live_ui import build_default_engine


def test_build_default_engine_graph_contains_scope_and_fft_nodes():
    eng = build_default_engine(sample_rate=48_000, block_size=512, channels=1)

    assert "gen" in eng.nodes
    assert "awgn" in eng.nodes
    assert "scope" in eng.nodes
    assert "fft" in eng.nodes
    assert eng.nodes["scope"].inputs == ["awgn"]
    assert eng.nodes["fft"].inputs == ["awgn"]
