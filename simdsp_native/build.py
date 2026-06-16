from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parent
SRC_DIR = ROOT / "src"
BUILD_DIR = ROOT / "build_artifacts"


def native_gain_library_path() -> Path:
    if sys.platform.startswith("linux"):
        return BUILD_DIR / "libsimdsp_native_gain.so"
    if sys.platform == "darwin":
        return BUILD_DIR / "libsimdsp_native_gain.dylib"
    if sys.platform == "win32":
        return BUILD_DIR / "simdsp_native_gain.dll"
    raise RuntimeError(f"Unsupported platform for native build: {sys.platform}")


def build_native_gain(force: bool = False) -> Path:
    source = SRC_DIR / "native_gain.cpp"
    output = native_gain_library_path()
    output.parent.mkdir(parents=True, exist_ok=True)

    if output.exists() and not force and output.stat().st_mtime >= source.stat().st_mtime:
        return output

    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        raise RuntimeError("No C++ compiler was found. Install g++, clang++, or equivalent.")

    if sys.platform.startswith("linux") or sys.platform == "darwin":
        cmd = [
            compiler,
            "-O3",
            "-std=c++17",
            "-shared",
            "-fPIC",
            str(source),
            "-o",
            str(output),
        ]
    elif sys.platform == "win32":
        raise RuntimeError("Windows native auto-build is not implemented yet for simdsp2.")
    else:
        raise RuntimeError(f"Unsupported platform for native build: {sys.platform}")

    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(
            "Failed to build native gain library.\n"
            f"Command: {' '.join(cmd)}\n"
            f"stdout:\n{result.stdout}\n"
            f"stderr:\n{result.stderr}"
        )

    return output
