from __future__ import annotations

from simdsp_blocks.catalog import DEFAULT_BLOCK_REGISTRY


def _param_map(type_name: str):
    spec = next(spec for spec in DEFAULT_BLOCK_REGISTRY.list_specs() if spec.type_name == type_name)
    return {param.name: param for param in spec.params}


def test_generator_params_expose_ranges_units_and_steps():
    sine = _param_map('Sine')
    square = _param_map('Square')

    assert sine['freq'].unit == 'Hz'
    assert sine['freq'].min_value == 0.0
    assert sine['freq'].step == 1.0
    assert sine['amp'].max_value == 1.0

    assert square['duty'].min_value == 0.0
    assert square['duty'].max_value == 1.0
    assert square['duty'].step == 0.01


def test_analysis_and_noise_params_expose_choices_and_units():
    fft = _param_map('FFTMag')
    awgn = _param_map('AWGN')

    assert fft['window'].choices == ('hann', 'rect')
    assert awgn['snr_db'].unit == 'dB'
    assert awgn['snr_db'].max_value == 120.0
