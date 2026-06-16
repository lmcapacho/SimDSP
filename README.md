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

## Credits

The project is led by Luis Miguel Capacho Valbuena.  
The original SimDSP app was written in C++/Qt by Luis Miguel Capacho Valbuena, Jorge Iván Marín Hurtado, and Alexander López Parrado. (c) 2017 - Universidad del Quindío, Armenia, Quindío, Colombia.

## License

This project remains under the same open source GNU license. See [LICENSE](./LICENSE).
