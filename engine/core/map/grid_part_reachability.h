#pragma once

#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/map/grid_part_objective.h"
#include "engine/core/map/grid_part_ruleset.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace urpg::map {

struct ReachabilityReport {
    bool ok = false;
    std::vector<GridPartDiagnostic> diagnostics;
    std::vector<std::pair<int32_t, int32_t>> reachable_cells;
};

ReachabilityReport ValidateReachability(const GridPartDocument& document, const GridPartCatalog& catalog,
                                        const GridRulesetProfile& ruleset, const MapObjective& objective);

} // namespace urpg::map
