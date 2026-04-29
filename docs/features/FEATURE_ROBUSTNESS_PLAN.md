# Feature Robustness Plan

Status date: 2026-04-29

URPG has enough native subsystems now that the next work should deepen existing surfaces instead of adding isolated demos. This plan tracks the feature-packed follow-through requested after the asset, AI, and plugin absorption passes.

## Priority Lanes

1. Battle feedback: chip damage/healing, configurable zero-damage presentation, custom buff caps, troop-position reuse, preview panel visibility, schema coverage, and legacy policy migration are started in native code; broader battle feedback fixture coverage remains.
2. State/message/picture: scoped/self/map/JS variable banks, nested text escapes, picture task bindings, high-count picture management, native picture UI runtime preview rows, and compat migration adapters for scoped state/picture tasks are started in native code; broader fixture import coverage remains.
3. Progression: level-up stat allocation pools, caps, class/actor rules, preview math, editor snapshots, applied stat-allocation save/load records, and post-load actor application rows are started in native code; richer visual controls remain.
4. Asset browser/runtime library: tag/status/category filters, duplicate groups, source/license badges, previewable/runtime-ready counts, used-by references, promote/archive action state, and shared editor/chatbot asset action rows are started in native code; actual file moves plus richer thumbnail/waveform UI remain.
5. AI editor workflow: approve/reject/apply/revert controls, validation blocking reasons, apply-preview patch counts, result diff patch counts, and undo-stack/apply-history snapshots are started in native code; richer painted diff browsing and per-record rationale UI remain.
6. Project knowledge indexing: project files, schemas, readiness reports, validation reports, asset catalogs, project docs, template specs, and source summaries are started in native project-data indexing; direct filesystem/doc ingestion and freshness checks remain.
7. Concrete AI tools: AI applies now emit concrete editor preview artifacts for event graphs, battle VFX timelines, lighting/weather previews, ability sandbox compositions, export preview configuration, and asset import/promotion; deeper native validator execution remains.
8. Live chat providers: OpenAI-compatible `IChatService` transport is started with deterministic dry-run request/command building, curl execution, and common response import for ChatGPT-compatible gateways, Kimi-compatible gateways, Ollama, LM Studio, OpenRouter, vLLM, and LocalAI; richer provider profile UI and streaming remain.
9. Export/release UX: export preview now exposes platform checklist rows, missing asset/artifact report, packaging diagnostics, signing/notarization status, and staged smoke evidence in the result and editor panel snapshot; full native signing/notarization and launched multi-platform smoke remain.

## Implementation Rule

Each lane should land as native project data, schema coverage when data-driven, runtime behavior, editor state/preview, diagnostics, focused tests, and status documentation. Raw imported plugin or asset folders remain ignored intake evidence, not shipped runtime dependencies.

## Chatbot and WYSIWYG Coverage

The chatbot and WYSIWYG bridge now has a native coverage report. Release top-level editor panels are indexed into chatbot knowledge as searchable `editor_panel` entries, AI capabilities must declare a WYSIWYG surface and at least one tool, and the asset lane is checked for both WYSIWYG panel registration and asset import/promotion tool coverage. The report is surfaced through both `AiAssistantPanel` and `ChatbotComponent`, with `AssetLibrarySnapshot` injection so the editor and chatbot can show asset action readiness alongside feature/tool coverage.

## Asset Action Rows

Asset action recommendations are shared by the WYSIWYG asset browser and chatbot snapshots through `buildAssetActionRows()`. Rows expose preview metadata, tags, usage references, status badges, promote/archive button enablement, disabled reasons, and recommendations such as `promote`, `archive_duplicate`, `add_license_evidence`, `fix_missing_file`, `convert_or_replace`, `ready`, and `archived`.

## Progression Save Records

Stat allocation now has a commit record that can be attached to runtime save JSON under `_stat_allocations` and hydrated by `RuntimeSaveLoader`. The record stores pool/class/actor identity, before/after stats, spent and remaining points, and per-stat spending so level-up allocation previews can survive save/load.

## Progression Post-Load Apply Rows

Loaded stat-allocation records now build actor-specific application previews that show current stats, saved after-stats, spent/remaining points, applicable/already-applied/blocked state, and diagnostics. `StatAllocationPanel` exposes those rows in its snapshot with explicit post-load actor selection so the editor, chatbot, and WYSIWYG surfaces can present loaded progression changes before applying them.

## Picture Runtime Preview

`PictureTaskDocument::previewRuntime()` now projects high-count picture slots into WYSIWYG-ready rows with bounds, z order, opacity, visibility, click/hover binding state, common-event targets, hover hit testing, and diagnostics for out-of-range runtime slots.

## State and Picture Migration

`UpgradeCompatMessageDocument()` now emits native `scoped_state_banks` and `picture_tasks` documents alongside migrated dialogue/style output. It maps plugin-style scoped/self/map/JS switch and variable rows, preserves scalar values, normalizes picture task triggers, drops unsupported rows with diagnostics, and exposes `scoped_state_banks.schema.json` as the data contract for imported state-bank records.

## Battle Feedback Policy

Battle feedback policy now round-trips through schema-versioned JSON, migrates legacy snake-case/plugin-style keys, clamps chip percentages and minimum values, and is exposed in `battle_presentation.schema.json` for editor/project authoring.
