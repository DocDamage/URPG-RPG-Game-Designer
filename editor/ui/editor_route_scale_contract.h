#pragma once

#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace urpg::editor {

struct EditorRouteScaleSnapshot {
    std::string route;
    float scale = 1.0F;
    float minimum_hit_target = 40.0F;
    float icon_size = 20.0F;
    float popup_preferred_width = 480.0F;
    float canvas_minimum_extent = 320.0F;
    float outer_margin = 12.0F;
    bool fonts_scaled = true;
    bool resizable_constraints = true;
    bool fixed_window_geometry = false;
    std::vector<std::string> workspace_regions;
};

const std::vector<std::string>& primaryEditorRoutes();
EditorRouteScaleSnapshot editorRouteScaleSnapshot(std::string route, float scale);
std::vector<std::string> validateEditorRouteScaleSnapshot(const EditorRouteScaleSnapshot& snapshot);
nlohmann::json editorRouteScaleMatrixSnapshot();

} // namespace urpg::editor
