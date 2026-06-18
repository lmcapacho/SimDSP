from __future__ import annotations

import json

import numpy as np
import pytest

from app_desktop.pipeline_tools import create_registry
from simdsp_core import Block
from simdsp_plugins import discover_plugin_manifests, load_plugin_manifest, register_plugins_from_dirs


def test_load_plugin_manifest_parses_python_plugin(tmp_path):
    plugin_dir = tmp_path / 'plugins' / 'example'
    plugin_dir.mkdir(parents=True)
    manifest_path = plugin_dir / 'simdsp_plugin.json'
    manifest_path.write_text(json.dumps({
        'plugin_id': 'example.python',
        'name': 'Example Python Plugin',
        'version': '0.1.0',
        'blocks': [
            {
                'kind': 'python-file',
                'path': 'plugin_gain.py',
                'class_name': 'PluginGain',
            }
        ],
    }))

    manifest = load_plugin_manifest(manifest_path)

    assert manifest.plugin_id == 'example.python'
    assert manifest.blocks[0].kind == 'python-file'
    assert manifest.blocks[0].class_name == 'PluginGain'



def test_discover_and_register_python_plugin_block(tmp_path):
    plugin_dir = tmp_path / 'plugins' / 'python_gain'
    plugin_dir.mkdir(parents=True)
    (plugin_dir / 'plugin_gain.py').write_text(
        'from simdsp_core import Block, BlockSpec, ParamSpec\n'
        'import numpy as np\n\n'
        'class PluginGain(Block):\n'
        '    SPEC = BlockSpec(type_name="PluginGain", implementation="python-plugin", inputs=1, outputs=1, category="effect", display_name="Plugin Gain", params=(ParamSpec("gain", "float", 1.0, "Gain"),))\n\n'
        '    def __init__(self, gain=1.0):\n'
        '        super().__init__(gain=gain)\n'
        '        self.gain = float(gain)\n\n'
        '    def process(self, inputs):\n'
        '        x = inputs[0].astype(np.float32, copy=False)\n'
        '        return [np.clip(x * self.gain, -1.0, 1.0)]\n'
    )
    (plugin_dir / 'simdsp_plugin.json').write_text(json.dumps({
        'plugin_id': 'example.python_gain',
        'name': 'Python Gain Plugin',
        'version': '0.1.0',
        'blocks': [
            {
                'kind': 'python-file',
                'path': 'plugin_gain.py',
                'class_name': 'PluginGain',
            }
        ],
    }))

    manifests = discover_plugin_manifests([tmp_path / 'plugins'])
    assert [manifest.plugin_id for manifest in manifests] == ['example.python_gain']

    registry, loaded = register_plugins_from_dirs([tmp_path / 'plugins'])
    assert [manifest.plugin_id for manifest in loaded] == ['example.python_gain']
    block = registry.create('PluginGain', {'gain': 0.25})
    block.init(48_000, 8, 1)
    x = np.ones((8, 1), dtype=np.float32)
    y = block.process([x])[0]

    assert np.allclose(y, 0.25)



def test_register_native_plugin_block(tmp_path):
    plugin_dir = tmp_path / 'plugins' / 'native_gain'
    plugin_dir.mkdir(parents=True)
    (plugin_dir / 'native_gain_backend.py').write_text(
        'import numpy as np\n'
        'from simdsp_core import NativeBlockCapabilities\n\n'
        'class PluginNativeGainBackend:\n'
        '    CAPABILITIES = NativeBlockCapabilities(backend_name="plugin_native_gain", language="c++", supported_platforms=("linux",), requires_compiler=False, auto_build=False)\n\n'
        '    def __init__(self, params):\n'
        '        self.gain = float(params.get("gain", 1.0))\n\n'
        '    def init(self, sample_rate, block_size, channels):\n'
        '        self.bs = int(block_size)\n'
        '        self.ch = int(channels)\n\n'
        '    def process(self, inputs):\n'
        '        x = inputs[0].astype(np.float32, copy=False)\n'
        '        return [np.clip(x * self.gain, -1.0, 1.0)]\n\n'
        '    def teardown(self):\n'
        '        return\n'
    )
    (plugin_dir / 'simdsp_plugin.json').write_text(json.dumps({
        'plugin_id': 'example.native_gain',
        'name': 'Native Gain Plugin',
        'version': '0.1.0',
        'blocks': [
            {
                'kind': 'native-python',
                'path': 'native_gain_backend.py',
                'backend_class': 'PluginNativeGainBackend',
                'spec': {
                    'type_name': 'PluginNativeGain',
                    'implementation': 'native-plugin',
                    'inputs': 1,
                    'outputs': 1,
                    'category': 'native',
                    'display_name': 'Plugin Native Gain',
                    'description': 'Native plugin gain.',
                    'tags': ['native', 'gain'],
                    'params': [
                        {'name': 'gain', 'param_type': 'float', 'default': 1.0, 'description': 'Gain'}
                    ]
                }
            }
        ],
    }))

    registry = create_registry(plugin_dirs=[tmp_path / 'plugins'])
    block = registry.create('PluginNativeGain', {'gain': 0.5})
    block.init(48_000, 8, 1)
    x = np.ones((8, 1), dtype=np.float32)
    y = block.process([x])[0]

    assert np.allclose(y, 0.5)
    assert block.capabilities is not None
    assert block.capabilities.backend_name == 'plugin_native_gain'



def test_plugin_manifest_rejects_unsupported_kind(tmp_path):
    plugin_dir = tmp_path / 'plugins' / 'bad'
    plugin_dir.mkdir(parents=True)
    manifest_path = plugin_dir / 'simdsp_plugin.json'
    manifest_path.write_text(json.dumps({
        'plugin_id': 'example.bad',
        'name': 'Bad Plugin',
        'version': '0.1.0',
        'blocks': [{'kind': 'zip-file', 'path': 'x.zip'}],
    }))

    with pytest.raises(ValueError, match="unsupported block kind 'zip-file'"):
        load_plugin_manifest(manifest_path)
