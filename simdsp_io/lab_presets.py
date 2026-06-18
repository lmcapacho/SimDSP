from __future__ import annotations

from copy import deepcopy
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from simdsp_io.pipeline_json import load_pipeline


@dataclass(frozen=True)
class PresetParam:
    key: str
    node_id: str
    param_name: str
    label: str
    description: str = ""


@dataclass(frozen=True)
class LabPreset:
    preset_id: str
    name: str
    category: str
    description: str
    pipeline_file: str
    tags: tuple[str, ...] = field(default_factory=tuple)
    exposed_params: tuple[PresetParam, ...] = field(default_factory=tuple)


_PRESET_DIR = Path(__file__).resolve().parent.parent / "examples" / "pipelines"
_PRESET_LIST = (
    LabPreset(
        preset_id="tone_clean",
        name="Clean Tone",
        category="signal-generator",
        description="Single-tone laboratory preset with scope and FFT views.",
        pipeline_file="tone_clean_lab.json",
        tags=("tone", "generator", "scope", "fft"),
        exposed_params=(
            PresetParam("freq", "gen", "freq", "Frequency", "Generator frequency in Hz."),
            PresetParam("amp", "gen", "amp", "Amplitude", "Generator peak amplitude."),
            PresetParam("gain", "gain", "gain", "Gain", "Linear output gain."),
        ),
    ),
    LabPreset(
        preset_id="tone_awgn",
        name="Tone + AWGN",
        category="channel-noise",
        description="Tone experiment with additive white Gaussian noise.",
        pipeline_file="tone_awgn_lab.json",
        tags=("tone", "awgn", "scope", "fft"),
        exposed_params=(
            PresetParam("freq", "gen", "freq", "Frequency", "Generator frequency in Hz."),
            PresetParam("amp", "gen", "amp", "Amplitude", "Generator peak amplitude."),
            PresetParam("snr_db", "awgn", "snr_db", "SNR", "Noise level in dB."),
        ),
    ),
    LabPreset(
        preset_id="tone_awgn_gain",
        name="Tone + AWGN + Gain",
        category="channel-chain",
        description="Tone experiment with additive noise and post-gain stage.",
        pipeline_file="tone_awgn_gain_lab.json",
        tags=("tone", "awgn", "gain", "scope", "fft"),
        exposed_params=(
            PresetParam("freq", "gen", "freq", "Frequency", "Generator frequency in Hz."),
            PresetParam("amp", "gen", "amp", "Amplitude", "Generator peak amplitude."),
            PresetParam("snr_db", "awgn", "snr_db", "SNR", "Noise level in dB."),
            PresetParam("gain", "gain", "gain", "Gain", "Linear output gain."),
        ),
    ),
    LabPreset(
        preset_id="audio_input_fft",
        name="Audio Input + FFT",
        category="audio-input",
        description="Live microphone/sound-card input with scope and FFT monitors.",
        pipeline_file="audio_input_fft.json",
        tags=("audio", "microphone", "live"),
        exposed_params=(
            PresetParam("device", "mic", "device", "Device", "Optional input device identifier."),
            PresetParam("latency", "mic", "latency", "Latency", "Requested host latency setting."),
        ),
    ),
    LabPreset(
        preset_id="wav_fft",
        name="WAV File + FFT",
        category="file-source",
        description="Replay a WAV file through scope and FFT monitors.",
        pipeline_file="wav_file_source_fft.json",
        tags=("audio", "wav", "file"),
        exposed_params=(
            PresetParam("path", "src", "path", "Path", "Path to the WAV file."),
            PresetParam("loop", "src", "loop", "Loop", "Loop file playback."),
        ),
    ),
    LabPreset(
        preset_id="mat_fft",
        name="MAT File + FFT",
        category="matlab-octave",
        description="Replay a Matlab/Octave signal matrix through scope and FFT monitors.",
        pipeline_file="mat_file_source_fft.json",
        tags=("matlab", "octave", "mat", "file"),
        exposed_params=(
            PresetParam("path", "src", "path", "Path", "Path to the MAT file."),
            PresetParam("variable", "src", "variable", "Variable", "Signal variable inside the MAT file."),
            PresetParam("loop", "src", "loop", "Loop", "Loop signal playback."),
        ),
    ),
)
_PRESETS = {preset.preset_id: preset for preset in _PRESET_LIST}


def list_lab_presets() -> list[LabPreset]:
    return list(_PRESET_LIST)


def get_lab_preset(preset_id: str) -> LabPreset:
    try:
        return _PRESETS[preset_id]
    except KeyError as exc:
        raise ValueError(f"Unknown lab preset '{preset_id}'.") from exc


def preset_pipeline_path(preset_id: str) -> Path:
    preset = get_lab_preset(preset_id)
    return (_PRESET_DIR / preset.pipeline_file).resolve()


def load_lab_preset(preset_id: str, overrides: dict[str, Any] | None = None) -> dict[str, Any]:
    preset = get_lab_preset(preset_id)
    pipeline = load_pipeline(preset_pipeline_path(preset_id))
    if overrides:
        _apply_overrides(pipeline, preset, overrides)
    return pipeline


def _apply_overrides(pipeline: dict[str, Any], preset: LabPreset, overrides: dict[str, Any]) -> None:
    allowed = {param.key: param for param in preset.exposed_params}
    unknown = sorted(set(overrides.keys()) - set(allowed.keys()))
    if unknown:
        raise ValueError(
            f"Preset '{preset.preset_id}' does not expose override(s): {', '.join(unknown)}."
        )

    nodes = {node["id"]: node for node in pipeline.get("nodes", [])}
    for key, value in overrides.items():
        param = allowed[key]
        node = nodes.get(param.node_id)
        if node is None:
            raise ValueError(
                f"Preset '{preset.preset_id}' references missing node '{param.node_id}' for override '{key}'."
            )
        params = deepcopy(node.get("params", {}))
        params[param.param_name] = value
        node["params"] = params
