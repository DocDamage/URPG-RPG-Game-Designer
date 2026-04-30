#pragma once

#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/map/grid_part_validator.h"

#include <cstdint>
#include <string>

namespace urpg::map {

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

GridPartValidationResult ValidateMapObjective(const GridPartDocument& document, const GridPartCatalog& catalog,
                                              const MapObjective& objective);

} // namespace urpg::map
