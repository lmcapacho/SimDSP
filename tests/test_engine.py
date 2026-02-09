import numpy as np
import pytest

from simdsp_core.engine import Engine
from simdsp_blocks.awgn import AWGN
from simdsp_blocks.sine import Sine
from simdsp_blocks.fft import FFTMag
from simdsp_blocks.scope import ScopeTap


def test_sine_peak_frequency():
    sr, bs = 48000, 2048
    eng = Engine(sample_rate=sr, block_size=bs, channels=1)
    eng.add_node("gen", Sine(freq=1000, amp=1.0), inputs=[])
    eng.add_node("fft", FFTMag(), inputs=["gen"])
    eng.add_node("scope", ScopeTap(), inputs=["gen"])
    eng.init()

    buffers = eng.run_once()

    mag = buffers["fft"][0][:, 0]
    freqs = np.fft.rfftfreq(bs, d=1/sr)
    peak_bin = np.argmax(mag)
    peak_freq = freqs[peak_bin]
    assert abs(peak_freq - 1000) < (sr/bs) * 2  # tolerance ~2 bins


def test_engine_topological_order_works_even_if_nodes_added_out_of_order():
    sr, bs = 48_000, 2048
    eng = Engine(sample_rate=sr, block_size=bs, channels=1)
    eng.add_node("fft", FFTMag(), inputs=["awgn"])
    eng.add_node("awgn", AWGN(snr_db=25, seed=7), inputs=["gen"])
    eng.add_node("gen", Sine(freq=1500, amp=0.8), inputs=[])

    eng.init()
    buffers = eng.run_once()

    mag = buffers["fft"][0][:, 0]
    freqs = np.fft.rfftfreq(bs, d=1/sr)
    peak_freq = freqs[np.argmax(mag)]
    assert abs(peak_freq - 1500) < (sr/bs) * 2


def test_engine_rejects_cycles():
    eng = Engine(sample_rate=48_000, block_size=256, channels=1)
    eng.add_node("a", ScopeTap(), inputs=["b"])
    eng.add_node("b", ScopeTap(), inputs=["a"])

    with pytest.raises(ValueError, match="Cycle detected"):
        eng.init()


def test_engine_rejects_missing_upstream_nodes():
    eng = Engine(sample_rate=48_000, block_size=256, channels=1)
    eng.add_node("fft", FFTMag(), inputs=["does_not_exist"])

    with pytest.raises(ValueError, match="missing input node"):
        eng.init()
