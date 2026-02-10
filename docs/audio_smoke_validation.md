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

Record the outputs and compare `xruns`:

| block_size | mode | callbacks | xruns | notes |
|---|---|---:|---:|---|
| 256 | audio |  |  |  |
| 512 | audio |  |  |  |
| 1024 | audio |  |  |  |
