from __future__ import annotations

from simdsp_core.block_api import BlockSpec

SOURCE_ROLE = "source"
NOISE_ROLE = "noise"
GAIN_ROLE = "gain"
SCOPE_ROLE = "scope"
FFT_ROLE = "fft"
RESERVED_ROLES = {SOURCE_ROLE, NOISE_ROLE, GAIN_ROLE, SCOPE_ROLE, FFT_ROLE}


def find_node_by_role(pipeline: dict, role: str) -> dict | None:
    for node in pipeline.get("nodes", []):
        if node.get("role") == role:
            return node
    return None



def role_matches_spec(role: str, spec: BlockSpec) -> bool:
    if role == SOURCE_ROLE:
        return spec.inputs == 0 and spec.category in {"generator", "source"}
    if role == NOISE_ROLE:
        return spec.inputs == 1 and "noise" in spec.tags
    if role == GAIN_ROLE:
        return spec.inputs == 1 and "gain" in spec.tags
    if role == SCOPE_ROLE:
        return spec.type_name == "ScopeTap" or (spec.inputs == 1 and "scope" in spec.tags)
    if role == FFT_ROLE:
        return spec.type_name == "FFTMag" or (spec.inputs == 1 and "spectrum" in spec.tags)
    return True
