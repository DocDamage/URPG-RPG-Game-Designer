# Editor Control Inventory

Status Date: 2026-04-30

This inventory records the P2-002 sweep of user-facing ImGui controls under `editor/`, plus the current AI assistant review controls and native Level Builder controls exposed through deterministic render snapshots.

## Production Panel Exposure Map

`engine/core/editor/editor_panel_registry.cpp` is the source of truth for editor shell exposure.

| Exposure | Panel IDs | Release rationale |
| --- | --- | --- |
| `ReleaseTopLevel` | `diagnostics`, `assets`, `ability`, `patterns`, `mod`, `analytics`, `level_builder` | Registered production navigation surfaces currently wired through the editor app shell and smoke workflow. `level_builder` is the native grid-part map editor surface. |
| `Nested` | `compat_report`, `save_inspector`, `event_authority`, `message_inspector`, `battle_inspector`, `menu_inspector`, `menu_preview`, `audio_inspector`, `migration_wizard`, `project_audit`, `project_health` | Rendered as tabs or child surfaces inside the Diagnostics workspace. |
| `Nested` | `spatial_authoring`, `elevation_brush`, `terrain_brush`, `region_rules`, `procedural_map`, `prop_placement`, `map_ability_binding`, `spatial_ability_canvas` | Rendered as supporting spatial tools under the native Level Builder or direct subsystem tests rather than primary shell navigation. |
| `DevOnly` | `diagnostics_bundle`, `developer_debug_overlay`, `ai_assistant`, `local_review`, `mod_sdk`, `core_asset_browser`, `core_hierarchy`, `core_property_inspector` | Support, collaboration, debug, SDK, or legacy core-editor tooling; compiled for developer workflows and excluded from release navigation. |
| `Deferred` | Implemented panels not wired into release navigation, plus feature-family records under `editor/gameplay`, `editor/community`, and `editor/maker` that are retained for direct tests, snapshots, or roadmap work. Examples include `event_authoring`, `plugin_inspector`, `new_project_wizard`, `quest`, `dialogue_graph`, `narrative_continuity`, `relationship`, `localization_workspace`, `timeline`, `replay`, `capture`, `photo_mode`, `database`, `balance`, `vendor`, `world`, `crafting`, `codex`, `calendar`, `npc`, `puzzle`, `export_diagnostics`, `character_creator`, `achievement`, `controller_binding`, `save_debugger`, `save_migration_preview`, `battle_presentation`, `boss_designer`, `formula_debugger`, `battle_preview`, `perf_diagnostics`, `sprite_animation_preview`, `accessibility`, `accessibility_assistant`, `audio_mix`, `input_remap`, `device_profile`, and `theme_builder`. | Compiled panels retained for direct tests, snapshots, or roadmap work; each registry entry documents the workflow or promotion gate required before release navigation. Feature-family entries are automatically demoted unless they are listed in the canonical release top-level set. |

Exhaustive compiled-panel ownership is enforced by `tests/unit/test_editor_panel_registry.cpp`. The test scans every `editor/**/*_panel.cpp` and `editor/**/*_workspace.cpp` file and requires exactly one registry exposure owner for each compiled user-facing surface. Shared implementation panels, such as gameplay/community/maker WYSIWYG families, grid-part child panels, save preview labs, and message/dialogue previews, are classified under their owning registry surfaces rather than release navigation.

## Native Level Builder Snapshot Controls

`editor/spatial/level_builder_workspace.*` is the release top-level map editor surface. It exposes shell-bindable toolbar/action IDs and deterministic command results for:

- `build`, `validate`, `playtest`, `package`, and `supporting_spatial` workflow modes.
- `save_level_draft`, `load_level_draft`, and `export_current_level` document lifecycle commands.
- `undo` and `redo` across placement and inspector edit histories.
- `mark_player_spawn` and `set_reach_exit_objective` for native level intent authoring.
- `mark_target_export_checks_passed`, `mark_accessibility_checks_passed`, `mark_performance_budget_passed`, and `mark_human_review_passed` for package-readiness evidence.
- Diagnostic rows with focus support through `FocusDiagnostic`.
- Supporting spatial pass-through actions: `supporting_elevation`, `supporting_props`, `supporting_abilities`, and `supporting_composite`.

Regression evidence: `tests/unit/test_grid_part_editor.cpp` and the CTest `grid_part` label lane.

## Release Panel State Evidence

The release top-level panels expose deterministic headless state evidence for Phase 3 empty, disabled, and error review:

- `analytics`: JSON snapshot reports dispatcher/uploader/profile binding, status messages, upload status, disabled upload reason, validation issues, and the latest action.
- `assets`: `AssetLibraryModelSnapshot` reports `status`, `status_message`, `error_message`, `remediation`, filter controls, asset action rows, and action history.
- `ability`: `AbilityInspectorPanel::RenderSnapshot` reports `status`, `disabled_reason`, `empty_reason`, `error_message`, command controls, draft validation issues, and latest command result.
- `patterns`: `PatternFieldPanel::RenderSnapshot` reports `status`, `empty_reason`, `error_message`, validation issues, active point count, and per-control disabled reasons.
- `mod`: JSON snapshot reports `status`, `disabled_reason`, `empty_reason`, `error_message`, binding state, action enablement, status messages, validation issues, cycle warning, hot-load events, and latest action.
- `level_builder`: `LevelBuilderWorkspace::RenderSnapshot` reports disabled/error/ready status, remediation, toolbar action state, validation diagnostics, readiness state, and command result snapshots.
- `diagnostics`: `DiagnosticsWorkspace::exportAsJson()` reports visible/active tab state, active-tab summary, and the active child panel snapshot; child panels carry their own empty/error/disabled evidence.

## Release Persistence Evidence

Phase 4 verifies release authoring actions through deterministic round-trip tests and path-specific failure feedback:

- Ability draft IO saves to nested paths, reloads into fresh workspaces, reports `last_io.operation`, `last_io.success`, `last_io.path`, and `last_io.message`, and preserves the previous valid draft after malformed loads.
- Ability project-content IO creates parent directories under `content/abilities`, refreshes the picker, selects the saved asset, and reports the saved project-content path through `last_io`.
- Level Builder save/load/export/playtest/package paths are covered by `tests/unit/test_grid_part_editor.cpp`; failed loads reject malformed or map-mismatched drafts without replacing the bound document.
- Analytics consent/settings and local JSONL export paths are covered by `tests/unit/test_analytics_panel.cpp` and `tests/unit/test_app_settings_store.cpp`.
- Optional external RPG Maker sample data tests fall back to seeded DataManager contracts when the sample project files are not present.

## Release Input And Pause Evidence

Phase 4 input checks keep runtime/editor navigation behavior explicit:

- Runtime startup maps keyboard confirm, cancel, and menu inputs through `RuntimeStartupServices`; controller defaults bind movement, confirm, cancel, and menu through `ControllerBindingRuntime`.
- Title and options scenes handle the same confirm/cancel/navigation action edges; map confirm routes authored interaction abilities and dialogue fallback.
- Native `MenuScene` forwards confirm, cancel, and directional actions into `MenuSceneGraph`, so runtime menus no longer silently ignore input.
- `SceneManager` suppresses active-scene input and update while paused, preserves the scene stack, and can clear stale `InputCore` action edges during pause.
- Touch remaps return `touch_binding_unsupported`; touch remains supported through hit-test driven UI/world paths rather than the action remap profile.

## Search Command

```powershell
rg -n "ImGui::(Button|MenuItem|Checkbox|Combo|Selectable|Slider|Drag)" editor
```

## Controls

| File | Control | Behavior | Disabled-State Evidence |
| --- | --- | --- | --- |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Checkbox("Opt In")` | Calls `AnalyticsPanel::setOptIn`, updates dispatcher/privacy state, records action result. | `actionDetails.setOptIn` disables the checkbox with `No analytics dispatcher is bound.` |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Button("Clear Queue")` | Calls `AnalyticsPanel::clearQueuedEvents`, clears queued dispatcher events when bound. | `actionDetails.clearQueue` exposes `No analytics dispatcher is bound.` or `No queued analytics events to clear.` |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Button("Apply Endpoint Profile")` | Calls `AnalyticsPanel::applyEndpointProfile`, applying the reviewed endpoint profile to the uploader. | `actionDetails.applyEndpointProfile` exposes missing uploader or missing endpoint-profile reasons. |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Button("Flush Upload")` | Calls `AnalyticsPanel::flushQueuedEvents`, exporting queued events through the configured local JSONL uploader. | `actionDetails.flushUpload` and `disabledUploadMessage` expose missing dispatcher/uploader/handler, denied consent, opt-out, or empty queue. |
| `editor/ui/menu_inspector_panel.cpp` | `ImGui::Selectable(...)` | Calls `MenuInspectorModel::SelectRow` and refreshes the render snapshot. | Selection exists only for visible audit rows; empty state is displayed as panel data rather than a disabled command. |
| `editor/ability/pattern_field_panel.cpp` | `ImGui::Button("Clear")` | Calls `PatternFieldPanel::clearPattern`, mutating the current pattern. | Disabled ImGui state and `RenderSnapshot::controls` expose `No selected pattern points to clear.` |
| `editor/ability/pattern_field_panel.cpp` | Preset `ImGui::Button(...)` | Calls `PatternFieldPanel::applyPreset`, mutating the current pattern. | `RenderSnapshot::controls` exposes `No pattern presets are available.` when disabled. |
| `editor/ability/pattern_field_panel.cpp` | Grid-cell `ImGui::Button(...)` | Calls `PatternFieldPanel::togglePoint`, mutating the current pattern. | Disabled ImGui state and `RenderSnapshot::controls` expose `No pattern is loaded.` |
| `editor/ability/ability_inspector_panel.cpp` | Ability `ImGui::Selectable(...)` | Calls `AbilityInspectorModel::selectAbility` and updates selected-ability snapshot fields. | Select control exposes `No abilities are bound to this runtime.` when disabled. |
| `editor/ability/ability_inspector_panel.cpp` | `Preview` button | Calls host-provided `preview_selected` callback. | Disabled tooltip/snapshot reason is `Select an ability before previewing.` or missing host handler. |
| `editor/ability/ability_inspector_panel.cpp` | `Validate` button | Calls `validateDraftPattern`, updating validation issues in the snapshot. | Disabled tooltip/snapshot reason is `Create or load a draft ability before validation.` |
| `editor/ability/ability_inspector_panel.cpp` | `Apply` button | Calls host-provided `apply_draft_to_runtime` callback. | Disabled tooltip/snapshot reason is `Editor host has not registered an apply-draft command handler.` |
| `editor/ability/ability_inspector_panel.cpp` | `Save` button | Calls host-provided `save_draft` callback. | Disabled tooltip/snapshot reason is `Editor host has not registered a save-draft command handler.` |
| `editor/ability/ability_inspector_panel.cpp` | `Load` button | Calls host-provided `load_draft` callback. | Disabled tooltip/snapshot reason is `Editor host has not registered a load-draft command handler.` |
| `editor/ai/ai_assistant_panel.cpp` | AI step `Approve` action | Calls `AiAssistantPanel::approveStep(stepId)` and moves the selected tool step from `needs_review` to `approved`. | `controls.step_controls[].approve_button.enabled` is false when the step is already approved or rejected, or when the tool does not require approval. |
| `editor/ai/ai_assistant_panel.cpp` | AI step `Reject` action | Calls `AiAssistantPanel::rejectStep(stepId)` and marks the selected tool step as rejected so it cannot be applied. | `controls.step_controls[].reject_button.enabled` is false after the step is already rejected, or when the tool does not require approval. |
| `editor/ai/ai_assistant_panel.cpp` | `Approve All` action | Calls `AiAssistantPanel::approveAllPendingSteps()` and approves all pending mutating steps in the active plan. | `controls.approve_all_button.enabled` is false when the approval manifest has no pending steps. |
| `editor/ai/ai_assistant_panel.cpp` | `Apply` action | Calls `AiAssistantPanel::applyApprovedPlan()`, writes the approved plan into project JSON, and records `last_apply`, `project_patch`, and `revert_patch`. | `controls.apply_button.enabled` is false until the active plan validates with all mutating steps approved and no rejected steps. |
| `editor/ai/ai_assistant_panel.cpp` | `Revert AI Change` action | Calls `AiAssistantPanel::revertLastAppliedPlan()` and applies the latest reverse JSON Patch to project data. | `controls.revert_button.enabled` is false until at least one AI-applied project change is recorded. |

## Verification

- `rg -n "ImGui::(Button|MenuItem|Checkbox|Combo|Selectable|Slider|Drag)" editor`
- `build\dev-ninja-debug\urpg_tests.exe "[ability][editor]"`
- `build\dev-ninja-debug\urpg_tests.exe "[pattern][editor]"`
- `build\dev-ninja-debug\urpg_tests.exe "[analytics][editor][panel]"`
- `build\dev-ninja-debug\urpg_tests.exe "[ui][editor][menu_inspector][panel]"`
- `ctest --preset dev-all -R "Ability|Pattern|Analytics|MenuInspector" --output-on-failure`
- `ctest --preset dev-all -R "AI (knowledge|task|tool|assistant)|Chatbot component" --output-on-failure`
- `ctest --preset dev-all -R "settings|persistence|save|load|grid_part|Ability" --output-on-failure`
- `ctest --preset dev-all -R "startup|settings|input|SceneManager|RuntimeTitleScene" --output-on-failure`

Manual graphical verification in `urpg_editor` remains required for visual tooltip affordance, because headless tests validate the disabled-state contract but do not hover controls.
