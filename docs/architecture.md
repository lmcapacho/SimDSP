# Architecture

This document describes the current SimDSP 2.0 architecture on branch `simdsp2`.

## Goals

The current design optimizes for four things:

- a DSP engine independent from the GUI
- reproducible experiment definitions through JSON pipelines
- extension through Python blocks and native-backed blocks
- recovery of the original SimDSP workflow through presets and fast experiment setup

## High-Level Layers

### `simdsp_core`

Core DSP contracts and runtime primitives.

Main responsibilities:

- block contracts (`Block`, `BlockSpec`, `ParamSpec`)
- native backend capabilities (`NativeBlockCapabilities`)
- pipeline role semantics (`SOURCE_ROLE`, `GAIN_ROLE`, etc.)
- runtime graph execution (`Engine`)
- block registration (`BlockRegistry`)

Key files:

- `simdsp_core/block_api.py`
- `simdsp_core/pipeline_roles.py`
- `simdsp_core/engine.py`
- `simdsp_core/registry.py`

### `simdsp_blocks`

Builtin DSP blocks and block catalog helpers.

Main responsibilities:

- builtin source, effect, analysis, sink, and user-loader blocks
- default block registry for the application
- block metadata lookup for validation and GUI

Key files:

- `simdsp_blocks/catalog.py`
- `simdsp_blocks/registry.py`
- `simdsp_blocks/sine.py`
- `simdsp_blocks/awgn.py`
- `simdsp_blocks/gain.py`
- `simdsp_blocks/python_module.py`

### `simdsp_io`

Pipeline persistence, validation, engine construction, presets, and audio stream bridge.

Main responsibilities:

- load/save JSON pipelines
- validate pipeline structure, roles, and parameters
- build an `Engine` from a validated pipeline
- expose reproducible lab presets
- bridge real-time audio I/O

Key files:

- `simdsp_io/pipeline_json.py`
- `simdsp_io/pipeline_validation.py`
- `simdsp_io/pipeline_engine.py`
- `simdsp_io/lab_presets.py`
- `simdsp_io/audio_stream.py`

### `simdsp_native`

Native-backed block support.

Main responsibilities:

- native adapter contract
- backend capability metadata
- native build helpers
- concrete native-backed blocks

Key files:

- `simdsp_native/adapter.py`
- `simdsp_native/build.py`
- `simdsp_native/native_gain.py`
- `simdsp_native/src/native_gain.cpp`

### `simdsp_plugins`

Plugin discovery and registration.

Main responsibilities:

- parse plugin manifests
- discover plugin folders
- register Python plugin blocks
- register native-style plugin backends through `NativeBlockAdapter`

Key files:

- `simdsp_plugins/manifest.py`
- `simdsp_plugins/registry.py`
- `simdsp_plugins/loader.py`

### `app_desktop`

Desktop-facing integration layer and current GUI prototype.

Main responsibilities:

- desktop live UI
- smoke audio CLI
- desktop-oriented pipeline helpers

Key files:

- `app_desktop/live_ui.py`
- `app_desktop/pipeline_tools.py`
- `app_desktop/smoke_audio.py`

## Execution Flow

### 1. Block registration

The application starts from a block registry.

Default path:

- builtin blocks are registered from `simdsp_blocks.registry`
- builtin catalog helpers are exposed from `simdsp_blocks.catalog`
- optional plugin directories can extend the registry through `create_registry(plugin_dirs=[...])`

### 2. Pipeline loading

A pipeline is loaded from JSON or produced by a preset loader.

Sources:

- `simdsp_io.load_pipeline(...)`
- `simdsp_io.load_lab_preset(...)`
- `app_desktop.pipeline_tools.default_pipeline(...)`

### 3. Pipeline validation

Validation happens before runtime construction.

`simdsp_io.pipeline_validation.validate_pipeline(...)` checks:

- required top-level fields
- valid graph edges
- duplicate node ids
- duplicate edges
- self-loops
- block input count compatibility
- reserved role compatibility
- parameter names
- parameter types
- parameter ranges
- parameter choices

### 4. Engine construction

`simdsp_io.pipeline_engine.pipeline_to_engine(...)` converts a validated pipeline into an `Engine`.

Process:

- derive node inputs from `edges`
- create runtime block instances through the registry factory
- add nodes to the engine
- initialize blocks with sample rate, block size, and channels

### 5. Runtime execution

`simdsp_core.engine.Engine` runs the DAG in topological order.

For each node:

- upstream outputs are gathered as inputs
- block `process(...)` is called
- outputs are stored by node id

The engine also publishes optional visualization taps for `scope` and `fft` queues used by the desktop UI.

## Pipeline Model

Pipelines are JSON DAGs with this shape:

- `schema_version`
- `sample_rate`
- `block_size`
- `channels`
- `nodes`
- `edges`

A node contains:

- `id`
- `type`
- optional `role`
- optional `params`

Current reserved roles:

- `source`
- `noise`
- `gain`
- `scope`
- `fft`

Reserved roles are used for:

- semantic validation
- preset workflows
- fast experiment mutation
- future GUI behavior

## Parameter Contract

Block parameters are described by `ParamSpec`.

Current metadata:

- `name`
- `param_type`
- `default`
- `description`
- `min_value`
- `max_value`
- `step`
- `unit`
- `choices`

Validation supports:

- `bool`
- `int`
- `float`
- `str`
- union types such as `str|int|null`

Blocks can opt into extra parameters with:

- `BlockSpec.accepts_extra_params = True`

This is currently required for `PythonModuleBlock` because user-defined blocks may receive arbitrary extra parameters.

## Preset Layer

`simdsp_io.lab_presets` provides reproducible laboratory presets above raw pipelines.

A preset contains:

- preset id
- name
- category
- description
- source pipeline file
- tags
- explicitly exposed override parameters

This layer is important because it restores the original SimDSP intent:

- quick switching between common DSP experiments
- reproducibility through JSON
- no need to manually rebuild the graph for standard scenarios

## Plugin Layer

Plugins are discovered through `simdsp_plugin.json` manifests.

Supported plugin block kinds:

- `python-file`
- `native-python`

### `python-file`

Expected contract:

- points to a Python file
- names a block class
- the class already defines `SPEC`

### `native-python`

Expected contract:

- points to a Python module containing a backend class
- names a backend class
- defines explicit `BlockSpec` metadata in the manifest
- backend is wrapped by `NativeBlockAdapter`

This design keeps plugin growth outside the core repository while staying compatible with the same registry and pipeline validation flow.

## Native Block Contract

Native-backed blocks are modeled as Python adapters over backend classes.

Contract pieces:

- `BlockSpec` describes the public block shape
- `NativeBlockCapabilities` describes backend/runtime properties
- `NativeBlockAdapter` enforces runtime output validation

Current capability metadata includes:

- backend name
- language
- supported platforms
- whether a compiler is required
- whether auto-build is supported
- shared library name
- operational notes

The first concrete implementation is `NativeGain`.

## Current Architectural Direction

The architecture is intentionally moving toward:

- stable core contracts first
- plugin and preset growth second
- richer GUI later

This order is deliberate.

The current GUI is a functional integration surface, not the final product architecture.
The main objective is to keep the model below the GUI strong enough that future interface redesign does not require rewriting the DSP foundation.

## Current Heavy Files

The heaviest module still is:

- `app_desktop/live_ui.py`

That file remains a reasonable next refactor target after the current documentation phase because most core/backend responsibilities have already been split into dedicated modules.
