#pragma once

#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_commands.h"

#include <string>
#include <vector>

namespace urpg::map {

struct GridPartPrefabUpdatePreview {
    bool valid = false;
    std::string code;
    std::string prefab_id;
    std::string from_version;
    std::string to_version;
    size_t affected_instance_count = 0;
    size_t preserved_override_count = 0;
    std::vector<PlacedPartInstance> replacements;
    std::vector<PlacedPartInstance> additions;
    std::vector<std::string> removals;
    std::vector<std::string> package_closure;
};

GridPartPrefabUpdatePreview previewGridPartPrefabUpdate(const GridPartDocument& document,
                                                        const GridPartCatalog& catalog,
                                                        const GridPartSmartPrefab& next);
bool applyGridPartPrefabUpdate(GridPartDocument& document, GridPartCommandHistory& history,
                               const GridPartPrefabUpdatePreview& preview);
bool detachGridPartPrefabGroup(GridPartDocument& document, GridPartCommandHistory& history,
                               const std::string& group_id);

} // namespace urpg::map
