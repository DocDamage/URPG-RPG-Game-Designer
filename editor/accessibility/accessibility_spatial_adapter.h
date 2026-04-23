#pragma once

#include "engine/core/accessibility/accessibility_auditor.h"
#include "editor/spatial/elevation_brush_panel.h"
#include "editor/spatial/prop_placement_panel.h"
#include <vector>

namespace urpg::editor {

/**
 * @brief Adapts live spatial editor panel snapshots into UiElementSnapshot elements
 *        for accessibility auditing.
 *
 * The spatial surface is modelled as a set of virtual elements:
 *  - One element per editor panel (elevation brush, prop placement).
 *  - One element per placed prop in the active overlay.
 */
class AccessibilitySpatialAdapter {
public:
    static std::vector<urpg::accessibility::UiElementSnapshot> ingest(
        const ElevationBrushPanel::RenderSnapshot& elevationSnapshot,
        const PropPlacementPanel::RenderSnapshot& propSnapshot);
};

} // namespace urpg::editor
