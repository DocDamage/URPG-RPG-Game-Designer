#pragma once

#include "engine/core/accessibility/accessibility_auditor.h"
#include "editor/spatial/elevation_brush_panel.h"
#include "editor/spatial/grid_part_placement_panel.h"
#include "editor/spatial/map_authoring_workspace.h"
#include "editor/spatial/prop_placement_panel.h"

#include <nlohmann/json.hpp>

#include <vector>

namespace urpg::editor {

/**
 * @brief Adapts live spatial editor panel snapshots into UiElementSnapshot elements
 *        for accessibility auditing.
 *
 * The spatial surface is modelled as a set of virtual elements:
 *  - One element per editor panel (elevation brush, prop placement, Grid Parts).
 *  - One element per placed prop in the active overlay.
 */
class AccessibilitySpatialAdapter {
public:
    // Compatibility overload for consumers that do not own a Grid Part
    // snapshot. It contributes no Grid Part virtual elements.
    static std::vector<urpg::accessibility::UiElementSnapshot> ingest(
        const ElevationBrushPanel::RenderSnapshot& elevationSnapshot,
        const PropPlacementPanel::RenderSnapshot& propSnapshot,
        const MapAuthoringWorkspaceSnapshot* mapSnapshot = nullptr);
    static std::vector<urpg::accessibility::UiElementSnapshot> ingest(
        const ElevationBrushPanel::RenderSnapshot& elevationSnapshot,
        const PropPlacementPanel::RenderSnapshot& propSnapshot,
        const GridPartPlacementPanel::RenderSnapshot& gridPartSnapshot,
        const MapAuthoringWorkspaceSnapshot* mapSnapshot = nullptr);
    // Creator-command plans are reviewed through the Map owner but are not
    // canvas-only controls. Expose their current review/apply state as
    // labelled virtual actions for the same read-only audit.
    static std::vector<urpg::accessibility::UiElementSnapshot> ingestCreatorCommand(
        const nlohmann::json& creator_command_snapshot);
};

} // namespace urpg::editor
