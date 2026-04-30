#pragma once

#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::map {

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

std::vector<GridPartDependency> CollectGridPartDependencies(const GridPartDocument& document,
                                                            const GridPartCatalog& catalog);

} // namespace urpg::map
