# Audio Smoke Validation

This note documents the smoke validation flow for real-time audio and block-size comparison.

## Example Pipelines

- `examples/pipelines/smoke_sine_awgn_256.json`
- `examples/pipelines/smoke_sine_awgn_512.json`
- `examples/pipelines/smoke_sine_awgn_1024.json`

All three pipelines use the same signal chain (`Sine -> AWGN -> FFT`) and only change `block_size`.

## Quick Checks

Dry-run (no audio device):

```bash
simdsp-smoke-audio examples/pipelines/smoke_sine_awgn_256.json --dry-run --seconds 2
simdsp-smoke-audio examples/pipelines/smoke_sine_awgn_512.json --dry-run --seconds 2
simdsp-smoke-audio examples/pipelines/smoke_sine_awgn_1024.json --dry-run --seconds 2
```

Real-time audio (use output node `awgn` to hear the tone):

```bash
simdsp-smoke-audio examples/pipelines/smoke_sine_awgn_256.json --seconds 5 --output-node awgn
simdsp-smoke-audio examples/pipelines/smoke_sine_awgn_512.json --seconds 5 --output-node awgn
simdsp-smoke-audio examples/pipelines/smoke_sine_awgn_1024.json --seconds 5 --output-node awgn
```

## Result Template

## Results

### Dry-run baseline (this environment)

Command outputs:

- `256`: `mode=dry-run blocks=1875 callbacks=1875 xruns=0`
- `512`: `mode=dry-run blocks=938 callbacks=938 xruns=0`
- `1024`: `mode=dry-run blocks=469 callbacks=469 xruns=0`

### Real-time audio

Real-time `xruns` could not be measured in this environment due to missing PortAudio device:

- `sounddevice.PortAudioError: Error querying device -1`

Run the three real-time commands on the target machine and fill the table:

| block_size | mode | callbacks | xruns | notes |
|---|---|---:|---:|---|
| 256 | audio |  |  | pending local device run |
| 512 | audio |  |  | pending local device run |
| 1024 | audio |  |  | pending local device run |
