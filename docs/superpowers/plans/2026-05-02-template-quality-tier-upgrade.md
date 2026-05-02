# Template Quality Tier Upgrade Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Upgrade all 76 READY templates with a shared quality-tier contract that improves template metadata, starter manifest usefulness, generated-project visibility, and governance validation without changing READY semantics.

**Architecture:** Add a data-first quality contract to every starter manifest and runtime profile JSON. Keep readiness separate from quality by preserving `status: READY` while adding tier, family, target camera, content, input, accessibility, audio, localization, performance, export, and known-limit metadata. Enforce the contract with focused C++ tests and existing PowerShell truth gates.

**Tech Stack:** C++20, nlohmann::json, Catch2, PowerShell validation scripts, Markdown docs, canonical JSON starter manifests.

---

## File Structure

- Modify: `content/templates/*_starter.json`
  - Add `quality_contract` to every starter manifest.
- Modify: `engine/core/project/template_runtime_profile.cpp`
  - Add generated `quality_contract` to `templateRuntimeProfileToJson` so generated projects expose the same contract.
- Modify: `tests/unit/test_template_acceptance.cpp`
  - Add tests that every canonical starter manifest and every generated runtime profile exposes a complete quality contract.
- Create: `docs/templates/TEMPLATE_QUALITY_CONTRACT.md`
  - Document tiers, required fields, known limits, and upgrade path.
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`
  - Record the quality-tier upgrade layer.

---

### Task 1: Add Quality Contract Metadata

**Files:**
- Modify: `content/templates/*_starter.json`
- Modify: `engine/core/project/template_runtime_profile.cpp`

- [ ] **Step 1: Generate starter manifest metadata**

For each template in `content/readiness/readiness_status.json`, update `content/templates/<template_id>_starter.json` with:

```json
"quality_contract": {
  "tier": "starter_ready",
  "quality_score": 70,
  "genre_family": "party_rpg",
  "target_camera": "top_down",
  "core_loops": ["save_loop"],
  "starter_content": {
    "maps_or_scenes": 3,
    "sample_entities": 4,
    "sample_progression_hooks": 2,
    "wysiwyg_surfaces": ["surface_a", "surface_b"]
  },
  "required_inputs": ["Confirm", "Cancel", "Menu"],
  "default_accessibility": ["focus_labels", "readable_contrast", "remap_visible_actions"],
  "audio_profile": "starter_mix",
  "localization_namespaces": ["template_id"],
  "performance_budget": {
    "frame_time_ms": 33.3,
    "arena_kb": 512,
    "active_entities": 128
  },
  "export_profile": "standalone_strict",
  "known_limits": [
    "bounded starter content only",
    "no platform-holder certification",
    "no online services or live economy"
  ]
}
```

- [ ] **Step 2: Add runtime profile quality JSON**

In `template_runtime_profile.cpp`, add a helper that derives a matching `quality_contract` from `TemplateRuntimeProfile` and include it in `templateRuntimeProfileToJson(profile)`.

- [ ] **Step 3: Run a fast manifest sanity check**

Run:

```powershell
$r = Get-Content content/readiness/readiness_status.json -Raw | ConvertFrom-Json
$r.templates | ForEach-Object { Test-Path "content/templates/$($_.id)_starter.json" }
```

Expected: all output values are `True`.

---

### Task 2: Add Validation Tests

**Files:**
- Modify: `tests/unit/test_template_acceptance.cpp`

- [ ] **Step 1: Add starter manifest contract test**

Add a Catch2 test that reads every canonical template from `content/readiness/readiness_status.json`, opens `content/templates/<id>_starter.json`, and requires non-empty fields for:

```cpp
quality_contract.tier
quality_contract.genre_family
quality_contract.target_camera
quality_contract.core_loops
quality_contract.starter_content
quality_contract.required_inputs
quality_contract.default_accessibility
quality_contract.audio_profile
quality_contract.localization_namespaces
quality_contract.performance_budget
quality_contract.export_profile
quality_contract.known_limits
```

- [ ] **Step 2: Add generated runtime-profile contract test**

Use `urpg::project::allTemplateRuntimeProfiles()` and `templateRuntimeProfileToJson(profile)` to assert each profile JSON contains the same `quality_contract` shape.

- [ ] **Step 3: Build affected test host**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests urpg_project_audit_unit_tests
```

Expected: exit code 0.

---

### Task 3: Document Quality Tiers

**Files:**
- Create: `docs/templates/TEMPLATE_QUALITY_CONTRACT.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [ ] **Step 1: Create quality contract doc**

Document:

```markdown
# Template Quality Contract

Status Date: 2026-05-01
Authority: canonical template quality-tier contract

## Tiers

| Tier | Meaning |
| --- | --- |
| `starter_ready` | Bounded starter content is coherent, validated, and export-ready. |
| `showcase_ready` | Starter includes richer demo flow and family-specific WYSIWYG surfaces. |
| `production_seed` | Starter is suitable as a production seed with deeper balance/content diagnostics. |

## Required Manifest Fields
...
```

- [ ] **Step 2: Update program status docs**

Add a short bullet stating that all 76 templates now carry `starter_ready` quality metadata while readiness remains a separate release claim.

---

### Task 4: Verify

**Files:**
- No new edits unless verification reveals a defect.

- [ ] **Step 1: Run template truth gates**

Run:

```powershell
.\tools\ci\check_template_spec_bar_drift.ps1
.\tools\ci\truth_reconciler.ps1
.\tools\ci\check_template_claims.ps1
.\tools\ci\check_release_readiness.ps1
```

Expected: all exit 0.

- [ ] **Step 2: Run focused CTest slice**

Run:

```powershell
ctest --preset dev-all -R "template|Template|project_audit|Project audit|governance|Governance|certification" --output-on-failure
```

Expected: 100% tests passed.

- [ ] **Step 3: Run repo hygiene**

Run:

```powershell
pre-commit run --all-files
git diff --check
```

Expected: both exit 0.

---

## Self-Review

- Spec coverage: The plan covers tier metadata, shared template contract fields, family grouping, WYSIWYG/content quality metadata, known limits, documentation, and validation.
- Placeholder scan: No TODO/TBD placeholders are present.
- Type consistency: The manifest and runtime JSON field is consistently named `quality_contract` across data, code, tests, and docs.
