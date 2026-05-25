# Feature Robustness Execution Tracker

Status date: 2026-05-25

This document turns `docs/features/FEATURE_ROBUSTNESS_PLAN.md` from prose backlog into governed execution work. The canonical machine-readable tracker is `content/readiness/feature_robustness_lanes.json`.

## Boundary

These ten lanes are URPG product-depth work. They are not bounded `v0.1.0` blockers unless a future scope explicitly promotes a lane into mandatory exit criteria.

## Execution Rule

A lane cannot be marked `ready` until all of these are true:

1. Native/runtime behavior is implemented or intentionally fenced behind a named unsupported/sandbox diagnostic.
2. Editor/WYSIWYG state exposes creator-visible controls, previews, disabled reasons, and blocked reasons.
3. Diagnostics explain failure paths without silent fallback.
4. Focused tests cover success and failure behavior.
5. Status docs name the exact completed scope and do not overclaim.

## First Implementation Slice

This branch starts `FRL-07` by adding `tools/ai/collect_project_knowledge.py`. It performs bounded filesystem walking and emits `filesystem_documents` records in the same shape already accepted by `ProjectKnowledgeIndex`.

Verification:

```powershell
python tools/ci/check_feature_robustness_lanes.py
python -m unittest tools.ai.tests.test_collect_project_knowledge
```

## FRL-06 Painted Diff Slice

This slice moves AI patch review from raw JSON patch rows toward a rendered contract. `buildAiToolResultDiff()` now emits painted add/remove/replace rows with before/after groups, tone classes, icons, selected detail payloads, and blocked-apply reasons. The AI assistant panel and chatbot snapshots both expose the same result diff shape so UI rendering can share one contract.

The completion slice adds a renderer-facing `render_contract` to the shared result diff payload. Editor and chatbot surfaces now expose the `painted_ai_diff_panel` contract with before/after row rendering, selection support, and blocked-apply support so ImGui can render the diff without reinterpreting JSON patches.

`FRL-06` is complete for the governed painted-diff scope. More interactive side-by-side editing should open a new lane.

Verification:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[ai_knowledge]"
python tools/ci/check_feature_robustness_lanes.py
```

## FRL-07 Native Reporting Slice

This slice promotes the crawler output into native AI/editor surfaces. `ProjectKnowledgeIndex` now indexes skipped-file diagnostics from collector output, `buildFilesystemKnowledgeReport()` summarizes direct document rows, freshness, skipped counts, and diagnostic rows, and both the chatbot tool snapshot and AI assistant panel render snapshot expose that filesystem knowledge report. The chatbot also accepts `AI_INGEST_FILESYSTEM_KNOWLEDGE:<json>` and the editor panel exposes `ingestFilesystemKnowledge()` so crawler output can be merged into project data without clobbering project-level fields. Both surfaces expose review/refresh control rows for direct documents and skipped-file diagnostics.

The completion slice adds the adapter-backed refresh contract. `buildFilesystemCrawlerInvocation()` emits the deterministic `tools/ai/collect_project_knowledge.py` command, arguments, output path, config, and follow-up ingest command. The editor snapshot exposes `filesystem_knowledge_refresh`, the editor refresh control points at `refresh_filesystem_knowledge`, and the chatbot supports `AI_REFRESH_FILESYSTEM_KNOWLEDGE` with optional root/output JSON.

`FRL-07` is complete for the governed adapter-backed scope. Future live background indexing or file-watch behavior should open a new lane instead of expanding this one.

Verification:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[ai_knowledge]"
python tools/ci/check_feature_robustness_lanes.py
```

## FRL-08 Validator Routing Slice

This slice deepens `run_validation` from preview-only pass/fail rows into routed subsystem validation evidence. Each validation row now records subsystem, validator source, command, artifact path, severity, and fallback status. Unsupported preview kinds emit a `subsystem_validator_unavailable` warning instead of silently passing.

The remaining `FRL-08` work is to invoke full typed native subsystem validators where project data can provide their real input contracts, plus script-output paths for delegated validators.

Verification:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[ai_knowledge][ai_assistant][tools][validation]"
python tools/ci/check_feature_robustness_lanes.py
```

## FRL-09 Stream Diagnostics Slice

This slice makes streaming delivery observable without replacing the curl transport yet. SSE imports now produce per-chunk diagnostics with partial text, completion state, cancellation state, provider error rows, and final state. `requestStream()` replays captured chunks individually from the response file so deterministic fixture streaming and UI chunk handling use the same event shape.

The completion slice adds an explicit `openai_compatible_stream_adapter` plan. It identifies fixture response replay, curl `--no-buffer` live delivery, callbacks, command, request path, and response path so future socket providers can plug into the same contract while CI keeps deterministic stream diagnostics.

`FRL-09` is complete for the governed adapter-backed streaming scope. A persistent non-curl socket client should be tracked as a new lane if it becomes release scope.

Verification:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[ai][chat][provider][stream]"
python tools/ci/check_feature_robustness_lanes.py
```

## FRL-10 Artifact Policy Slice

This slice adds platform artifact policy rows to export validation reports. Signing, notarization, and launched-smoke evidence now report deterministic status, provider, missing credential key, evidence path, release-required flag, and release blocking counts. The rows are intentionally policy/evidence only until real provider invocation and launched smoke execution are wired.

The completion slice adds provider adapter metadata to the platform artifact policy report. The adapter names the signing/notarization/smoke provider seams, keeps real credentials explicit, and preserves deterministic missing-credential and smoke-evidence states for CI.

`FRL-10` is complete for the governed adapter-backed artifact policy scope. Credentialed provider execution should open a new lane if it becomes mandatory release scope.

Verification:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[export][validation][policy]"
python tools/ci/check_feature_robustness_lanes.py
```

## FRL-03 Stat Control Slice

This slice adds creator-facing stat allocation control rows to the editor snapshot. The panel now exposes pool/class/actor rows, per-stat increment/decrement controls, cap warnings, commit disabled reasons, and post-load apply buttons that distinguish applicable rows from already-applied rows.

The completion slice adds a renderer-facing `stat_allocation_control_panel` contract for segmented pool rows, stat stepper rows, and saved-allocation apply rows. The snapshot continues to carry cap warnings and disabled reasons so the ImGui layer can render controls without recomputing progression rules.

`FRL-03` is complete for the governed visual-control contract scope.

Verification:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[progression][stat_allocation]"
python tools/ci/check_feature_robustness_lanes.py
```

## FRL-05 Asset Readiness Slice

This slice adds shared package/readiness diagnostics to asset action and preview rows. Missing license evidence, runtime payloads, thumbnail metadata, waveform peaks, and sequence/video representative previews now surface as structured diagnostics with severity and target fields instead of only implicit preview states.

The remaining `FRL-05` work is curated bulk-promotion manifests plus generated preview media evidence for the next release-safe payload groups.

Verification:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library]"
python tools/ci/check_feature_robustness_lanes.py
```

## FRL-04 Task Replay Slice

This slice deepens ability task-graph runtime reporting. Execution events now include task cursor, wait state, next-task id, and branch decision metadata, and result JSON exposes `taskRuntimeReplay` rows plus wait-row and cancellation summaries for deterministic replay/debug views.

The remaining `FRL-04` work is long-running async scheduling hooks for waits that span frames while keeping arbitrary scripting behind the explicit unsupported/sandbox policy.

Verification:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[ability][orchestration][task][phase_two]"
python tools/ci/check_feature_robustness_lanes.py
```

## FRL-01 Follow-Up Slice

This branch completes the governed `FRL-01` scope by adding native imported/plugin-style battle feedback fixture parsing through `BattleRuleResolver::importFeedbackPolicyFixture()`. The importer accepts plugin parameter-style keys and string values for chip damage, chip healing, zero-damage presentation, custom buff caps, and troop-position reuse; it emits success/failure diagnostics plus deterministic fixture coverage rows. `BattlePresentationProfileFromJson()` now ingests `feedback_policy`, `BattlePresentationProfileToJson()` writes the normalized schema-versioned policy back out, and `BattlePresentationPanelSnapshot` exposes feedback fixture coverage counts, policy diagnostic counts, and editor-facing coverage row labels. Focused unit, profile/panel, and compat fixture tests now cover the completed lane.

Verification:

```powershell
ctest --preset dev-all -R "BattleRuleResolver|battle presentation|Compat fixture import: battle feedback" --output-on-failure
python tools\ci\check_feature_robustness_lanes.py
```

## FRL-02 Follow-Up Slice

This slice makes unsupported state/message/picture fixture imports visible instead of only diagnostic side effects. Compat migration now keeps dropped scoped-state rows and invalid picture-task bindings under `unsupported_rows`, including source index, source row, kind, code, and reason, while preserving the existing diagnostic stream. The focused migration test now covers unsupported state scope, malformed state rows, non-scalar state values, invalid picture bindings, and malformed picture rows.

Verification target:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[message][migration]"
python tools\ci\check_feature_robustness_lanes.py
```

## Lane Order

| Lane | System | Remaining Depth |
| --- | --- | --- |
| `FRL-01` | Battle feedback | Complete for the governed FRL-01 fixture-depth scope. |
| `FRL-02` | State/message/picture | Dedicated compat fixture files and rendered high-count picture snapshot evidence. |
| `FRL-03` | Progression | Complete for stat allocation control rows and render contract. |
| `FRL-04` | Gameplay abilities | Long-running async scheduling and scripting sandbox policy. |
| `FRL-05` | Asset browser/runtime library | Curated bulk promotion and generated preview media evidence. |
| `FRL-06` | AI editor workflow | Complete for shared painted diff rows and render contract. |
| `FRL-07` | Project knowledge indexing | Complete for adapter-backed crawler invocation and ingestion. |
| `FRL-08` | Concrete AI tools | Full typed native validator invocation and delegated output paths. |
| `FRL-09` | Live chat providers | Complete for adapter-backed streaming diagnostics. |
| `FRL-10` | Export/release UX | Complete for adapter-backed artifact policy and smoke evidence rows. |
