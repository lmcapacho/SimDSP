from __future__ import annotations

import pytest

from app_desktop.pipeline_tools import lookup_block_spec
from simdsp_io import get_lab_preset, list_lab_presets, load_lab_preset, preset_pipeline_path, validate_pipeline


def test_list_lab_presets_contains_expected_catalog():
    presets = {preset.preset_id: preset for preset in list_lab_presets()}

    assert {"tone_clean", "tone_awgn", "tone_awgn_gain", "audio_input_fft", "wav_fft", "mat_fft"}.issubset(presets)
    assert presets["tone_clean"].category == "signal-generator"
    assert presets["tone_awgn_gain"].exposed_params[-1].key == "gain"



def test_get_lab_preset_rejects_unknown_id():
    with pytest.raises(ValueError, match="Unknown lab preset 'missing'"):
        get_lab_preset("missing")



def test_preset_pipeline_paths_exist():
    for preset in list_lab_presets():
        assert preset_pipeline_path(preset.preset_id).exists()



def test_load_lab_preset_returns_valid_pipeline():
    pipeline = load_lab_preset("tone_awgn_gain")

    validate_pipeline(pipeline, spec_lookup=lookup_block_spec)
    assert [node["id"] for node in pipeline["nodes"]] == ["gen", "awgn", "gain", "scope", "fft"]



def test_load_lab_preset_applies_allowed_overrides():
    pipeline = load_lab_preset("tone_awgn_gain", overrides={"freq": 750.0, "snr_db": 12.5, "gain": 2.0})
    nodes = {node["id"]: node for node in pipeline["nodes"]}

    assert nodes["gen"]["params"]["freq"] == 750.0
    assert nodes["awgn"]["params"]["snr_db"] == 12.5
    assert nodes["gain"]["params"]["gain"] == 2.0



def test_load_lab_preset_rejects_unknown_override_key():
    with pytest.raises(ValueError, match=r"does not expose override\(s\): seed"):
        load_lab_preset("tone_awgn", overrides={"seed": 123})
