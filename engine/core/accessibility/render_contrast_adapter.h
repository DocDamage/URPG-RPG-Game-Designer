#pragma once

#include "engine/core/accessibility/accessibility_auditor.h"
#include "engine/core/render/render_layer.h"

#include <vector>

namespace urpg::accessibility {

struct RenderContrastAdapterOptions {
    bool text_is_focusable = false;
    int32_t first_focus_order = 1;
};

std::vector<UiElementSnapshot>
ingestRendererContrastElements(const std::vector<urpg::FrameRenderCommand>& commands,
                               const RenderContrastAdapterOptions& options = {});

} // namespace urpg::accessibility
