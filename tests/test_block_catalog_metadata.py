from __future__ import annotations

from simdsp_blocks.catalog import DEFAULT_BLOCK_REGISTRY


def test_block_specs_expose_gui_ready_metadata():
    specs = {spec.type_name: spec for spec in DEFAULT_BLOCK_REGISTRY.list_specs()}

    assert specs['Sine'].category == 'generator'
    assert specs['Sine'].display_name == 'Sine'
    assert 'legacy' in specs['Sine'].tags

    assert specs['AudioInput'].category == 'source'
    assert 'microphone' in specs['AudioInput'].tags

    assert specs['MatFileSink'].category == 'sink'
    assert 'export' in specs['MatFileSink'].tags

    assert specs['Gain'].category == 'effect'
    assert 'gain' in specs['Gain'].tags

    assert specs['NativeGain'].category == 'native'
    assert specs['NativeGain'].display_name == 'Native Gain'


def test_catalog_categories_cover_current_block_families():
    categories = {spec.category for spec in DEFAULT_BLOCK_REGISTRY.list_specs()}

    assert {'generator', 'source', 'analysis', 'sink', 'effect', 'user', 'native'}.issubset(categories)
