#pragma once

#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/map/grid_part_validator.h"

#include <cstdint>
#include <string>

namespace urpg::map {

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

GridRulesetProfile MakeDefaultGridRulesetProfile(GridPartRuleset ruleset);

GridPartValidationResult ValidateGridPartRuleset(const GridPartDocument& document, const GridPartCatalog& catalog,
                                                 const GridRulesetProfile& profile);

} // namespace urpg::map
