#pragma once

#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_commands.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/map/grid_part_validator.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::map {

struct GridPartStamp {
    std::string stamp_id;
    std::string display_name;
    std::vector<PlacedPartInstance> parts;

    int32_t width = 1;
    int32_t height = 1;
    std::vector<std::string> tags;
};

struct GridPartStampPlacementResult {
    bool ok = false;
    std::vector<std::string> placed_instance_ids;
    std::vector<GridPartDiagnostic> diagnostics;
};

GridPartStampPlacementResult PlaceGridPartStamp(GridPartDocument& document, const GridPartCatalog& catalog,
                                                const GridPartStamp& stamp, int32_t origin_x, int32_t origin_y);

} // namespace urpg::map
