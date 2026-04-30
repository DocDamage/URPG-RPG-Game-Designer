#pragma once

#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"

#include <optional>
#include <vector>

namespace urpg::map {

struct GridPartValidationOptions {
    std::optional<GridPartRuleset> ruleset;
};

struct GridPartValidationResult {
    bool ok = true;
    std::vector<GridPartDiagnostic> diagnostics;

    bool hasErrors() const;
    bool hasBlockers() const;
};

GridPartValidationResult ValidateGridPartDocument(const GridPartDocument& document, const GridPartCatalog& catalog,
                                                  const GridPartValidationOptions& options = {});

} // namespace urpg::map
