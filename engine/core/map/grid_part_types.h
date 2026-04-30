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

enum class GridPartLayer : uint8_t { Terrain = 0, Decoration, Collision, Object, Actor, Trigger, Region, Overlay };

enum class GridPartCollisionPolicy : uint8_t { None = 0, Solid, Hazard, TriggerOnly, Custom };

enum class GridPartSeverity : uint8_t { Info = 0, Warning, Error, Blocker };

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

struct GridPartChunkCoord {
    int32_t chunk_x = 0;
    int32_t chunk_y = 0;

    friend bool operator==(const GridPartChunkCoord& left, const GridPartChunkCoord& right) {
        return left.chunk_x == right.chunk_x && left.chunk_y == right.chunk_y;
    }
};

struct GridPartChunk {
    GridPartChunkCoord coord;
    std::vector<std::string> instance_ids;
    bool dirty = true;
};

} // namespace urpg::map
