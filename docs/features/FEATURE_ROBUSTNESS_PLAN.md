# Feature Robustness Plan

Status date: 2026-04-29

URPG has enough native subsystems now that the next work should deepen existing surfaces instead of adding isolated demos. This plan tracks the feature-packed follow-through requested after the asset, AI, and plugin absorption passes.

## Priority Lanes

1. Battle feedback: chip damage/healing, configurable zero-damage presentation, custom buff caps, troop-position reuse, preview panel visibility, schema coverage, and legacy policy migration are started in native code; broader battle feedback fixture coverage remains.
2. State/message/picture: scoped/self/map/JS variable banks, nested text escapes, picture task bindings, high-count picture management, native picture UI runtime preview rows, and compat migration adapters for scoped state/picture tasks are started in native code; broader fixture import coverage remains.
3. Progression: level-up stat allocation pools, caps, class/actor rules, preview math, editor snapshots, applied stat-allocation save/load records, and post-load actor application rows are started in native code; richer visual controls remain.
4. Asset browser/runtime library: tag/status/category filters, duplicate groups, source/license badges, previewable/runtime-ready counts, used-by references, promote/archive action state, shared editor/chatbot asset action rows, shared thumbnail/waveform preview rows, aggregate image-sequence pack metadata, aggregate sequence/frame/clip counts, sequence-pack quick-filter state, and filtered visible action/preview rows are started in native code; curated file promotion and richer generated preview assets remain.
5. AI editor workflow: approve/reject/apply/revert controls, validation blocking reasons, apply-preview patch counts, result diff patch counts, undo-stack/apply-history snapshots, structured diff rows, and per-step rationale rows are started in native code; richer painted diff rendering remains.
6. Project knowledge indexing: project files, schemas, readiness reports, validation reports, asset catalogs, project docs, template specs, source summaries, directly ingested filesystem/doc records, and freshness metadata are started in native project-data indexing; broader automatic filesystem walking remains.
7. Concrete AI tools: AI applies now emit concrete editor preview artifacts for event graphs, battle VFX timelines, lighting/weather previews, ability sandbox compositions, export preview configuration, and asset import/promotion; `run_validation` now executes deterministic validator rows over those preview artifacts, while broader subsystem validator invocation remains.
8. Live chat providers: OpenAI-compatible `IChatService` transport is started with deterministic dry-run request/command building, curl execution, common response import, provider profile UI state, streaming request toggles, curl `--no-buffer`, and SSE response import for ChatGPT-compatible gateways, Kimi-compatible gateways, Ollama, LM Studio, OpenRouter, vLLM, and LocalAI; true socket-level live chunk delivery remains.
9. Export/release UX: export preview now exposes platform checklist rows, missing asset/artifact report, packaging diagnostics, signing/notarization status, and staged smoke evidence in the result and editor panel snapshot; full native signing/notarization and launched multi-platform smoke remain.

## Implementation Rule

Each lane should land as native project data, schema coverage when data-driven, runtime behavior, editor state/preview, diagnostics, focused tests, and status documentation. Raw imported plugin or asset folders remain ignored intake evidence, not shipped runtime dependencies.

## Chatbot and WYSIWYG Coverage

The chatbot and WYSIWYG bridge now has a native coverage report. Release top-level editor panels are indexed into chatbot knowledge as searchable `editor_panel` entries, AI capabilities must declare a WYSIWYG surface and at least one tool, and the asset lane is checked for both WYSIWYG panel registration and asset import/promotion tool coverage. The report is surfaced through both `AiAssistantPanel` and `ChatbotComponent`, with `AssetLibrarySnapshot` injection so the editor and chatbot can show asset action readiness alongside feature/tool coverage.

## Asset Action Rows

Asset action recommendations are shared by the WYSIWYG asset browser and chatbot snapshots through `buildAssetActionRows()`. Rows expose preview metadata, tags, usage references, status badges, promote/archive button enablement, disabled reasons, aggregate sequence metadata for large animation packs, and recommendations such as `promote`, `archive_duplicate`, `add_license_evidence`, `fix_missing_file`, `convert_or_replace`, `ready`, and `archived`.

## Asset Preview Rows

Asset preview metadata is now shared through `buildAssetPreviewRows()`. Promotion catalog records can carry thumbnail dimensions, audio duration, waveform peaks, and aggregate sequence fields such as frame count, sequence count, and representative sequence samples; the editor asset browser, AI assistant panel, and chatbot snapshots expose ready/pending/missing preview state for images, video thumbnails, audio waveforms, and large animation-frame packs without relying on a separate UI-only interpretation. `AssetLibrarySnapshot` and `AssetLibraryModelSnapshot` also expose aggregate sequence asset, frame, and clip counts so WYSIWYG and chatbot surfaces can summarize SRC-008-style drops without scanning raw files.

## Asset Filter Controls

`AssetLibraryModelSnapshot` now exposes deterministic filter controls for WYSIWYG and chatbot surfaces. The snapshot records the active filter, filtered result count, and quick filters for sequence packs, runtime-ready assets, and previewable assets; sequence-pack quick filters include aggregate pack, frame, and clip counts so SRC-008-style animation drops are discoverable without opening raw intake folders. Asset action rows and preview rows are built from the active filtered asset set, while top-level counts continue to summarize the full loaded library.

## AI Review Diff Rows

`AiAssistantPanel` now projects each reviewable step into rationale rows with capability, tool, approval state, project paths, arguments, and a user-facing rationale string. Apply previews, result diffs, and apply-history entries now expose structured diff rows derived from JSON patches, including operation, root path, before/after values, tone, and summary data for a future painted diff browser.

## Project Knowledge Freshness

`ProjectKnowledgeIndex` now accepts directly ingested `filesystem_documents` and `ingested_docs` records supplied through project data. Entries retain content excerpts, content sizes, path/kind metadata, direct-ingestion flags, and deterministic freshness state from indexed/modified timestamps or age thresholds so chatbot search can find real file/doc text and highlight stale records.

## Concrete AI Validation

`run_validation` now builds `last_ai_validation`, appends `ai_validation_reports`, and emits a `validation_execution` preview artifact. The report executes deterministic validator rows for AI-generated event graph, ability sandbox, battle VFX timeline, lighting/weather, asset import/promotion, and export preview artifacts, surfacing issue counts, severity status, validator names, and structured issue records.

## Live Provider UI

OpenAI-compatible provider profiles now expose editor-ready UI state for ChatGPT/OpenAI, OpenRouter, Kimi/Moonshot, Ollama, LM Studio, vLLM, and LocalAI. `AiAssistantPanel` surfaces selectable profile rows, resolved endpoint/model, API-key requirements, local/hosted provider status, dry-run versus live execution state, test-request controls, a stream toggle, and `available` versus `streaming_requested` state. `OpenAiCompatibleChatService::requestStream()` now sends OpenAI-compatible `stream: true` request bodies, adds curl `--no-buffer`, and imports saved SSE `data:` chunks into the same response/command callback shape used by normal chat responses.

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
