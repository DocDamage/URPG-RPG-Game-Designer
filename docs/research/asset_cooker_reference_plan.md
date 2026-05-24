# AssetCooker Reference Plan for URPG

Status: Planning reference
Source repo: `DocDamage/AssetCooker`
Decision: Study and reimplement useful concepts natively in URPG. Do not vendor AssetCooker wholesale.

## Executive Decision

AssetCooker can improve URPG's asset iteration speed if its core ideas are adapted into URPG's own offline/editor asset pipeline.

It should not be imported directly into the URPG runtime or editor app as-is.

## Why It Is Useful

AssetCooker contains several concepts URPG needs:

- rule-based asset cooking
- input filters and wildcard matching
- command-line cooking rules
- file-copy cooking rules
- dependency-file support
- dirty-state detection
- missing/outdated output tracking
- rule-version invalidation
- priority-based cooking queues
- no-UI command operation
- cooking logs and diagnostics

These features map directly to URPG problems:

- avoiding full asset rescans on editor startup
- avoiding recooking unchanged PNGs, atlases, previews, audio, and manifests
- producing deterministic processed asset outputs
- speeding up editor preview generation
- making package/export prep more reliable
- surfacing asset errors as structured diagnostics instead of loose console text

## Why It Should Not Be Vendored

Do not copy AssetCooker wholesale into URPG.

Reasons:

- The app layer is Win32, D3D11, and ImGui based.
- The implementation uses its own Bedrock framework types and platform helpers.
- The license is MPL-2.0, which allows use but carries source-file obligations that should be handled deliberately.
- URPG needs a cross-platform offline/tooling pipeline, not a second standalone editor runtime embedded into the product.

## Recommended URPG Target

Create a native URPG asset-cooking lane under tools and engine/editor diagnostics.

Suggested ownership:

```text
tools/assets/cooker/
engine/core/assets/
editor/assets/
docs/assets/
tests/tools/
tests/unit/
```

Suggested files:

```text
tools/assets/cooker/asset_cooker.py
tools/assets/cooker/cook_rules.schema.json
tools/assets/cooker/default_cook_rules.json
tools/assets/cooker/cook_manifest.py
tools/assets/cooker/cook_cache.py
tools/assets/tests/test_asset_cooker.py
content/schemas/asset_cook_manifest.schema.json
docs/assets/asset_cooking_pipeline.md
editor/assets/asset_cook_diagnostics_panel.*
engine/core/assets/asset_cook_manifest.*
```

## Native URPG Concepts To Implement

### 1. Cook Rule Contract

A cook rule should describe:

- rule id
- rule version
- priority
- input glob patterns
- output path template
- command type
- command line or native operation
- declared static inputs
- declared static outputs
- optional dep-file path
- optional dep-file format
- cache policy

### 2. Dirty-State Detection

URPG should cook only when needed.

Dirty reasons:

- input missing
- input changed
- output missing
- output outdated
- rule version changed
- dependency file changed
- previous cook failed
- all static inputs removed, requiring cleanup

### 3. Dependency Tracking

Support both static and dynamic dependencies.

Initial dependency formats:

- URPG JSON dep files
- Make-style dep files from compilers/tools
- manifest-declared inputs and outputs

### 4. Cook Cache

Store a deterministic cache record per cooked asset.

Suggested fields:

```json
{
  "rule_id": "sprite_atlas_preview",
  "rule_version": 1,
  "input_path": "content/assets/example.png",
  "input_hash": "sha256",
  "output_paths": ["build/asset-cache/example.preview.png"],
  "dep_paths": [],
  "status": "success | error | skipped | cleaned",
  "last_cooked_at": "iso8601",
  "diagnostics": []
}
```

### 5. Command Execution Boundary

Support command-line tools, but keep them outside the runtime.

Allowed:

- image conversion tools
- atlas/preview generators
- audio preprocessing
- manifest validators
- compression experiments

Not allowed:

- runtime dependency on cooking tools
- blocking editor startup on full recook
- hidden conversion without diagnostics

### 6. Editor Diagnostics

The editor should show:

- dirty assets
- missing outputs
- failed commands
- stale cache entries
- rule version mismatch
- last cook duration
- affected source paths
- affected generated paths

### 7. CI Integration

Add CI gates only after the basic tool lands.

Potential gates:

```text
python tools/assets/cooker/asset_cooker.py --check
python tools/assets/tests/test_asset_cooker.py
python tools/assets/cooker/asset_cooker.py --cook --dry-run
```

## Implementation Phases

### Phase 1: Schema and Dry-Run Tool

Deliver:

- cook rule schema
- default cook rule seed file
- dry-run CLI that discovers candidate inputs and reports would-cook/would-skip
- tests for glob matching, duplicate outputs, and invalid rules

### Phase 2: Cache and Dirty-State Logic

Deliver:

- cook cache manifest
- file hash tracking
- rule version invalidation
- dirty reason reporting
- tests for input changed, output missing, and version mismatch

### Phase 3: First Real Cookers

Deliver first narrow cookers:

- copy curated assets into package staging
- generate preview thumbnails
- validate image size/format
- generate simple atlas metadata

### Phase 4: Editor Diagnostics Panel

Expose the cook status in the editor.

Deliver:

- cooked/dirty/error counts
- selected asset diagnostics
- rebuild selected
- rebuild all dirty
- open output location

### Phase 5: Package/Export Integration

Use cooked outputs during export/package prep.

Deliver:

- export consumes validated cooked artifacts
- missing cooked artifacts fail clearly
- package smoke checks verify cooked payload availability

## Non-Goals

- Do not embed the AssetCooker Win32/D3D11 app.
- Do not vendor the Bedrock framework.
- Do not copy MPL source files without a separate legal/source review.
- Do not run asset cooking inside shipped games.
- Do not use this to justify importing the bulk quarantined asset tree.

## Acceptance Criteria

A future URPG asset-cooking implementation is acceptable when:

- it cooks only changed assets
- it records why assets are dirty
- it has deterministic output manifests
- it has test coverage for cache invalidation
- it works from CLI without UI
- editor diagnostics can explain every failure
- package/export checks consume the same cook manifest

## Final Recommendation

Use AssetCooker as a strong architecture reference for URPG's asset cooking pipeline. Build the URPG version natively, small, testable, and cross-platform.
