# Migration Parity Matrix (legacy `master` -> `simdsp2`)

This document tracks behavior parity between the C/C++ implementation and the Python migration.

## Scope

- Legacy references:
  - `src/sdcore/sdsignal.cpp`
  - `src/sdcore/sdaudio.cpp`
  - `src/gui/simdsp.cpp`
- Python references:
  - `simdsp_core/engine.py`
  - `simdsp_blocks/*.py`

## Parity Matrix

| Legacy feature | Expected behavior | Python target block/module | Test id | Status |
|---|---|---|---|---|
| Sine generator | Output sinusoid with configured frequency and amplitude | `simdsp_blocks.sine.Sine` | `test_sine_frequency_peak_matches_configured_frequency` | Implemented |
| FFT view signal peak | FFT magnitude peak should align with dominant sine frequency | `simdsp_blocks.fft.FFTMag` | `test_fft_peak_matches_input_sine_frequency` | Implemented |
| Signal output range clipping | Legacy clamps output to `[-1.0, 1.0]` before ADC/DAC scaling | `simdsp_blocks.sine.Sine` (source clipping) | `test_signal_range_is_clipped_to_minus1_plus1_legacy_behavior` | Implemented |
| AWGN injection + SNR control | Enable AWGN and keep measured SNR near configured target | `simdsp_blocks.awgn.AWGN` | `test_awgn_snr_matches_target_within_tolerance` | Implemented |

## Notes

- `AWGN` now supports deterministic runs using `seed`.
- Current clipping parity is implemented at the `Sine` source block output.
- If full legacy equivalence is required across all blocks, add a dedicated graph-level clip stage.
