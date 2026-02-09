import numpy as np
from simdsp_core.engine import Engine
from simdsp_blocks.sine import Sine
from simdsp_blocks.fft import FFTMag
from simdsp_blocks.scope import ScopeTap

def test_sine_peak_frequency():
    """ 
    test the peak frequency detection of the FFT block
    """
    sr, bs = 48000, 2048
    eng = Engine(sample_rate=sr, block_size=bs, channels=1)
    eng.add_node("gen", Sine(freq=1000, amp=1.0), inputs=[])
    eng.add_node("fft", FFTMag(), inputs=["gen"])
    eng.add_node("scope", ScopeTap(), inputs=["gen"])
    eng.init()

    # execute a single iteration of the engine
    # (simulate one processing block)
    buffers = {}
    for nid in ["gen", "fft", "scope"]:
        node = eng.nodes[nid]
        ins = buffers[node.inputs[0]] if node.inputs else [np.zeros((bs,1), np.float32)]
        buffers[nid] = node.block.process(ins)

    mag = buffers["fft"][0][:,0]
    freqs = np.fft.rfftfreq(bs, d=1/sr)
    peak_bin = np.argmax(mag)
    peak_freq = freqs[peak_bin]
    assert abs(peak_freq - 1000) < (sr/bs) * 2  # tolerance ~2 bins
