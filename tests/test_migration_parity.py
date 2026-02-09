import numpy as np

from simdsp_blocks.awgn import AWGN
from simdsp_blocks.fft import FFTMag
from simdsp_blocks.sine import Sine


def _run_block(block, sr: int, bs: int, ch: int = 1):
    block.init(sr, bs, ch)
    silence = np.zeros((bs, ch), dtype=np.float32)
    return block.process([silence])[0]


def test_sine_frequency_peak_matches_configured_frequency():
    sr, bs, target_freq = 48_000, 4096, 1_000.0
    y = _run_block(Sine(freq=target_freq, amp=1.0), sr=sr, bs=bs, ch=1)[:, 0]

    spectrum = np.fft.rfft(y)
    freqs = np.fft.rfftfreq(bs, d=1.0 / sr)
    peak_freq = freqs[np.argmax(np.abs(spectrum))]

    assert abs(peak_freq - target_freq) <= (sr / bs) * 2


def test_fft_peak_matches_input_sine_frequency():
    sr, bs, target_freq = 48_000, 2048, 2_000.0
    x = _run_block(Sine(freq=target_freq, amp=1.0), sr=sr, bs=bs, ch=1)

    fft = FFTMag()
    fft.init(sr, bs, 1)
    mag = fft.process([x])[0][:, 0]

    freqs = np.fft.rfftfreq(bs, d=1.0 / sr)
    peak_freq = freqs[np.argmax(mag)]

    assert abs(peak_freq - target_freq) <= (sr / bs) * 2


def test_signal_range_is_clipped_to_minus1_plus1_legacy_behavior():
    y = _run_block(Sine(freq=1_000.0, amp=1.5), sr=48_000, bs=1024, ch=1)
    assert np.max(np.abs(y)) <= 1.0


def test_awgn_snr_matches_target_within_tolerance():
    sr, bs, target_snr_db = 48_000, 8192, 20.0
    x = _run_block(Sine(freq=1_000.0, amp=0.8), sr=sr, bs=bs, ch=1)

    awgn = AWGN(snr_db=target_snr_db, seed=1234)
    awgn.init(sr, bs, 1)
    y = awgn.process([x])[0]

    noise = y - x
    signal_power = float(np.mean(x * x))
    noise_power = float(np.mean(noise * noise))
    measured_snr_db = 10.0 * np.log10(signal_power / noise_power)

    assert abs(measured_snr_db - target_snr_db) <= 1.0
