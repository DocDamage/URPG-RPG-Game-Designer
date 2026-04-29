# Feature Robustness Plan

Status date: 2026-04-29

URPG has enough native subsystems now that the next work should deepen existing surfaces instead of adding isolated demos. This plan tracks the feature-packed follow-through requested after the asset, AI, and plugin absorption passes.

## Priority Lanes

1. Battle feedback: chip damage/healing, configurable zero-damage presentation, custom buff caps, troop-position reuse, preview panel visibility, and migration/schema coverage.
2. State/message/picture: scoped/self/map/JS variable banks, nested text escapes, picture task bindings, and high-count picture management are started in native code; picture UI runtime preview remains.
3. Progression: level-up stat allocation pools, caps, class/actor rules, preview math, and editor snapshots are started in native code; save integration remains.
4. Asset browser/runtime library: tag/status/category filters, duplicate groups, source/license badges, previewable/runtime-ready counts, used-by references, and promote/archive action state are started in native code; actual file moves plus richer thumbnail/waveform UI remain.
5. AI editor workflow: approve/reject/apply/revert controls, validation blocking reasons, apply-preview patch counts, result diff patch counts, and undo-stack/apply-history snapshots are started in native code; richer painted diff browsing and per-record rationale UI remain.
6. Project knowledge indexing: project files, schemas, readiness reports, validation reports, asset catalogs, project docs, template specs, and source summaries are started in native project-data indexing; direct filesystem/doc ingestion and freshness checks remain.
7. Concrete AI tools: AI applies now emit concrete editor preview artifacts for event graphs, battle VFX timelines, lighting/weather previews, ability sandbox compositions, export preview configuration, and asset import/promotion; deeper native validator execution remains.
8. Live chat providers: OpenAI-compatible `IChatService` transport is started with deterministic dry-run request/command building, curl execution, and common response import for ChatGPT-compatible gateways, Kimi-compatible gateways, Ollama, LM Studio, OpenRouter, vLLM, and LocalAI; richer provider profile UI and streaming remain.
9. Export/release UX: export preview now exposes platform checklist rows, missing asset/artifact report, packaging diagnostics, signing/notarization status, and staged smoke evidence in the result and editor panel snapshot; full native signing/notarization and launched multi-platform smoke remain.

## Implementation Rule

Each lane should land as native project data, schema coverage when data-driven, runtime behavior, editor state/preview, diagnostics, focused tests, and status documentation. Raw imported plugin or asset folders remain ignored intake evidence, not shipped runtime dependencies.

## Chatbot and WYSIWYG Coverage

The chatbot and WYSIWYG bridge now has a native coverage report. Release top-level editor panels are indexed into chatbot knowledge as searchable `editor_panel` entries, AI capabilities must declare a WYSIWYG surface and at least one tool, and the asset lane is checked for both WYSIWYG panel registration and asset import/promotion tool coverage. The report is surfaced through both `AiAssistantPanel` and `ChatbotComponent`, with `AssetLibrarySnapshot` injection so the editor and chatbot can show asset action readiness alongside feature/tool coverage.
