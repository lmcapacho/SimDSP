from __future__ import annotations

import pytest

from simdsp_native.native_gain import NativeGainBackend


class _NoSymbolLibrary:
    pass


def test_native_gain_backend_reports_library_load_failure(monkeypatch, tmp_path):
    fake_lib = tmp_path / 'libfake.so'
    fake_lib.write_text('placeholder')

    monkeypatch.setattr('simdsp_native.native_gain.build_native_gain', lambda: fake_lib)

    def _raise_load(path):
        raise OSError('dlopen failed')

    monkeypatch.setattr('simdsp_native.native_gain.ctypes.CDLL', _raise_load)

    backend = NativeGainBackend({'gain': 0.5})
    with pytest.raises(RuntimeError, match='Could not load native library'):
        backend.init(48_000, 32, 1)


def test_native_gain_backend_reports_missing_symbol(monkeypatch, tmp_path):
    fake_lib = tmp_path / 'libfake.so'
    fake_lib.write_text('placeholder')

    monkeypatch.setattr('simdsp_native.native_gain.build_native_gain', lambda: fake_lib)
    monkeypatch.setattr('simdsp_native.native_gain.ctypes.CDLL', lambda path: _NoSymbolLibrary())

    backend = NativeGainBackend({'gain': 0.5})
    with pytest.raises(RuntimeError, match="does not export 'simdsp_native_gain_process'"):
        backend.init(48_000, 32, 1)
