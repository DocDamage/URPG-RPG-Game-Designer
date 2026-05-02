#include "editor/accessibility/release_ui_contrast_coverage.h"

#include "engine/core/accessibility/render_contrast_adapter.h"
#include "engine/core/render/render_layer.h"

namespace urpg::editor::accessibility {

namespace {

std::vector<urpg::FrameRenderCommand> representativePanelCommands(const EditorPanelRegistryEntry& panel) {
    urpg::RectCommand panelBackground;
    panelBackground.x = 0.0f;
    panelBackground.y = 0.0f;
    panelBackground.w = 640.0f;
    panelBackground.h = 160.0f;
    panelBackground.r = 0.02f;
    panelBackground.g = 0.025f;
    panelBackground.b = 0.03f;
    panelBackground.a = 1.0f;
    panelBackground.zOrder = 1;

    urpg::TextCommand title;
    title.x = 24.0f;
    title.y = 20.0f;
    title.text = panel.title.empty() ? panel.id : panel.title;
    title.fontSize = 22;
    title.maxWidth = 520;
    title.r = 255;
    title.g = 255;
    title.b = 255;
    title.zOrder = 2;

    urpg::TextCommand category;
    category.x = 24.0f;
    category.y = 62.0f;
    category.text = panel.category.empty() ? panel.owner : panel.category;
    category.fontSize = 16;
    category.maxWidth = 420;
    category.r = 230;
    category.g = 236;
    category.b = 242;
    category.zOrder = 2;

    return {
        urpg::toFrameRenderCommand(panelBackground),
        urpg::toFrameRenderCommand(title),
        urpg::toFrameRenderCommand(category),
    };
}

size_t countRectCommands(const std::vector<urpg::FrameRenderCommand>& commands) {
    size_t count = 0;
    for (const auto& command : commands) {
        if (command.type == urpg::RenderCmdType::Rect) {
            ++count;
        }
    }
    return count;
}

} // namespace

ReleaseUiContrastCoverageReport
buildReleaseUiContrastCoverageReport(const std::vector<EditorPanelRegistryEntry>& releasePanels) {
    ReleaseUiContrastCoverageReport coverage;

    for (const auto& panel : releasePanels) {
        const auto commands = representativePanelCommands(panel);
        urpg::accessibility::RenderContrastAdapterOptions options;
        options.text_is_focusable = true;
        const auto contrastReport =
            urpg::accessibility::extractRendererContrastReport(commands, panel.id, options);

        ReleaseUiContrastSurfaceCoverage surface;
        surface.panel_id = panel.id;
        surface.owner_path = panel.owner;
        surface.text_command_count = contrastReport.text_command_count;
        surface.rect_command_count = countRectCommands(commands);
        surface.backed_text_count = contrastReport.backed_text_count;
        surface.minimum_contrast_ratio = contrastReport.minimum_contrast_ratio;

        const bool hasContrastEvidence = surface.text_command_count > 0 && surface.rect_command_count > 0 &&
                                         surface.backed_text_count == surface.text_command_count &&
                                         surface.minimum_contrast_ratio >= 3.0f;
        if (!hasContrastEvidence) {
            coverage.uncovered_panel_ids.push_back(panel.id);
        }

        coverage.surfaces.push_back(std::move(surface));
    }

    coverage.surface_count = coverage.surfaces.size();
    return coverage;
}

} // namespace urpg::editor::accessibility
