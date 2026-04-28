#pragma once

#include "engine/core/accessibility/accessibility_auditor.h"
#include "engine/core/render/render_layer.h"

#include <string>
#include <vector>

namespace urpg::accessibility {

struct RenderContrastAdapterOptions {
    bool text_is_focusable = false;
    int32_t first_focus_order = 1;
};

struct RenderContrastExtractionReport {
    std::string surface_id;
    size_t text_command_count = 0;
    size_t backed_text_count = 0;
    size_t unbacked_text_count = 0;
    size_t audited_element_count = 0;
    float minimum_contrast_ratio = 0.0f;
    std::vector<UiElementSnapshot> elements;
};

std::vector<UiElementSnapshot>
ingestRendererContrastElements(const std::vector<urpg::FrameRenderCommand>& commands,
                               const RenderContrastAdapterOptions& options = {});

RenderContrastExtractionReport
extractRendererContrastReport(const std::vector<urpg::FrameRenderCommand>& commands,
                              std::string surface_id,
                              const RenderContrastAdapterOptions& options = {});

} // namespace urpg::accessibility
