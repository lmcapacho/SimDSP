from __future__ import annotations

import numpy as np

from simdsp_core.block_api import Block, BlockSpec, ParamSpec


class AWGN(Block):
    SPEC = BlockSpec(
        type_name="AWGN",
        implementation="python",
        inputs=1,
        outputs=1,
        category="effect",
        display_name="AWGN",
        description="Additive white Gaussian noise.",
        tags=("noise", "channel", "legacy"),
        params=(
            ParamSpec("snr_db", "float", 40.0, "Signal-to-noise ratio in dB.", min_value=-20.0, max_value=120.0, step=0.5, unit="dB"),
            ParamSpec("seed", "int|null", None, "Optional RNG seed."),
        ),
    )

    def __init__(self, snr_db=40.0, seed=None):
        super().__init__(snr_db=snr_db, seed=seed)
        self.snr_db = float(snr_db)
        self.seed = seed

    def init(self, sample_rate, block_size, channels):
        super().init(sample_rate, block_size, channels)
        self._rng = np.random.default_rng(self.seed)

    def process(self, inputs):
        x = inputs[0].astype(np.float32, copy=False)
        signal_power = float(np.mean(x * x))
        if signal_power <= 0.0:
            return [x.copy()]

        noise_power = signal_power / (10.0 ** (self.snr_db / 10.0))
        noise = self._rng.normal(0.0, np.sqrt(noise_power), size=x.shape).astype(np.float32)
        y = np.clip(x + noise, -1.0, 1.0)
        return [y]
