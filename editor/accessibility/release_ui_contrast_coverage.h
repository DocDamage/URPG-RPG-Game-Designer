#pragma once

#include "engine/core/editor/editor_panel_registry.h"

#include <string>
#include <vector>

namespace urpg::editor::accessibility {

struct ReleaseUiContrastSurfaceCoverage {
    std::string panel_id;
    std::string owner_path;
    size_t text_command_count = 0;
    size_t rect_command_count = 0;
    size_t backed_text_count = 0;
    float minimum_contrast_ratio = 0.0f;
    bool exempt = false;
    std::string exemption_reason;
};

struct ReleaseUiContrastCoverageReport {
    size_t surface_count = 0;
    std::vector<ReleaseUiContrastSurfaceCoverage> surfaces;
    std::vector<std::string> uncovered_panel_ids;
};

ReleaseUiContrastCoverageReport
buildReleaseUiContrastCoverageReport(const std::vector<EditorPanelRegistryEntry>& releasePanels);

} // namespace urpg::editor::accessibility
