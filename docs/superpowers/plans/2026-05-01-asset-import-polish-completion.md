# Asset Import Polish Completion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Finish the remaining global asset import polish items after Phases 1-5 by surfacing extractor configuration state, locking nested archive behavior, cleaning stale docs, and running a broader validation slice.

**Architecture:** Keep runtime/editor code independent from Python internals. The editor model will only expose a deterministic configuration snapshot and command handoff; the Python importer remains responsible for bounded extraction and archive scanning behavior.

**Tech Stack:** C++20 editor model with Catch2 tests, Python importer with unittest coverage, Markdown docs, CMake/Ninja validation.

---

### Task 1: Expose Extractor Configuration Status In Wizard Snapshot

**Files:**
- Modify: `editor/assets/asset_library_model.cpp`
- Modify: `editor/assets/asset_library_model.h`
- Modify: `editor/assets/asset_library_panel.cpp`
- Modify: `editor/assets/asset_library_panel.h`
- Test: `tests/unit/test_asset_library_panel.cpp`
- Docs: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`

- [x] **Step 1: Write the failing C++ test**

Add Catch2 cases near existing Add Source handoff tests asserting that `snapshot().import_wizard["extractor_configuration"]`
and `AssetLibraryPanel::lastImportWizardSnapshot().extractor_configuration` report `configured`, `source`, `command`,
and `supports_rar_7z` before any pending Add Source request exists.

- [x] **Step 2: Run the focused test to verify red**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryModel exposes configured archive extractor status in the wizard snapshot" --output-on-failure
```

Expected: FAIL because `extractor_configuration` is missing.

- [x] **Step 3: Implement the model snapshot field**

Add a small helper that reads `URPG_ASSET_ARCHIVE_EXTRACTOR`, splits it with the same command parser used for Add
Source, and returns JSON. Initialize the model snapshot in the default constructor and copy the JSON into the panel's
typed render snapshot:

```json
{
  "configured": true,
  "source": "environment",
  "environment_variable": "URPG_ASSET_ARCHIVE_EXTRACTOR",
  "supports_rar_7z": true,
  "command": ["C:/Program Files/7-Zip/7z.exe", "x", "-y", "{source}", "-o{destination}"]
}
```

When not configured, return `configured: false`, `source: "none"`, `supports_rar_7z: false`, and an empty command array.

- [x] **Step 4: Run focused and adjacent C++ tests**

Run:

```powershell
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryModel exposes configured archive extractor status in the wizard snapshot|AssetLibraryModel applies configured external archive extractor to add-source requests|AssetLibraryModel requests add-source.*handoff" --output-on-failure
```

Expected: PASS.

### Task 2: Add Nested Archive Import Regression Coverage

**Files:**
- Modify: `tools/assets/tests/test_global_asset_import.py`
- Docs: `docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md`

- [x] **Step 1: Write the failing or confirming Python test**

Add `test_nested_archives_are_cataloged_without_recursive_extraction`, creating an outer ZIP with `nested/inner.zip` and a regular image. Assert the session has an archive record for `nested/inner.zip`, no record extracted from inside the inner archive, and the archive record carries `unsupported_format`.

- [x] **Step 2: Run the Python importer test**

Run:

```powershell
python tools/assets/tests/test_global_asset_import.py
```

Expected: PASS if existing behavior already matches policy; otherwise FAIL on recursive extraction or missing archive record.

- [x] **Step 3: Implement only if red**

If the test fails, change only `tools/assets/global_asset_import.py` scan/copy behavior so nested archive files remain catalog records and are not recursively extracted.

- [x] **Step 4: Re-run the Python importer test**

Run:

```powershell
python tools/assets/tests/test_global_asset_import.py
```

Expected: PASS.

### Task 3: Clean Stale Future/Later Wording

**Files:**
- Modify: `docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md`
- Modify: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`

- [x] **Step 1: Update stale text**

Replace older “later/future” wording for RAR/7z extraction, conversion workflows, native picker, animation grouping, and RPG Maker mapping with current implementation status while preserving first-pass constraints.

- [x] **Step 2: Scan for stale markers**

Run:

```powershell
rg -n "RAR/7z later|catalog_animation_asset_drop.py for aggregate animation-frame handling later|convert_audio_to_ogg.py as a reference for future conversion flows|Phase 1 intentionally did not claim native file-dialog UX, conversion workflows, optional RAR/7z extraction" docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md
```

Expected: no matches.

### Task 4: Document Settings Surface Path

**Files:**
- Modify: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`
- Modify: `docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md`

- [x] **Step 1: Add configuration note**

Document that `URPG_ASSET_ARCHIVE_EXTRACTOR` is the current deterministic local configuration hook, and that a future editor/user settings surface should persist the same argv vector shape rather than a shell command string.

- [x] **Step 2: Run markdown/stale text checks**

Run:

```powershell
rg -n "URPG_ASSET_ARCHIVE_EXTRACTOR|settings surface|argv vector" docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md
git diff --check
```

Expected: relevant matches for the new docs and no whitespace errors.

### Task 5: Broader Validation And Commit

**Files:**
- No source changes expected beyond previous tasks.

- [x] **Step 1: Run focused regression slice**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryModel exposes configured archive extractor status in the wizard snapshot|AssetLibraryModel applies configured external archive extractor to add-source requests|AssetLibraryModel requests add-source.*handoff|AssetLibraryPanel requests add-source through an import source picker|Asset.*(Library|Import|Attachment)|asset_library|asset_attachment|global_asset_import|ExportPackager.*asset" --output-on-failure
python tools/assets/tests/test_global_asset_import.py
git diff --check
```

Expected: PASS.

- [x] **Step 2: Commit and push**

Run:

```powershell
git add docs/superpowers/plans/2026-05-01-asset-import-polish-completion.md docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md editor/assets/asset_library_model.cpp tests/unit/test_asset_library_panel.cpp tools/assets/tests/test_global_asset_import.py
git commit -m "Complete asset import polish items"
git push
```
