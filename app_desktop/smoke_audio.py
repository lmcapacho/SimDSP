from __future__ import annotations

import argparse
import time
from typing import Sequence

from simdsp_blocks.awgn import AWGN
from simdsp_blocks.fft import FFTMag
from simdsp_blocks.scope import ScopeTap
from simdsp_blocks.sine import Sine
from simdsp_io import AudioEngineStream, load_pipeline, pipeline_to_engine


def _block_factory(block_type: str, params: dict):
    registry = {
        "Sine": Sine,
        "AWGN": AWGN,
        "FFTMag": FFTMag,
        "ScopeTap": ScopeTap,
    }
    if block_type not in registry:
        raise ValueError(f"Unknown block type '{block_type}'.")
    return registry[block_type](**params)


def _teardown_engine(engine) -> None:
    for node in engine.nodes.values():
        try:
            node.block.teardown()
        except Exception:
            pass


def run_smoke(
    pipeline_path: str,
    seconds: float,
    output_node: str | None,
    dry_run: bool,
) -> dict:
    if seconds <= 0:
        raise ValueError("seconds must be > 0")

    pipeline = load_pipeline(pipeline_path)
    engine = pipeline_to_engine(pipeline, _block_factory)

    if dry_run:
        num_blocks = max(1, int(round(seconds * engine.sr / engine.bs)))
        for _ in range(num_blocks):
            engine.run_once()
        _teardown_engine(engine)
        return {"mode": "dry-run", "blocks": num_blocks, "xruns": 0, "callbacks": num_blocks}

    stream = AudioEngineStream(engine, output_node=output_node)
    stream.start()
    try:
        time.sleep(seconds)
        stats = stream.stats()
        return {
            "mode": "audio",
            "blocks": stats.callbacks,
            "callbacks": stats.callbacks,
            "xruns": stats.xruns,
        }
    finally:
        stream.stop()


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Run a SimDSP audio smoke test.")
    parser.add_argument("pipeline", help="Path to pipeline JSON file.")
    parser.add_argument("--seconds", type=float, default=3.0, help="Run duration in seconds.")
    parser.add_argument("--output-node", default=None, help="Node id used as stream output.")
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Execute processing blocks without opening an audio device.",
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = _build_parser()
    args = parser.parse_args(argv)

    result = run_smoke(
        pipeline_path=args.pipeline,
        seconds=args.seconds,
        output_node=args.output_node,
        dry_run=args.dry_run,
    )
    print(
        f"mode={result['mode']} blocks={result['blocks']} "
        f"callbacks={result['callbacks']} xruns={result['xruns']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
