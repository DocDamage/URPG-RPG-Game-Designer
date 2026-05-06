# URPG Maker Shell UI/UX Plan

Status date: 2026-05-05

This plan merges the Mario Maker-style shell migration direction with the current game-maker asset browser and onboarding plan. It is written against the active asset-ingestion branch, not the older `main` scan.

## Decision

Implement the Maker Shell, but do it as an adapter-first migration over the current editor contracts.

The product direction is correct: URPG Maker needs to stop feeling like disconnected windows and become a cohesive, canvas-first maker. The implementation must preserve the existing release navigation, deterministic snapshots, headless validation, Level Builder source-of-truth model, and asset-governance rules.

## Current Baseline

- The canonical map authoring surface is `level_builder`.
- `GridPartDocument` remains the editable map source of truth.
- Current release top-level panels remain:
  - `diagnostics`
  - `assets`
  - `ability`
  - `patterns`
  - `mod`
  - `analytics`
  - `level_builder`
- Default startup uses `content/part_catalogs/base_jrpg_parts.json`.
- The full generated game-maker catalog is opt-in through `content/part_catalogs/game_maker_all_parts.json`.
- UI theme metadata now distinguishes `game_ui_theme` from `editor_theme`.
- `complete_ui_essential_flat` is game-UI-ready.
- `wenrexa_hologram` remains candidate-only until missing bar/progress assets or equivalent mapping exists.

## Product Target

URPG Maker should open into a real creator flow:

1. Main menu.
2. Continue/Open/New Project.
3. Adaptive onboarding for new projects.
4. Template-scoped project setup.
5. Canvas-first Maker Shell.
6. Left-side asset browser by default.
7. Top part belt for frequent Level Builder parts.
8. Side rails for context, inspector, assets, diagnostics, package, and All Tools.
9. Obvious Playtest/Save/Package loop.
10. Full library available on demand without loading the whole library at startup.

The result should feel inspired by maker tools, not copied from any specific commercial product. URPG must use original names, icons, sounds, art direction, and layout details.

## Non-Negotiable Contracts

- Do not introduce a second map document model.
- Do not promote deferred/dev panels into production top-level navigation just because they appear in All Tools.
- Keep `--list-panels`, `--open-panel`, and `--smoke` working.
- Add `--list-surfaces`, `--open-surface`, and UI-document dump commands only as additive APIs.
- Default startup must stay bounded and must not load the full generated asset library.
- Every ready claim needs tests, snapshots or exports, and docs.
- Every disabled/deferred/dev-only surface needs an explicit reason.

## Architecture

Current editor flow:

```text
EditorShell -> panel/workspace -> direct ImGui render + deterministic snapshots
```

Target migration flow:

```text
EditorShell
  -> EditorSurfaceRuntime
      -> existing panel/workspace snapshots and actions
      -> SurfaceManifest
      -> CommandManifest
      -> UiDocument
          -> Headless JSON
          -> ImGui Maker backend
```

The first migrated surface should be Level Builder, but only after the surface manifest, command manifest, UI document, route aliases, onboarding, and asset-browser foundations exist.

## Maker Shell Zones

| Zone | Purpose | Initial data source |
| --- | --- | --- |
| Main menu | Continue, New, Open, Recent, Pinned, Settings | project/session settings |
| Onboarding wizard | Game type, template, mechanics, world size, asset scope | template manifests |
| Top part belt | Fast part selection | active `GridPartCatalog` scope |
| Left browser | Folders, categories, assets, templates, previews | asset index and project scope |
| Main canvas | Grid painting, selection, preview, playtest entry | `LevelBuilderWorkspace` |
| Right rail | Inspector, diagnostics, assets, package, All Tools | snapshots and surface manifest |
| Bottom rail | Playtest, save, dirty state, blockers, coords, zoom | Level Builder snapshot |
| All Tools | Truthful card for every release/nested/dev/deferred/showcase surface | registry + app factories |

## Real Grid-Part Category Mapping

The top belt must start from real catalog categories.

| Maker label | Grid part categories |
| --- | --- |
| Terrain | `Tile`, `Hazard` |
| Walls | `Wall` |
| Structure | `Platform`, `Door`, `LevelBlock` |
| Props | `Prop`, `SavePoint`, `Shop` |
| Actors | `Npc` |
| Enemies | `Enemy` |
| Rewards | `TreasureChest`, `QuestItem` |
| Events | `Trigger`, `CutsceneZone` |
| Search | all categories |

Future categories such as Dialogue, Quests, Battle, Audio, UI, and Templates should appear as All Tools/Search cards until they have real placeable canvas data.

## Surface Manifest

Every editor surface should eventually have a generated record:

```json
{
  "schema": "urpg.editor_surface_manifest.v1",
  "id": "level_builder",
  "panelId": "level_builder",
  "route": "/maker/level_builder",
  "legacyOpenPanelId": "level_builder",
  "title": "Level Builder",
  "category": "Spatial",
  "exposure": "ReleaseTopLevel",
  "makerZone": "canvas",
  "status": "ready",
  "releaseVisible": true,
  "devVisible": true,
  "disabledReason": "",
  "promotionGate": "",
  "factoryStatus": "top_level_factory",
  "snapshotSource": "LevelBuilderWorkspace::lastRenderSnapshot",
  "commandSource": "LevelBuilderWorkspace::ActivateToolbarAction"
}
```

## Command Manifest

Commands should be exported as stable data before major visual rewrites. Level Builder command records come first and should map to existing toolbar/action IDs, selection, canvas actions, save/load/export, playtest, package, diagnostics focus, and readiness evidence commands.

Each command needs:

- `commandId`
- label
- surface/route
- zone
- enabled state
- disabled reason
- accessibility label
- handler source
- test evidence

## UI Document V1

Use a small backend-neutral document first:

```cpp
enum class UiNodeKind {
    Surface,
    Canvas,
    TopBelt,
    PartCard,
    Drawer,
    Rail,
    Card,
    Button,
    Toggle,
    SearchBox,
    InspectorField,
    StatusChip,
    ValidationList,
    Minimap,
    ContextBubble,
    Text,
    Section,
};
```

Do not build a full layout solver first. The v1 target is stable node IDs, command bindings, enabled/disabled state, accessibility labels, headless JSON, and ImGui rendering through one backend adapter.

## Implementation Order

### Phase 0: Docs And Invariants

- Create this plan.
- Create `docs/ui/MAKER_UI_VISUAL_LANGUAGE.md`.
- Link the plan from `README.md`, `docs/agent/INDEX.md`, and `docs/agent/KNOWN_DEBT.md`.
- Confirm current release top-level panels stay unchanged.

### Phase 1: Asset And Template Foundations

Use `docs/superpowers/plans/2026-05-05-game-maker-asset-browser-onboarding-plan.md` as the implementation plan for this phase.

- Runtime template profiles now emit Maker Shell setup metadata through `subsystems.template_runtime`: onboarding question profile, small default world size, recommended mechanics, bounded default asset catalogs, opt-in lazy full-library catalogs, game-UI theme eligibility, and a reserved community-template slot.
- `asset_library_index` and `game_template_manifest` schemas now exist, with tests for asset preview metadata and seven bounded onboarding starter manifests under `content/templates/game_maker/`.
- `GridPartCatalogScope` now provides the first backend primitive for switching active catalog scopes without retaining inactive full-library rows.
- Add `asset_library_index` schema/tooling.
- Add `game_template_manifest` schema and starter manifests.
- Add template-scoped catalog loading.
- Add lazy load/unload tests.

### Phase 2: Main Menu And Onboarding

- Add a real main menu as the first user-facing screen.
- Add adaptive onboarding wizard.
- Support last 10 recent projects, pinned projects, missing-project locate prompts, and hidden missing projects.
- Add settings toggles for onboarding/help tips and browser layout.

### Phase 3: Surface Manifest And Routes

- Generate surface manifest from `editorPanelRegistry()` and app factory IDs.
- Add `--list-surfaces`.
- Add `--dump-surface-manifest`.
- Add additive route aliases with `--open-surface`.
- Keep `--open-panel` working.

### Phase 4: Command Manifest

- Add Level Builder command manifest.
- Add command manifest rows for the other six release top-level panels.
- Add `--list-commands` and `--dump-command-manifest`.

### Phase 5: UI Document And Headless JSON

- Add `UiDocument`, `UiNode`, and JSON writer.
- Add `LevelBuilderUiDocumentBuilder`.
- Add `--dump-ui-document /maker/level_builder <path>`.
- Keep legacy ImGui rendering unchanged while this lands.

### Phase 6: Asset Browser And Preview Drawer

- Build the left-side collapsible browser.
- Support folder/category search, preview metadata, pinned/favorites, template scope, and full-library opt-in.
- Preview images, spritesheets, UI theme pieces, audio/font metadata, manifests, and unknown files.

### Phase 7: ImGui Maker Backend

- Render UI documents through ImGui behind a feature flag.
- Only migrated surfaces should use the Maker backend.
- Legacy panels may keep direct ImGui until migrated.

### Phase 8: Level Builder Maker Shell

- Replace the Level Builder toolbar/layout under the feature flag.
- Use top belt, left browser, canvas, right rail, bottom rail, and context bubble.
- Preserve all existing commands, diagnostics, save/load/export/playtest/package behavior.

### Phase 9: All Tools And Search

- Build All Tools from the surface manifest.
- Include release, nested, dev-only, deferred, template, and showcase cards.
- Dev-only cards are gated by dev mode.
- Deferred cards show promotion gates.

### Phase 10: Release Drawers

Migrate the remaining release panels as Maker Shell drawers/cards:

1. Diagnostics.
2. Assets.
3. Ability.
4. Patterns.
5. Mod.
6. Analytics.

### Phase 11: Nested Owner Routes

Migrate nested diagnostics and supporting spatial tools through owner routes. They should be discoverable, but not promoted to top-level release navigation without explicit registry/test/doc changes.

### Phase 12: Accessibility And Input Parity

- Add focus graph.
- Add keyboard/controller command hints.
- Add touch target metadata.
- Add reduced-motion and high-contrast tokens.
- Require accessibility labels for every visible command.

### Phase 13: Backend Experiments

Only after the ImGui Maker backend and headless UI document paths are stable, evaluate Slint, Qt/QML, RmlUi, or custom runtime UI experiments.

## Initial File Targets

```text
docs/ui/
  URPG_MAKER_SHELL_UIUX_PLAN.md
  MAKER_UI_VISUAL_LANGUAGE.md

editor/ui_kit/
  ui_document.*
  ui_node.*
  ui_command.*
  ui_surface_manifest.*
  ui_surface_router.*
  ui_document_json.*

editor/ui_kit/level_builder/
  level_builder_command_manifest.*
  level_builder_ui_document_builder.*
  level_builder_part_category_mapper.*

editor/ui_backends/imgui/
  imgui_maker_backend.*

editor/project/
  main_menu_panel.*
  onboarding_wizard.*

tools/editor/
  generate_editor_surface_manifest.py
  generate_editor_command_manifest.py
```

Every new `.cpp` must be added to `CMakeLists.txt`.

## Testing

Keep these passing:

```powershell
ctest --test-dir build\dev-ninja-debug -R "Editor panel registry|editor app panels|urpg_editor_smoke|urpg_editor_list_panels" --output-on-failure
ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library]"
.\build\dev-ninja-debug\urpg_tests.exe "[theme]"
```

Add focused suites as phases land:

```text
tests/unit/test_editor_surface_manifest.cpp
tests/unit/test_editor_command_manifest.cpp
tests/unit/test_ui_document.cpp
tests/unit/test_editor_surface_router.cpp
tests/unit/test_level_builder_maker_shell.cpp
tests/unit/test_project_main_menu.cpp
tests/unit/test_project_onboarding.cpp
tests/unit/test_asset_library_index.cpp or Python equivalent
```

## Definition Of Done

- App starts at a real main menu.
- New projects go through adaptive onboarding unless disabled.
- Templates load bounded starter catalogs.
- Full library is opt-in and unloads when leaving full-library scope.
- Level Builder opens as the Maker Shell by default.
- All existing Level Builder actions still work and are tested.
- Top belt, left browser, canvas, side rails, bottom playtest rail, and All Tools are present.
- Every registry entry has a truthful surface card.
- Every route opens or reports an explicit reason.
- Dev-only surfaces do not appear in production navigation.
- Migrated surfaces export headless UI documents.
- Existing release, grid-part, asset, theme, and editor smoke gates pass.
