#include "engine/core/ui/urpg_design_tokens.h"

#include <algorithm>
#include <cmath>

#include <nlohmann/json.hpp>

namespace urpg::ui {
namespace {

double channel(const float value) {
    return value <= 0.04045F ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
}

double luminance(const UrpgColorToken& color) {
    return 0.2126 * channel(color.red) + 0.7152 * channel(color.green) + 0.0722 * channel(color.blue);
}

const char* modeName(const UrpgThemeMode mode) {
    switch (mode) {
    case UrpgThemeMode::Light: return "light";
    case UrpgThemeMode::Dark: return "dark";
    case UrpgThemeMode::HighContrast: return "high_contrast";
    }
    return "dark";
}

} // namespace

double urpgColorContrastRatio(const UrpgColorToken& foreground, const UrpgColorToken& background) {
    const auto light = std::max(luminance(foreground), luminance(background));
    const auto dark = std::min(luminance(foreground), luminance(background));
    return (light + 0.05) / (dark + 0.05);
}

UrpgDesignTokens makeUrpgDesignTokens(const UrpgThemeMode mode, const float scale, const bool reduced_motion) {
    UrpgDesignTokens tokens;
    tokens.id = std::string("urpg.playful_workshop.") + modeName(mode);
    tokens.mode = mode;
    tokens.scale = scale;
    tokens.reduced_motion = reduced_motion;
    if (mode == UrpgThemeMode::Light) {
        tokens.colors = {{"surface", {0.96F, 0.94F, 0.89F, 1.0F}}, {"surface_raised", {1.0F, 0.99F, 0.96F, 1.0F}},
                         {"text", {0.12F, 0.14F, 0.17F, 1.0F}}, {"text_muted", {0.31F, 0.33F, 0.36F, 1.0F}},
                         {"accent", {0.08F, 0.32F, 0.48F, 1.0F}}, {"danger", {0.56F, 0.08F, 0.09F, 1.0F}},
                         {"warning", {0.50F, 0.27F, 0.02F, 1.0F}}, {"success", {0.07F, 0.39F, 0.19F, 1.0F}},
                         {"focus", {0.03F, 0.25F, 0.60F, 1.0F}}};
    } else if (mode == UrpgThemeMode::Dark) {
        tokens.colors = {{"surface", {0.08F, 0.09F, 0.11F, 1.0F}}, {"surface_raised", {0.14F, 0.15F, 0.18F, 1.0F}},
                         {"text", {0.95F, 0.94F, 0.90F, 1.0F}}, {"text_muted", {0.72F, 0.72F, 0.70F, 1.0F}},
                         {"accent", {0.38F, 0.76F, 0.94F, 1.0F}}, {"danger", {1.0F, 0.48F, 0.45F, 1.0F}},
                         {"warning", {1.0F, 0.75F, 0.30F, 1.0F}}, {"success", {0.43F, 0.86F, 0.56F, 1.0F}},
                         {"focus", {1.0F, 0.82F, 0.32F, 1.0F}}};
    } else {
        tokens.colors = {{"surface", {0.0F, 0.0F, 0.0F, 1.0F}}, {"surface_raised", {0.08F, 0.08F, 0.08F, 1.0F}},
                         {"text", {1.0F, 1.0F, 1.0F, 1.0F}}, {"text_muted", {0.85F, 0.85F, 0.85F, 1.0F}},
                         {"accent", {0.25F, 0.85F, 1.0F, 1.0F}}, {"danger", {1.0F, 0.35F, 0.35F, 1.0F}},
                         {"warning", {1.0F, 0.90F, 0.0F, 1.0F}}, {"success", {0.20F, 1.0F, 0.40F, 1.0F}},
                         {"focus", {1.0F, 1.0F, 0.0F, 1.0F}}};
    }
    const auto scaled = [scale](const float value) { return value * scale; };
    tokens.typography = {{"caption", scaled(12.0F)}, {"body", scaled(15.0F)}, {"label", scaled(14.0F)},
                         {"heading", scaled(20.0F)}, {"title", scaled(28.0F)}};
    tokens.spacing = {{"2xs", scaled(2.0F)}, {"xs", scaled(4.0F)}, {"sm", scaled(8.0F)},
                      {"md", scaled(12.0F)}, {"lg", scaled(18.0F)}, {"xl", scaled(24.0F)}};
    tokens.corner_radius = {{"control", scaled(5.0F)}, {"card", scaled(8.0F)}, {"popover", scaled(10.0F)}};
    tokens.borders = {{"hairline", scaled(1.0F)}, {"strong", scaled(2.0F)}};
    tokens.elevation = {{"flat", 0.0F}, {"card", scaled(2.0F)}, {"popover", scaled(8.0F)}};
    tokens.icons = {{"add", "urpg.icon.add"}, {"delete", "urpg.icon.delete"}, {"diagnostic", "urpg.icon.diagnostic"},
                    {"playtest", "urpg.icon.playtest"}, {"save", "urpg.icon.save"}};
    tokens.motion_ms = {{"instant", 0}, {"feedback", reduced_motion ? 0U : 90U},
                        {"transition", reduced_motion ? 0U : 160U}, {"emphasis", reduced_motion ? 0U : 240U}};
    tokens.sounds = {{"confirm", "urpg.sound.confirm"}, {"cancel", "urpg.sound.cancel"},
                     {"warning", "urpg.sound.warning"}};
    tokens.focus = {{"ring_width", scaled(mode == UrpgThemeMode::HighContrast ? 3.0F : 2.0F)},
                    {"ring_offset", scaled(2.0F)}};
    tokens.density = {{"minimum_hit_target", std::max(30.0F, scaled(40.0F))}, {"row_compact", scaled(28.0F)},
                      {"row_comfortable", scaled(36.0F)}};
    return tokens;
}

std::vector<std::string> validateUrpgDesignTokens(const UrpgDesignTokens& tokens) {
    std::vector<std::string> diagnostics;
    if (tokens.id.empty() || tokens.scale < 0.5F || tokens.scale > 3.0F) diagnostics.push_back("design_tokens_identity_invalid");
    for (const auto* role : {"surface", "surface_raised", "text", "text_muted", "accent", "danger", "warning",
                             "success", "focus"}) {
        if (!tokens.colors.contains(role)) diagnostics.push_back(std::string("design_tokens_color_missing:") + role);
    }
    if (diagnostics.empty()) {
        if (urpgColorContrastRatio(tokens.colors.at("text"), tokens.colors.at("surface")) < 4.5)
            diagnostics.push_back("design_tokens_text_contrast_low");
        if (urpgColorContrastRatio(tokens.colors.at("focus"), tokens.colors.at("surface")) < 3.0)
            diagnostics.push_back("design_tokens_focus_contrast_low");
    }
    if (!tokens.density.contains("minimum_hit_target") || tokens.density.at("minimum_hit_target") < 30.0F)
        diagnostics.push_back("design_tokens_hit_target_too_small");
    if (tokens.reduced_motion && std::any_of(tokens.motion_ms.begin(), tokens.motion_ms.end(), [](const auto& token) {
            return token.first != "instant" && token.second != 0;
        })) diagnostics.push_back("design_tokens_reduced_motion_nonzero");
    return diagnostics;
}

nlohmann::json urpgDesignTokenGallerySnapshot(const UrpgDesignTokens& tokens) {
    nlohmann::json colors = nlohmann::json::object();
    for (const auto& [name, color] : tokens.colors) colors[name] = {color.red, color.green, color.blue, color.alpha};
    return {{"id", tokens.id}, {"mode", modeName(tokens.mode)}, {"scale", tokens.scale},
            {"reduced_motion", tokens.reduced_motion}, {"colors", std::move(colors)},
            {"typography", tokens.typography}, {"spacing", tokens.spacing}, {"corner_radius", tokens.corner_radius},
            {"borders", tokens.borders}, {"elevation", tokens.elevation}, {"icons", tokens.icons},
            {"motion_ms", tokens.motion_ms}, {"sounds", tokens.sounds}, {"focus", tokens.focus},
            {"density", tokens.density}};
}

} // namespace urpg::ui
