# URPG Grid-Part Level Builder — Master Implementation Plan v4

This is the consolidated production plan for adding a Super Mario Maker-style grid-part level builder to URPG without breaking the current project.

It merges and upgrades the earlier plans:

```text
v1: Base grid-part authoring idea
v2: Core implementation plan with code examples, validation, runtime compiler, undo/redo, editor panels
v3: Production addendum with rulesets, objectives, reachability, dependencies, playtest, packaging, security
v4: Master execution package with coding order, tickets, acceptance gates, risk register, migration plan, and agent prompts
```

---

# 0. Executive Decision

Build it.

Do **not** rewrite URPG.

Implement a new additive system:

```text
Grid Part Authoring Layer
        ↓
Validation / Rulesets / Objectives / Dependencies
        ↓
Runtime Compiler
        ↓
Existing URPG runtime systems
```

Existing systems become compile targets:

```text
MapScene
TileLayerDocument
SpatialMapOverlay
SpawnTable
MapRegionRule
LevelAssemblyWorkspace
InteractionAbilityBinding
Save/runtime state
```

The major product goal:

```text
URPG should let a creator place tiles, walls, hazards, doors, NPCs, enemies, chests, save points, triggers, cutscene zones, shops, quest items, props, and modular rooms on a grid; validate the map; instantly playtest it; and export/share it safely.
```

---

# 1. Current Repo Reality

URPG already has a lot of the foundation.

## 1.1 Existing systems to reuse

```text
engine/core/scene/map_scene.h/.cpp
- Native map runtime
- Grid size
- tile passability
- collision checks
- movement
- tile/prop/region/global ability bindings

engine/core/scene/map_loader.h
- RPG Maker MZ map import
- map layers
- tileset collision flags
- passability conversion into native MapScene

engine/core/map/tile_layer_document.h/.cpp
- editable tile document
- multiple layers
- locked layers
- collision/navigation flags
- navigation diagnostics

engine/core/map/terrain_brush.h/.cpp
- single/rectangle/line/random/stamp/autotile brush preview

engine/core/map/map_region_rules.h/.cpp
- rectangular regions
- encounter table
- ambient audio
- weather
- hazard
- movement rule
- event id

engine/core/map/spawn_table.h/.cpp
- enemy spawn entries
- JSON round-trip
- missing enemy validation

engine/core/level/level_assembly.h/.cpp
- modular level blocks
- snap connectors
- block placement
- occupied-cell validation
- library thumbnails

engine/core/presentation/presentation_schema.h
- SpatialMapOverlay
- ElevationGrid
- PropInstance
- Light/Fog/PostFX

editor/spatial/prop_placement_panel.h/.cpp
- screen-to-ground projection
- prop placement
- selected asset snapshot

editor/spatial/map_ability_binding_panel.h/.cpp
- ability binding to global/tile/prop/region
- prop picking
- painted regions
- trigger switching/rebinding

editor/spatial/spatial_ability_canvas_panel.h/.cpp
- hover previews
- selection
- conflicts
- badges
- trigger menu

editor/spatial/spatial_authoring_workspace.h/.cpp
- unified spatial toolbar
- modes: composite/elevation/props/abilities
- canvas routing
```

## 1.2 The core missing abstraction

URPG does **not** yet have one unified placed-part document model.

Current pieces are split:

```text
tiles               -> MapScene / TileLayerDocument / MZ MapLoader
props               -> SpatialMapOverlay.props
enemy spawns         -> SpawnTable
regions/hazards      -> MapRegionRule
modular rooms        -> LevelAssemblyWorkspace
tile/prop abilities  -> MapScene interaction bindings
```

The missing layer:

```text
GridPartDocument
```

This becomes the editor/source-of-truth format for creator-built maps.

---

# 2. Target Architecture

## 2.1 Main data flow

```text
GridPartCatalog
        +
GridPartDocument
        +
MapRulesetProfile
        +
MapObjective
        ↓
GridPartValidator
        ↓
GridPartDependencyGraph
        ↓
GridPartRuntimeCompiler
        ↓
Compiled Runtime Outputs:
    TileLayerDocument
    SpatialMapOverlay
    SpawnTable
    MapRegionRule[]
    LevelAssemblyWorkspace changes
    InteractionAbilityBinding[]
    GridPartRuntimeState defaults
        ↓
MapScene / Runtime / Playtest / Export
```

## 2.2 Subsystems to create

```text
engine/core/map/grid_part_types.h
engine/core/map/grid_part_document.h/.cpp
engine/core/map/grid_part_catalog.h/.cpp
engine/core/map/grid_part_validator.h/.cpp
engine/core/map/grid_part_dependency_graph.h/.cpp
engine/core/map/grid_part_runtime_compiler.h/.cpp
engine/core/map/grid_part_runtime_state.h/.cpp
engine/core/map/grid_part_commands.h/.cpp
engine/core/map/grid_part_ruleset.h/.cpp
engine/core/map/grid_part_objective.h/.cpp
engine/core/map/grid_part_reachability.h/.cpp
engine/core/map/grid_part_stamp.h/.cpp
engine/core/map/grid_part_serializer.h/.cpp
engine/core/map/grid_part_migration.h/.cpp
```

Editor:

```text
editor/spatial/grid_part_palette_panel.h/.cpp
editor/spatial/grid_part_placement_panel.h/.cpp
editor/spatial/grid_part_inspector_panel.h/.cpp
editor/spatial/grid_part_diagnostics_panel.h/.cpp
editor/spatial/grid_part_playtest_panel.h/.cpp
```

Schemas:

```text
content/schemas/grid_part_authoring.schema.json
content/schemas/grid_part_catalog.schema.json
content/schemas/grid_part_runtime_state.schema.json
content/schemas/grid_part_ruleset.schema.json
content/schemas/grid_part_objective.schema.json
content/schemas/grid_part_stamp.schema.json
```

Content:

```text
content/part_catalogs/base_jrpg_parts.json
content/part_catalogs/base_dungeon_parts.json
content/part_catalogs/base_side_scroller_parts.json
content/part_catalogs/base_tactical_parts.json
content/rulesets/top_down_jrpg.json
content/rulesets/dungeon_room_builder.json
content/rulesets/side_scroller_action.json
```

Tests:

```text
tests/unit/test_grid_part_document.cpp
tests/unit/test_grid_part_catalog.cpp
tests/unit/test_grid_part_validator.cpp
tests/unit/test_grid_part_dependency_graph.cpp
tests/unit/test_grid_part_runtime_compiler.cpp
tests/unit/test_grid_part_runtime_state.cpp
tests/unit/test_grid_part_commands.cpp
tests/unit/test_grid_part_ruleset.cpp
tests/unit/test_grid_part_objective.cpp
tests/unit/test_grid_part_reachability.cpp
tests/unit/test_grid_part_stamp.cpp
tests/unit/test_grid_part_editor.cpp
tests/integration/test_grid_part_map_scene_integration.cpp
tests/integration/test_grid_part_playtest_pipeline.cpp
```

---

# 3. Non-Negotiable Rules

## 3.1 Do not break existing map loading

Keep:

```text
MapLoader
MZ data import
MapScene
TilemapRenderer
TileLayerDocument
SpatialMapOverlay
```

The new layer must be optional.

Valid map modes:

```text
1. Pure MZ imported map
2. Pure URPG grid-part map
3. MZ base map + grid-part overlay
4. Procedural map + grid-part editing
5. LevelBlock modular map + grid-part dressing
```

## 3.2 Every placed object needs stable identity

Never rely on:

```text
assetId only
coordinates only
enemyId only
npcId only
tileId only
```

Use:

```text
instanceId
```

Format:

```text
<mapId>:<partId>:<x>:<y>[:suffix]
```

Example:

```text
001:chest.basic:10:4
001:enemy.slime:8:6
001:zone.cutscene:2:12
```

## 3.3 Runtime state uses instanceId

Correct:

```text
chest opened state -> instanceId
quest item collected -> instanceId
door unlocked -> instanceId
trigger fired -> instanceId
enemy defeated -> instanceId
```

Wrong:

```text
save by x/y only
save by assetId only
```

## 3.4 Editor uses command history

Every destructive edit should be undoable.

Required commands:

```text
PlacePartCommand
RemovePartCommand
MovePartCommand
ResizePartCommand
ReplacePartCommand
BulkGridPartCommand
ChangePartPropertyCommand
```

## 3.5 Validation before publishing

Draft save can allow errors.

Publish/export cannot.

---

# 4. Core Types

## 4.1 `grid_part_types.h`

```cpp
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::map {

enum class GridPartCategory : uint8_t {
    Tile = 0,
    Wall,
    Platform,
    Hazard,
    Door,
    Npc,
    Enemy,
    TreasureChest,
    SavePoint,
    Trigger,
    CutsceneZone,
    Shop,
    QuestItem,
    Prop,
    LevelBlock
};

enum class GridPartLayer : uint8_t {
    Terrain = 0,
    Decoration,
    Collision,
    Object,
    Actor,
    Trigger,
    Region,
    Overlay
};

enum class GridPartCollisionPolicy : uint8_t {
    None = 0,
    Solid,
    Hazard,
    TriggerOnly,
    Custom
};

enum class GridPartSeverity : uint8_t {
    Info = 0,
    Warning,
    Error,
    Blocker
};

enum class GridPartRuleset : uint8_t {
    TopDownJRPG = 0,
    SideScrollerAction,
    TacticalGrid,
    DungeonRoomBuilder,
    WorldMap,
    TownHub,
    BattleArena,
    CutsceneStage
};

struct GridPartFootprint {
    int32_t width = 1;
    int32_t height = 1;
    bool allow_overlap = false;
    bool blocks_navigation = false;
};

struct GridPartDefinition {
    std::string part_id;
    std::string display_name;
    std::string description;

    GridPartCategory category = GridPartCategory::Prop;
    GridPartLayer default_layer = GridPartLayer::Object;
    GridPartCollisionPolicy collision_policy = GridPartCollisionPolicy::None;

    std::vector<GridPartRuleset> supported_rulesets;

    GridPartFootprint footprint;

    std::string asset_id;
    std::string prefab_path;
    int32_t tile_id = 0;

    std::vector<std::string> tags;

    std::unordered_map<std::string, std::string> default_properties;
};

struct PlacedPartInstance {
    std::string instance_id;
    std::string part_id;

    GridPartCategory category = GridPartCategory::Prop;
    GridPartLayer layer = GridPartLayer::Object;

    int32_t grid_x = 0;
    int32_t grid_y = 0;
    int32_t grid_z = 0;

    int32_t width = 1;
    int32_t height = 1;

    float rot_y = 0.0f;
    float scale = 1.0f;

    bool locked = false;
    bool hidden = false;

    std::unordered_map<std::string, std::string> properties;
};

struct GridPartDiagnostic {
    GridPartSeverity severity = GridPartSeverity::Error;
    std::string code;
    std::string message;
    std::string instance_id;
    std::string part_id;
    int32_t x = -1;
    int32_t y = -1;
    std::string target;
};

} // namespace urpg::map
```

---

# 5. GridPartDocument

## 5.1 Required API

```cpp
class GridPartDocument {
public:
    GridPartDocument() = default;
    GridPartDocument(std::string map_id, int32_t width, int32_t height);

    const std::string& mapId() const;
    int32_t width() const;
    int32_t height() const;

    bool placePart(const PlacedPartInstance& instance);
    bool replacePart(const PlacedPartInstance& instance);
    bool removePart(const std::string& instance_id);
    bool movePart(const std::string& instance_id, int32_t next_x, int32_t next_y, int32_t next_z = 0);
    bool resizePart(const std::string& instance_id, int32_t next_width, int32_t next_height);

    PlacedPartInstance* findPartMutable(const std::string& instance_id);
    const PlacedPartInstance* findPart(const std::string& instance_id) const;

    std::vector<const PlacedPartInstance*> partsAt(int32_t x, int32_t y) const;
    std::vector<const PlacedPartInstance*> partsAtLayer(int32_t x, int32_t y, GridPartLayer layer) const;

    bool inBounds(int32_t x, int32_t y) const;
    bool footprintInBounds(const PlacedPartInstance& instance) const;
    bool hasInstanceId(const std::string& instance_id) const;

    const std::vector<PlacedPartInstance>& parts() const;

private:
    std::string map_id_;
    int32_t width_ = 0;
    int32_t height_ = 0;
    std::vector<PlacedPartInstance> parts_;
};
```

## 5.2 Acceptance tests

```text
places unique part
rejects missing instance ID
rejects missing part ID
rejects duplicate instance ID
rejects out-of-bounds footprint
moves part
rejects moving locked part
resizes part
rejects invalid resize
finds parts at coordinate
finds parts at coordinate/layer
removes part
```

---

# 6. GridPartCatalog

## 6.1 Required API

```cpp
class GridPartCatalog {
public:
    bool addDefinition(GridPartDefinition definition);
    const GridPartDefinition* find(const std::string& part_id) const;

    std::vector<GridPartDefinition> allDefinitions() const;
    std::vector<GridPartDefinition> filterByCategory(GridPartCategory category) const;
    std::vector<GridPartDefinition> filterByRuleset(GridPartRuleset ruleset) const;
    std::vector<GridPartDefinition> search(const std::string& query) const;

    size_t size() const;

private:
    std::unordered_map<std::string, GridPartDefinition> definitions_;
};
```

## 6.2 Deterministic ordering

All returned vectors must sort by:

```text
part_id
```

That keeps tests and Git diffs stable.

---

# 7. Ruleset System

## 7.1 Ruleset profile

```cpp
struct GridRulesetProfile {
    GridPartRuleset ruleset = GridPartRuleset::TopDownJRPG;
    std::string id;
    std::string display_name;

    bool requires_player_spawn = true;
    bool requires_exit = false;
    bool allows_gravity = false;
    bool allows_platforms = false;
    bool allows_freeform_props = true;
    bool uses_tile_passability = true;
    bool uses_physics_collision = false;
    bool uses_tactical_cover = false;
    bool allows_multiple_object_layers = true;

    int32_t default_tile_size = 48;
    int32_t max_width = 256;
    int32_t max_height = 256;
};
```

## 7.2 Validation diagnostics

```text
ruleset_missing
part_ruleset_incompatible
map_exceeds_ruleset_size
ruleset_requires_spawn
ruleset_requires_exit
platform_not_allowed_in_ruleset
gravity_part_not_allowed_in_ruleset
```

---

# 8. Objective System

## 8.1 Objective type

```cpp
enum class MapObjectiveType : uint8_t {
    None = 0,
    ReachExit,
    DefeatBoss,
    DefeatAllEnemies,
    CollectQuestItem,
    TalkToNpc,
    TriggerEvent,
    SurviveWaves,
    SolvePuzzle,
    OpenChest,
    ReachRegion
};

struct MapObjective {
    MapObjectiveType type = MapObjectiveType::None;
    std::string objective_id;
    std::string target_instance_id;
    std::string target_event_id;
    std::string required_flag;
    bool required_for_publish = true;
};
```

## 8.2 Diagnostics

```text
objective_missing
objective_target_missing
objective_target_wrong_category
objective_unreachable
objective_has_no_completion_signal
```

---

# 9. Reachability and Softlock Validation

## 9.1 Required checks

```text
player spawn exists
objective exists
exit exists when required
spawn reaches objective
spawn reaches exit
required key reaches locked door
required quest item reachable
save point reachable
boss clear condition exists
cutscene returns control
trigger does not permanently disable movement
solid objects do not isolate required areas
```

## 9.2 Minimal BFS API

```cpp
struct ReachabilityReport {
    bool ok = false;
    std::vector<GridPartDiagnostic> diagnostics;
    std::vector<std::pair<int32_t, int32_t>> reachable_cells;
};

ReachabilityReport ValidateReachability(
    const GridPartDocument& document,
    const GridPartCatalog& catalog,
    const GridRulesetProfile& ruleset,
    const MapObjective& objective
);
```

## 9.3 Door/key dependency diagnostics

```text
required_key_unreachable
door_key_cycle
door_target_unreachable_after_unlock
objective_dependency_cycle
```

---

# 10. Dependency Graph

## 10.1 Dependency type

```cpp
enum class GridPartDependencyType : uint8_t {
    Asset = 0,
    Enemy,
    Npc,
    Dialogue,
    ShopTable,
    LootTable,
    Quest,
    QuestItem,
    Cutscene,
    Timeline,
    Ability,
    Audio,
    Animation,
    Script,
    Tileset,
    Prefab
};

struct GridPartDependency {
    GridPartDependencyType type = GridPartDependencyType::Asset;
    std::string id;
    std::string source_instance_id;
    bool required = true;
};
```

## 10.2 API

```cpp
std::vector<GridPartDependency> CollectGridPartDependencies(
    const GridPartDocument& document,
    const GridPartCatalog& catalog
);
```

## 10.3 Diagnostics

```text
dependency_missing
dependency_optional_missing
dependency_duplicate
dependency_version_mismatch
dependency_license_blocked
```

---

# 11. Runtime Compiler

## 11.1 Output model

```cpp
struct GridPartRuntimeCompileResult {
    bool ok = true;
    std::vector<GridPartDiagnostic> diagnostics;

    TileLayerDocument tile_document;
    presentation::SpatialMapOverlay spatial_overlay;
    SpawnTable spawn_table;
    std::vector<MapRegionRule> region_rules;
};
```

## 11.2 Category compilation table

| Category | Compile target |
|---|---|
| Tile | `TileLayerDocument` terrain |
| Wall | terrain + collision |
| Platform | object/platform collision |
| Hazard | `MapRegionRule.hazard`, optional collision |
| Door | prop + lock/transition metadata + optional interaction binding |
| Npc | prop/entity metadata + dialogue/shop/quest fields |
| Enemy | `SpawnTable.entries` |
| TreasureChest | prop + loot metadata + runtime state |
| SavePoint | prop + save metadata + runtime state |
| Trigger | `MapRegionRule` or ability binding |
| CutsceneZone | `MapRegionRule.event_id` |
| Shop | NPC/prop + shop table dependency |
| QuestItem | prop + quest item runtime state |
| Prop | `SpatialMapOverlay.props` |
| LevelBlock | `LevelAssemblyWorkspace` / prefab expansion |

## 11.3 ApplyToMapScene rule

Compiler must update:

```text
scene.setLayerData(...)
scene.setTile(...)
scene.setTilePassable(...)
```

Do not only update visual layers.

---

# 12. Runtime State

## 12.1 State structure

```cpp
struct GridPartInstanceState {
    std::string instance_id;
    bool enabled = true;
    bool visible = true;
    bool consumed = false;
    std::unordered_map<std::string, std::string> state;
};

class GridPartRuntimeState {
public:
    GridPartInstanceState& getOrCreate(const std::string& instance_id);
    const GridPartInstanceState* find(const std::string& instance_id) const;

    bool setFlag(const std::string& instance_id, const std::string& key, const std::string& value);
    std::string getFlag(const std::string& instance_id, const std::string& key, const std::string& fallback = "") const;
};
```

## 12.2 Runtime state examples

```text
Chest:
state.opened = true

Quest item:
consumed = true
state.collected = true

Door:
state.locked = false
state.open = true

Enemy:
state.alive = false
state.respawn_at_tick = 12345

Trigger:
state.fired = true
```

---

# 13. Prop Instance Migration

## 13.1 Update PropInstance

```cpp
struct PropInstance {
    std::string instanceId;
    std::string assetId;
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
    float rotY = 0.0f;
    float scale = 1.0f;
};
```

## 13.2 Migration helper

```cpp
inline void EnsureStablePropInstanceIds(SpatialMapOverlay& overlay) {
    std::unordered_map<std::string, int32_t> counts;

    for (auto& prop : overlay.props) {
        if (!prop.instanceId.empty()) {
            continue;
        }

        const int32_t index = counts[prop.assetId]++;
        prop.instanceId = overlay.mapId + ":" + prop.assetId + ":" + std::to_string(index);
    }
}
```

## 13.3 Binding upgrade

Add to `InteractionAbilityBinding`:

```cpp
std::string prop_instance_id;
```

Keep:

```cpp
std::string prop_asset_id;
```

Legacy behavior:

```text
If prop_instance_id exists, use it.
Else use prop_asset_id fallback.
If fallback is ambiguous, emit diagnostic.
```

---

# 14. Undo/Redo

## 14.1 Required command interface

```cpp
class IGridPartCommand {
public:
    virtual ~IGridPartCommand() = default;
    virtual bool apply(GridPartDocument& document) = 0;
    virtual bool undo(GridPartDocument& document) = 0;
    virtual std::string label() const = 0;
};
```

## 14.2 Required commands

```text
PlacePartCommand
RemovePartCommand
MovePartCommand
ResizePartCommand
ReplacePartCommand
ChangePartPropertyCommand
BulkGridPartCommand
```

## 14.3 Atomic bulk behavior

If one operation fails:

```text
undo all applied operations
return false
do not pollute undo history
```

---

# 15. Editor Integration

## 15.1 Add Parts mode

Current:

```text
Composite
Elevation
Props
Abilities
```

Add:

```text
Parts
```

## 15.2 New editor panels

```text
GridPartPalettePanel
GridPartPlacementPanel
GridPartInspectorPanel
GridPartDiagnosticsPanel
GridPartPlaytestPanel
```

## 15.3 Editor snapshots

Each panel needs:

```cpp
const RenderSnapshot& lastRenderSnapshot() const;
```

Required placement snapshot:

```cpp
struct GridPartPlacementSnapshot {
    bool visible = true;
    bool has_document = false;
    bool has_catalog = false;
    bool has_spatial_overlay = false;

    std::string selected_part_id;

    bool hover_active = false;
    bool hover_valid = false;
    std::string hover_reason;

    int32_t hover_x = -1;
    int32_t hover_y = -1;
    int32_t footprint_width = 1;
    int32_t footprint_height = 1;

    size_t placed_count = 0;
    size_t diagnostic_count = 0;

    bool can_undo = false;
    bool can_redo = false;
};
```

---

# 16. Bulk Tools

Required:

```text
single place
rectangle fill
line draw
paint bucket
replace matching
select by category
select by tag
box select
duplicate selection
delete selection
move selection
mirror selection
rotate selection
stamp selection
lock selection
hide selection
```

Add these after the core placement panel is stable.

---

# 17. Stamps and Prefabs

## 17.1 Stamp

```cpp
struct GridPartStamp {
    std::string stamp_id;
    std::string display_name;
    std::vector<PlacedPartInstance> parts;

    int32_t width = 1;
    int32_t height = 1;
    std::vector<std::string> tags;
};
```

## 17.2 Stamp placement

Rules:

```text
rebase coordinates
generate fresh instance IDs
validate all parts
commit atomically
```

## 17.3 LevelBlock integration

Use existing:

```text
LevelBlock
LevelBlockLibrary
LevelAssemblyWorkspace
SnapConnector
```

Grid parts should not replace level blocks. Level blocks are for modular rooms. Stamps are for arbitrary part groups.

---

# 18. Playtest Pipeline

## 18.1 Required playtest actions

```text
Playtest From Start
Playtest From Here
Playtest Selected Room
Playtest Objective Path
Return To Editor
```

## 18.2 Flow

```text
save draft snapshot
validate
compile temp runtime map
launch playtest scene
track result
return to editor camera/selection
```

## 18.3 Playtest result

```cpp
struct GridPartPlaytestResult {
    std::string map_id;
    bool completed_objective = false;
    bool player_died = false;
    bool softlocked = false;

    float elapsed_seconds = 0.0f;

    int32_t start_x = 0;
    int32_t start_y = 0;
    int32_t end_x = 0;
    int32_t end_y = 0;

    std::vector<std::string> visited_instance_ids;
    std::vector<GridPartDiagnostic> diagnostics;
};
```

---

# 19. Chunking and Performance

## 19.1 Add chunk tracking

```cpp
struct GridPartChunkCoord {
    int32_t chunk_x = 0;
    int32_t chunk_y = 0;
};

struct GridPartChunk {
    GridPartChunkCoord coord;
    std::vector<std::string> instance_ids;
    bool dirty = true;
};
```

Default chunk size:

```text
16x16 for normal maps
32x32 for large maps
```

## 19.2 Dirty operations

Mark chunks dirty when:

```text
part placed
part deleted
part moved
part resized
layer changed
collision changed
visibility changed
property affecting compile changed
```

---

# 20. Export and Package Governance

## 20.1 Creator package permissions

```cpp
struct CreatorPackagePermissions {
    bool can_edit = true;
    bool can_remix = false;
    bool can_export = true;
    bool can_redistribute = false;
    bool requires_original_assets = true;
    bool contains_private_assets = false;
    bool contains_third_party_assets = false;
};
```

## 20.2 Export diagnostics

```text
package_contains_private_asset
package_missing_license_evidence
package_redistribution_forbidden
package_dependency_missing
package_uses_unapproved_asset
```

---

# 21. Accessibility Checks

Add validation for:

```text
flashing hazard without reduced-motion alternative
color-only puzzle clue
required audio cue without visual cue
tiny interact target
unlabeled NPC/shop/save point
timed challenge without assist setting
```

Diagnostics:

```text
accessibility_color_only_signal
accessibility_missing_visual_cue
accessibility_flashing_hazard
accessibility_timed_challenge_no_assist
```

---

# 22. Difficulty and Balance Budget

## 22.1 Budget structure

```cpp
struct MapDifficultyBudget {
    int32_t target_level = 1;
    int32_t max_enemy_budget = 100;
    int32_t max_hazard_budget = 30;
    int32_t max_required_damage_budget = 100;
    int32_t max_required_jump_precision = 0;
};
```

## 22.2 Diagnostics

```text
enemy_budget_exceeded
hazard_budget_exceeded
boss_level_mismatch
resource_starvation
```

---

# 23. Security and Plugin Safety

## 23.1 Script classification

```text
None
DataOnly
BuiltInAction
TrustedPlugin
UntrustedPlugin
ExternalScript
```

## 23.2 Rules

```text
scripts do not run during palette preview
scripts do not run during validation unless sandboxed
untrusted scripts cannot be exported silently
playtest uses runtime sandbox rules
```

Diagnostics:

```text
unsafe_script_dependency
script_not_allowed_in_ruleset
script_missing_signature
```

---

# 24. AI-Assisted Authoring Boundaries

AI may:

```text
suggest layouts
place draft parts
generate stamps
name maps
suggest objectives
run validation
repair diagnostics
```

AI may not silently:

```text
delete locked user work
use unlicensed assets
publish maps
override blockers
change permissions
```

AI proposal model:

```cpp
struct GridPartEditProposal {
    std::string proposal_id;
    std::string summary;
    std::vector<PlacedPartInstance> parts_to_add;
    std::vector<std::string> instance_ids_to_remove;
    std::vector<PlacedPartInstance> parts_to_replace;
    std::vector<GridPartDiagnostic> expected_resolved_diagnostics;
};
```

Flow:

```text
AI proposes
user previews diff
validation runs
user accepts/rejects
accepted proposal becomes undoable command
```

---

# 25. Diff/Merge Rules

Canonical JSON output must:

```text
sort parts by layer, y, x, instanceId
sort properties alphabetically
omit editor camera state from map content
avoid timestamps in deterministic content
use stable IDs
use stable schema field order
```

Merge diagnostics:

```text
merge_duplicate_instance_id
merge_deleted_part_has_runtime_state
merge_catalog_version_conflict
merge_schema_version_conflict
merge_same_cell_solid_conflict
```

---

# 26. Readiness Levels

```text
Draft
Playable
Validated
Publishable
Exportable
Certified
```

## 26.1 Draft

Can save even with errors.

## 26.2 Playable

Requires:

```text
player spawn
compiled runtime map
no fatal compiler errors
```

## 26.3 Validated

Requires:

```text
no blockers
objective exists
reachability passes
dependencies resolved
```

## 26.4 Publishable

Requires:

```text
validated
clear proof or automated reachability proof
no missing licenses
no ambiguous legacy prop bindings
```

## 26.5 Exportable

Requires:

```text
publishable
package dependencies complete
target export checks pass
```

## 26.6 Certified

Requires:

```text
exportable
accessibility checks pass or waived
performance budget pass
manual review if configured
```

---

# 27. Implementation Order

## Phase 1 — Core document

Create:

```text
grid_part_types.h
grid_part_document.h/.cpp
test_grid_part_document.cpp
```

No runtime changes.

## Phase 2 — Catalog

Create:

```text
grid_part_catalog.h/.cpp
grid_part_catalog.schema.json
base_jrpg_parts.json
test_grid_part_catalog.cpp
```

## Phase 3 — Command history

Create:

```text
grid_part_commands.h/.cpp
test_grid_part_commands.cpp
```

## Phase 4 — Validator

Create:

```text
grid_part_validator.h/.cpp
test_grid_part_validator.cpp
```

## Phase 5 — Runtime compiler

Create:

```text
grid_part_runtime_compiler.h/.cpp
test_grid_part_runtime_compiler.cpp
test_grid_part_map_scene_integration.cpp
```

## Phase 6 — Runtime state

Create:

```text
grid_part_runtime_state.h/.cpp
grid_part_runtime_state.schema.json
test_grid_part_runtime_state.cpp
```

## Phase 7 — Prop instance migration

Modify:

```text
presentation_schema.h
presentation_schema.json
prop_placement_panel.*
map_ability_binding_panel.*
map_scene.*
test_spatial_editor.cpp
```

## Phase 8 — Editor Parts mode

Create:

```text
grid_part_palette_panel.*
grid_part_placement_panel.*
grid_part_inspector_panel.*
test_grid_part_editor.cpp
```

Modify:

```text
spatial_authoring_workspace.*
```

## Phase 9 — Rulesets/objectives

Create:

```text
grid_part_ruleset.h/.cpp
grid_part_objective.h/.cpp
test_grid_part_ruleset.cpp
test_grid_part_objective.cpp
```

## Phase 10 — Reachability

Create:

```text
grid_part_reachability.h/.cpp
test_grid_part_reachability.cpp
```

## Phase 11 — Dependency graph

Create:

```text
grid_part_dependency_graph.h/.cpp
test_grid_part_dependency_graph.cpp
```

## Phase 12 — Playtest pipeline

Create:

```text
grid_part_playtest_panel.*
test_grid_part_playtest_pipeline.cpp
```

## Phase 13 — Stamps/bulk tools

Create:

```text
grid_part_stamp.h/.cpp
test_grid_part_stamp.cpp
```

Expand editor placement panel.

## Phase 14 — Chunking/performance

Add dirty chunk tracking and partial compile.

## Phase 15 — Export/package governance

Integrate package permissions, dependency manifest, license checks, readiness gates.

---

# 28. Definition of Done

A phase is not done unless:

```text
code compiles
CMakeLists updated
unit tests added
integration tests added if runtime-affecting
schemas updated if persisted data changed
SCHEMA_CHANGELOG updated if schema changed
old tests still pass
diagnostics are explicit
no silent fallback for dangerous ambiguity
no behavior hidden behind TODO claims
```

---

# 29. Risk Register

| Risk | Impact | Mitigation |
|---|---|---|
| Breaking old MZ maps | High | Keep GridPartDocument optional; do not replace MapLoader |
| Duplicate prop asset ambiguity | High | Add `instanceId`; legacy ambiguity diagnostic |
| Visual/collision mismatch | High | Compiler updates both renderer layers and MapScene passability |
| Giant PR becomes unreviewable | High | Use phase/PR slicing |
| UI built before model | Medium | Implement headless models/tests first |
| Save data breaks after map edit | High | Runtime state by `instanceId`, not coordinates |
| Export includes private assets | High | Dependency/provenance validation |
| Large maps slow down | Medium | Chunking and dirty compile |
| Softlocked maps get published | High | Objective + reachability validation |
| AI edits delete user work | Medium | AI proposals must be reviewed and command-based |
| Plugins execute unsafe scripts | High | sandbox/script classification |

---

# 30. Copy-Paste IDE Prompts

## 30.1 Phase 1 prompt

```text
Implement Phase 1 of the URPG Grid-Part Level Builder.

Do not replace MapScene, MapLoader, TileLayerDocument, SpatialMapOverlay, or any existing editor panel.

Add:
- engine/core/map/grid_part_types.h
- engine/core/map/grid_part_document.h
- engine/core/map/grid_part_document.cpp
- tests/unit/test_grid_part_document.cpp

Implement:
- GridPartCategory
- GridPartLayer
- GridPartCollisionPolicy
- GridPartSeverity
- GridPartRuleset
- GridPartFootprint
- GridPartDefinition
- PlacedPartInstance
- GridPartDiagnostic
- GridPartDocument

GridPartDocument must support:
- mapId, width, height
- placePart
- replacePart
- removePart
- movePart
- resizePart
- findPartMutable
- findPart
- partsAt
- partsAtLayer
- inBounds
- footprintInBounds
- hasInstanceId

Tests must cover:
- unique placement
- duplicate instance rejection
- missing instance ID rejection
- missing part ID rejection
- out-of-bounds footprint rejection
- move
- locked move rejection
- resize
- invalid resize rejection
- partsAt
- partsAtLayer
- remove

Update CMakeLists.txt.

Run:
ctest --test-dir build-local -R "grid_part"

Do not weaken tests to pass.
```

## 30.2 Phase 5 compiler prompt

```text
Implement Phase 5 of the URPG Grid-Part Level Builder.

Read:
- engine/core/map/grid_part_document.h
- engine/core/map/grid_part_catalog.h
- engine/core/map/tile_layer_document.h
- engine/core/map/spawn_table.h
- engine/core/map/map_region_rules.h
- engine/core/presentation/presentation_schema.h
- engine/core/scene/map_scene.h
- engine/core/scene/map_scene.cpp

Add:
- engine/core/map/grid_part_runtime_compiler.h
- engine/core/map/grid_part_runtime_compiler.cpp
- tests/unit/test_grid_part_runtime_compiler.cpp
- tests/integration/test_grid_part_map_scene_integration.cpp

Compiler must convert:
- Tile -> TileLayerDocument terrain
- Wall -> terrain + collision
- Enemy -> SpawnTable
- Hazard/Trigger/CutsceneZone -> MapRegionRule
- Prop/Door/Npc/TreasureChest/SavePoint/Shop/QuestItem -> SpatialMapOverlay props
- solid object footprints -> collision layer

ApplyToMapScene must update both:
- scene.setLayerData(...)
- scene.setTile(...) / scene.setTilePassable(...)

Do not change MapLoader.
Do not break existing map/spatial/level tests.
```

## 30.3 Phase 8 editor prompt

```text
Implement Phase 8 of the URPG Grid-Part Level Builder.

Read:
- editor/spatial/spatial_authoring_workspace.h/.cpp
- editor/spatial/prop_placement_panel.h
- editor/spatial/map_ability_binding_panel.h
- tests/unit/test_spatial_editor.cpp

Add:
- editor/spatial/grid_part_palette_panel.h/.cpp
- editor/spatial/grid_part_placement_panel.h/.cpp
- editor/spatial/grid_part_inspector_panel.h/.cpp
- tests/unit/test_grid_part_editor.cpp

Add ToolMode::Parts to SpatialAuthoringWorkspace.

Follow existing editor snapshot patterns:
- render captures snapshots
- tests use snapshots
- no live ImGui dependency for unit tests

GridPartPlacementPanel must:
- bind GridPartDocument
- bind GridPartCatalog
- bind SpatialMapOverlay
- select part
- hover selected part from screen
- place selected part from screen
- expose can_undo/can_redo
- support undo/redo through GridPartCommandHistory

Do not add advanced visuals yet.
Headless behavior first.
```

---

# 31. Final Verdict

The current URPG repo is already positioned well for this feature.

The correct architecture is:

```text
GridPartDocument as authoring source of truth
GridPartCatalog as available part library
Ruleset/Objectives as map intent
Validator/Reachability as safety proof
DependencyGraph as export proof
RuntimeCompiler as bridge into existing systems
RuntimeState as save/load authority
CommandHistory as editor safety
SpatialAuthoringWorkspace as UI integration point
```

This is not a rewrite.

This is a controlled upgrade that turns URPG's existing map/spatial/editor systems into a real maker-style creation pipeline.
