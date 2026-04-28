# Editor Control Inventory

Status Date: 2026-04-28

This inventory records the P2-002 sweep of user-facing ImGui controls under `editor/`, plus the current AI assistant review controls exposed through deterministic render snapshots.

## Production Panel Exposure Map

`engine/core/editor/editor_panel_registry.cpp` is the source of truth for editor shell exposure.

| Exposure | Panel IDs | Release rationale |
| --- | --- | --- |
| `ReleaseTopLevel` | `diagnostics`, `assets`, `ability`, `patterns`, `mod`, `analytics` | Registered production navigation surfaces for the release shell and smoke workflow. |
| `Nested` | `compat_report`, `save_inspector`, `event_authority`, `message_inspector`, `battle_inspector`, `menu_inspector`, `menu_preview`, `audio_inspector`, `migration_wizard`, `project_audit`, `project_health` | Rendered as tabs or child surfaces inside the Diagnostics workspace. |
| `Nested` | `elevation_brush`, `terrain_brush`, `region_rules`, `procedural_map`, `prop_placement`, `map_ability_binding`, `spatial_ability_canvas` | Rendered through the incubating Spatial Authoring workspace rather than direct shell navigation. |
| `DevOnly` | `diagnostics_bundle`, `ai_assistant`, `local_review`, `mod_sdk`, `core_asset_browser`, `core_hierarchy`, `core_property_inspector` | Support, collaboration, SDK, or legacy core-editor tooling; compiled for developer workflows and excluded from release navigation. |
| `Deferred` | `event_authoring`, `plugin_inspector`, `new_project_wizard`, `quest`, `dialogue_graph`, `narrative_continuity`, `relationship`, `localization_workspace`, `timeline`, `replay`, `capture`, `photo_mode`, `database`, `balance`, `vendor`, `world`, `crafting`, `codex`, `calendar`, `npc`, `puzzle`, `export_diagnostics`, `character_creator`, `achievement`, `controller_binding`, `save_debugger`, `save_migration_preview`, `battle_presentation`, `boss_designer`, `formula_debugger`, `battle_preview`, `perf_diagnostics`, `spatial_authoring`, `sprite_animation_preview`, `accessibility`, `accessibility_assistant`, `audio_mix`, `input_remap`, `device_profile`, `theme_builder` | Compiled panels retained for direct tests, snapshots, or roadmap work; each registry entry documents the workflow or promotion gate required before release navigation. |

## Search Command

```powershell
rg -n "ImGui::(Button|MenuItem|Checkbox|Combo|Selectable|Slider|Drag)" editor
```

## Controls

| File | Control | Behavior | Disabled-State Evidence |
| --- | --- | --- | --- |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Checkbox("Opt In")` | Calls `AnalyticsPanel::setOptIn`, updates dispatcher/privacy state, records action result. | If dispatcher is missing, action returns false and snapshot records `No analytics dispatcher is bound.` |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Button("Clear Queue")` | Calls `AnalyticsPanel::clearQueuedEvents`, clears queued dispatcher events when bound. | If dispatcher is missing, action returns false-equivalent count and snapshot records `No analytics dispatcher is bound.` |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Button("Flush Upload")` | Calls `AnalyticsPanel::flushQueuedEvents`, exporting queued events through the configured local JSONL uploader. | Snapshot exposes `disabledUploadMessage` for missing uploader, missing handler, denied consent, opt-out, or empty queue. |
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

Manual graphical verification in `urpg_editor` remains required for visual tooltip affordance, because headless tests validate the disabled-state contract but do not hover controls.
