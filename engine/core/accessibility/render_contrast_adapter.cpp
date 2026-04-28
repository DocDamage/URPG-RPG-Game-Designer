#include "engine/core/accessibility/render_contrast_adapter.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace urpg::accessibility {

namespace {

struct RectBounds {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

struct CandidateBackground {
    RectBounds bounds;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    int32_t z = 0;
};

float linearize(float channel) {
    channel = std::clamp(channel, 0.0f, 1.0f);
    return channel <= 0.03928f ? channel / 12.92f : std::pow((channel + 0.055f) / 1.055f, 2.4f);
}

float luminance(float r, float g, float b) {
    return 0.2126f * linearize(r) + 0.7152f * linearize(g) + 0.0722f * linearize(b);
}

float contrastRatio(float textR, float textG, float textB, float bgR, float bgG, float bgB) {
    const float l1 = luminance(textR, textG, textB);
    const float l2 = luminance(bgR, bgG, bgB);
    const float lighter = std::max(l1, l2);
    const float darker = std::min(l1, l2);
    return (lighter + 0.05f) / (darker + 0.05f);
}

bool contains(const RectBounds& outer, const RectBounds& inner) {
    return inner.x >= outer.x && inner.y >= outer.y && inner.x + inner.w <= outer.x + outer.w &&
           inner.y + inner.h <= outer.y + outer.h;
}

RectBounds textBounds(const urpg::FrameRenderCommand& command, const urpg::TextRenderData& text) {
    const float estimatedWidth =
        text.maxWidth > 0 ? static_cast<float>(text.maxWidth)
                          : std::max(1.0f, static_cast<float>(text.text.size()) * static_cast<float>(text.fontSize) * 0.55f);
    const float estimatedHeight = std::max(1.0f, static_cast<float>(text.fontSize) * 1.25f);
    return {command.x, command.y, estimatedWidth, estimatedHeight};
}

std::vector<CandidateBackground> collectBackgrounds(const std::vector<urpg::FrameRenderCommand>& commands) {
    std::vector<CandidateBackground> backgrounds;
    for (const auto& command : commands) {
        if (command.type != urpg::RenderCmdType::Rect) {
            continue;
        }
        const auto* rect = command.tryGet<urpg::RectRenderData>();
        if (rect == nullptr || rect->a <= 0.0f) {
            continue;
        }
        backgrounds.push_back({
            {command.x, command.y, rect->w, rect->h},
            rect->r,
            rect->g,
            rect->b,
            command.zOrder,
        });
    }
    return backgrounds;
}

const CandidateBackground* findBackgroundForText(const std::vector<CandidateBackground>& backgrounds,
                                                 const RectBounds& bounds,
                                                 int32_t textZ) {
    const CandidateBackground* best = nullptr;
    for (const auto& background : backgrounds) {
        if (background.z > textZ || !contains(background.bounds, bounds)) {
            continue;
        }
        if (best == nullptr || background.z >= best->z) {
            best = &background;
        }
    }
    return best;
}

} // namespace

std::vector<UiElementSnapshot>
ingestRendererContrastElements(const std::vector<urpg::FrameRenderCommand>& commands,
                               const RenderContrastAdapterOptions& options) {
    std::vector<UiElementSnapshot> elements;
    const auto backgrounds = collectBackgrounds(commands);
    int32_t focusOrder = options.first_focus_order;
    size_t textIndex = 0;

    for (const auto& command : commands) {
        if (command.type != urpg::RenderCmdType::Text) {
            continue;
        }
        const auto* text = command.tryGet<urpg::TextRenderData>();
        if (text == nullptr || text->text.empty()) {
            continue;
        }

        const auto bounds = textBounds(command, *text);
        const auto* background = findBackgroundForText(backgrounds, bounds, command.zOrder);
        float ratio = 0.0f;
        if (background != nullptr) {
            ratio = contrastRatio(static_cast<float>(text->r) / 255.0f,
                                  static_cast<float>(text->g) / 255.0f,
                                  static_cast<float>(text->b) / 255.0f,
                                  background->r,
                                  background->g,
                                  background->b);
        }

        UiElementSnapshot element;
        element.id = "render.text." + std::to_string(textIndex);
        element.label = text->text;
        element.hasFocus = options.text_is_focusable;
        element.focusOrder = options.text_is_focusable ? focusOrder++ : 0;
        element.contrastRatio = ratio;
        element.sourceContext = "engine/core/render/render_layer.h";
        elements.push_back(std::move(element));
        ++textIndex;
    }

    return elements;
}

} // namespace urpg::accessibility
