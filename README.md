# SimDSP 2.0

**Sim**ulation software for **D**igital **S**ignal **P**rocessing

SimDSP 2.0 is a complete update of the original C++/Qt application, now redesigned in **Python** with:

- **DSP Core** decoupled from the UI, with blocks implemented in Python or C/C++ (via pybind11).
- **Desktop UI** built on **PySide6 + pyqtgraph**, enabling smooth real-time visualizations.
- **Audio I/O** support through PortAudio (sounddevice).
- **JSON pipelines** for saving and loading reproducible projects.

## Current Status

🚧 SimDSP 2.0 is under active development on the `simdsp2` branch.  
The original Qt/C++ version is preserved in:
- [`/legacy`](./legacy) folder  

## Roadmap

1. Python DSP core with unified block API.
2. PySide6 + pyqtgraph UI (time, FFT, spectrogram).
3. Audio I/O integration (sounddevice).
4. JSON pipelines (save/load projects).
5. Additional DSP blocks (FIR, IIR, noise, meters).
6. Native C++/pybind11 integration.
7. Multiplatform packaging (AppImage, EXE, DMG).

## Installation

> Instructions will be provided once the first Python release is available.  
For now, clone the repo and switch to the `simdsp2` branch.

## Audio Smoke Defaults (current)

- Current recommended `block_size` default for smoke tests: `512` (balanced latency/CPU tradeoff).
- Validate on target hardware with:
  - `examples/pipelines/smoke_sine_awgn_256.json`
  - `examples/pipelines/smoke_sine_awgn_512.json`
  - `examples/pipelines/smoke_sine_awgn_1024.json`
- See `docs/audio_smoke_validation.md` for command sequence and result table.

## Desktop Live UI (scope + FFT)

Run a minimal real-time desktop UI:

```bash
simdsp-desktop --sample-rate 48000 --block-size 1024 --channels 1
```

Open a saved JSON pipeline directly:

```bash
simdsp-desktop --pipeline examples/pipelines/smoke_sine_awgn_512.json
```

Alternative invocation:

```bash
python -m app_desktop.live_ui --sample-rate 48000 --block-size 1024 --channels 1
```

Current desktop UI capabilities:

- Open an existing pipeline JSON file.
- Save the current pipeline state back to JSON.
- Save As to create new reproducible pipeline files.
- Reload the built-in default scope/FFT demo pipeline.

The block catalog now carries metadata for the next GUI stage: category, display name, tags, parameter descriptions, numeric ranges, units, steps, and discrete choices.

## User-Defined Python Blocks

SimDSP 2.0 can now load DSP blocks written by the user in Python, without modifying the core application.

Example pipeline:

```bash
simdsp-desktop --pipeline examples/pipelines/python_user_gain.json
```

The example block source lives in `examples/user_blocks/gain_block.py`.
This keeps the original SimDSP spirit: the user can write algorithm code and run it inside the DSP engine.

## Source Blocks

SimDSP 2.0 now includes source blocks closer to the original workflow:

- `Sine`, `Square`, `Triangle`: built-in generated test signals.
- `WavFileSource`: read PCM WAV files as streaming test inputs.
- `AudioInput`: capture microphone or sound-card input as a live source.

Example pipelines:

```bash
simdsp-desktop --pipeline examples/pipelines/wav_file_source_fft.json
simdsp-desktop --pipeline examples/pipelines/audio_input_fft.json
```

This keeps audio-style experiments separate from the future Matlab/Octave-style data import path.

## Matlab/Octave Data Blocks

SimDSP 2.0 now also supports Matlab/Octave-style data exchange:

- `MatFileSource`: import a signal matrix from a `.mat` file.
- `MatFileSink`: capture a signal stream and export it to a `.mat` file.

Example pipelines:

```bash
simdsp-desktop --pipeline examples/pipelines/mat_file_source_fft.json
```

The capture example `examples/pipelines/mat_roundtrip_capture.json` is intended for scripted runs and regression checks.

## Plugin Discovery

SimDSP 2.0 now has a formal plugin discovery/loading layer based on `simdsp_plugin.json` manifests.

Supported plugin block kinds:

- `python-file`: loads a Python block class that already defines `SPEC`
- `native-python`: loads a backend class under the native adapter contract plus explicit `BlockSpec` metadata in the manifest

Example plugin folders:

- `examples/plugins/python_gain_plugin`
- `examples/plugins/native_gain_plugin`

Programmatic loading example:

```python
from app_desktop.pipeline_tools import create_registry

registry = create_registry(plugin_dirs=["examples/plugins"])
block = registry.create("PluginGain", {"gain": 0.5})
```

This keeps third-party growth out of the core repository and prepares the path for future plugin-aware tooling.

## Laboratory Presets

SimDSP 2.0 now includes a reproducible laboratory preset catalog on top of JSON pipelines.

Available preset ids:

- `tone_clean`
- `tone_awgn`
- `tone_awgn_gain`
- `audio_input_fft`
- `wav_fft`
- `mat_fft`

The preset API lives in `simdsp_io.lab_presets` and supports:

- listing presets
- loading a preset pipeline
- overriding only explicitly exposed experiment parameters

Example:

```python
from simdsp_io import load_lab_preset

pipeline = load_lab_preset("tone_awgn_gain", overrides={
    "freq": 750.0,
    "snr_db": 18.0,
    "gain": 1.5,
})
```

This is the base layer for recovering the original SimDSP workflow of switching quickly between common DSP experiments without rebuilding the pipeline by hand.

## Native C++ Block

SimDSP 2.0 now includes a first real native C++ block wired into the same pipeline contract:

- `NativeGain`: gain block executed inside a compiled shared library loaded from Python.

Example pipeline:

```bash
simdsp-desktop --pipeline examples/pipelines/native_gain_demo.json
```

The native source lives in `simdsp_native/src/native_gain.cpp` and is auto-built on supported systems the first time it is used.

## Credits

The project is led by Luis Miguel Capacho Valbuena.  
The original SimDSP app was written in C++/Qt by Luis Miguel Capacho Valbuena, Jorge Iván Marín Hurtado, and Alexander López Parrado. (c) 2017 - Universidad del Quindío, Armenia, Quindío, Colombia.

## License

This project remains under the same open source GNU license. See [LICENSE](./LICENSE).
