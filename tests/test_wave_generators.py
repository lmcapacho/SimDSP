from __future__ import annotations

import numpy as np

from simdsp_blocks.fft import FFTMag
from simdsp_blocks.scope import ScopeTap
from simdsp_blocks.square import Square
from simdsp_blocks.triangle import Triangle
from simdsp_core.engine import Engine


def _peak_frequency(signal_block, freq: float, sr: int = 48_000, bs: int = 2048) -> float:
    eng = Engine(sample_rate=sr, block_size=bs, channels=1)
    eng.add_node('gen', signal_block, inputs=[])
    eng.add_node('fft', FFTMag(), inputs=['gen'])
    eng.add_node('scope', ScopeTap(), inputs=['gen'])
    eng.init()
    buffers = eng.run_once()
    mag = buffers['fft'][0][:, 0]
    freqs = np.fft.rfftfreq(bs, d=1 / sr)
    return freqs[np.argmax(mag)]


def test_square_peak_frequency_matches_requested_tone():
    peak = _peak_frequency(Square(freq=1000, amp=0.8), 1000)
    assert abs(peak - 1000) < (48_000 / 2048) * 2


def test_triangle_peak_frequency_matches_requested_tone():
    peak = _peak_frequency(Triangle(freq=750, amp=0.6), 750)
    assert abs(peak - 750) < (48_000 / 2048) * 2


def test_square_and_triangle_output_stay_in_legacy_signal_range():
    eng = Engine(sample_rate=48_000, block_size=512, channels=1)
    eng.add_node('square', Square(freq=500, amp=0.9, duty=0.25), inputs=[])
    eng.add_node('triangle', Triangle(freq=500, amp=0.9), inputs=[])
    eng.init()

    buffers = eng.run_once()
    square = buffers['square'][0]
    triangle = buffers['triangle'][0]

    assert np.max(square) <= 1.0
    assert np.min(square) >= -1.0
    assert np.max(triangle) <= 1.0
    assert np.min(triangle) >= -1.0
