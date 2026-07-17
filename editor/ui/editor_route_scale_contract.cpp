#include "editor/ui/editor_route_scale_contract.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include <nlohmann/json.hpp>

namespace urpg::editor {

const std::vector<std::string>& primaryEditorRoutes() {
    static const std::vector<std::string> routes = {
        "startup", "map", "assets", "diagnostics", "ability", "character_creator",
        "spatial_authoring", "patterns", "mod"};
    return routes;
}

EditorRouteScaleSnapshot editorRouteScaleSnapshot(std::string route, const float requestedScale) {
    const auto scale = std::isfinite(requestedScale) ? std::clamp(requestedScale, 0.5F, 3.0F) : 1.0F;
    return {std::move(route), scale, std::max(30.0F, 40.0F * scale), 20.0F * scale,
            480.0F * scale, 320.0F * scale, 12.0F * scale, true, true, false,
            {"project_navigator", "context_toolbar", "central_canvas", "inspector",
             "status_jobs", "diagnostics", "playtest_controls"}};
}

std::vector<std::string> validateEditorRouteScaleSnapshot(const EditorRouteScaleSnapshot& snapshot) {
    std::vector<std::string> diagnostics;
    if (std::find(primaryEditorRoutes().begin(), primaryEditorRoutes().end(), snapshot.route) ==
        primaryEditorRoutes().end()) diagnostics.push_back("editor_scale_route_unknown");
    if (snapshot.scale < 0.5F || snapshot.scale > 3.0F) diagnostics.push_back("editor_scale_out_of_range");
    if (snapshot.minimum_hit_target < 30.0F) diagnostics.push_back("editor_scale_hit_target_too_small");
    if (snapshot.icon_size <= 0.0F || snapshot.popup_preferred_width <= 0.0F ||
        snapshot.canvas_minimum_extent <= 0.0F || snapshot.outer_margin <= 0.0F) {
        diagnostics.push_back("editor_scale_geometry_invalid");
    }
    if (!snapshot.fonts_scaled) diagnostics.push_back("editor_scale_fonts_unscaled");
    if (!snapshot.resizable_constraints || snapshot.fixed_window_geometry) {
        diagnostics.push_back("editor_scale_fixed_geometry");
    }
    if (snapshot.workspace_regions != std::vector<std::string>{
            "project_navigator", "context_toolbar", "central_canvas", "inspector",
            "status_jobs", "diagnostics", "playtest_controls"}) {
        diagnostics.push_back("editor_workspace_regions_incomplete");
    }
    return diagnostics;
}

nlohmann::json editorRouteScaleMatrixSnapshot() {
    constexpr float scales[]{0.5F, 1.0F, 1.5F, 2.0F, 3.0F};
    auto rows = nlohmann::json::array();
    for (const auto scale : scales) {
        for (const auto& route : primaryEditorRoutes()) {
            const auto snapshot = editorRouteScaleSnapshot(route, scale);
            rows.push_back({{"route", snapshot.route}, {"scale", snapshot.scale},
                            {"minimum_hit_target", snapshot.minimum_hit_target},
                            {"icon_size", snapshot.icon_size},
                            {"popup_preferred_width", snapshot.popup_preferred_width},
                            {"canvas_minimum_extent", snapshot.canvas_minimum_extent},
                            {"outer_margin", snapshot.outer_margin},
                            {"fonts_scaled", snapshot.fonts_scaled},
                            {"resizable_constraints", snapshot.resizable_constraints},
                            {"fixed_window_geometry", snapshot.fixed_window_geometry},
                            {"workspace_regions", snapshot.workspace_regions}});
        }
    }
    return {{"schema", "urpg.editor_route_scale_matrix.v1"}, {"rows", std::move(rows)}};
}

} // namespace urpg::editor
