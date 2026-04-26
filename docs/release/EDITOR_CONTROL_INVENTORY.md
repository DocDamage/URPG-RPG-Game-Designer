# Editor Control Inventory

Status Date: 2026-04-26

This inventory records the P2-002 sweep of user-facing ImGui controls under `editor/`.

## Search Command

```powershell
rg -n "ImGui::(Button|MenuItem|Checkbox|Combo|Selectable|Slider|Drag)" editor
```

## Controls

| File | Control | Behavior | Disabled-State Evidence |
| --- | --- | --- | --- |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Checkbox("Opt In")` | Calls `AnalyticsPanel::setOptIn`, updates dispatcher/privacy state, records action result. | If dispatcher is missing, action returns false and snapshot records `No analytics dispatcher is bound.` |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Button("Clear Queue")` | Calls `AnalyticsPanel::clearQueuedEvents`, clears queued dispatcher events when bound. | If dispatcher is missing, action returns false-equivalent count and snapshot records `No analytics dispatcher is bound.` |
| `editor/analytics/analytics_panel.cpp` | `ImGui::Button("Flush Upload")` | Calls `AnalyticsPanel::flushQueuedEvents`, uploads queued events through configured uploader. | Snapshot exposes `disabledUploadMessage` for missing uploader, missing handler, denied consent, opt-out, or empty queue. |
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

## Verification

- `rg -n "ImGui::(Button|MenuItem|Checkbox|Combo|Selectable|Slider|Drag)" editor`
- `build\dev-ninja-debug\urpg_tests.exe "[ability][editor]"`
- `build\dev-ninja-debug\urpg_tests.exe "[pattern][editor]"`
- `build\dev-ninja-debug\urpg_tests.exe "[analytics][editor][panel]"`
- `build\dev-ninja-debug\urpg_tests.exe "[ui][editor][menu_inspector][panel]"`
- `ctest --preset dev-all -R "Ability|Pattern|Analytics|MenuInspector" --output-on-failure`

Manual graphical verification in `urpg_editor` remains required for visual tooltip affordance, because headless tests validate the disabled-state contract but do not hover controls.
