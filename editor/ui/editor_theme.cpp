#include "editor/ui/editor_theme.h"

#include <algorithm>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor::ui {
namespace {

#ifdef URPG_IMGUI_ENABLED
ImVec4 toImVec4(const EditorColor color) { return {color.r, color.g, color.b, color.a}; }
#endif

} // namespace

EditorThemeTokens defaultEditorTheme() { return {}; }

float editorUiScaleFactor(const EditorUiScale scale) {
    switch (scale) {
    case EditorUiScale::Percent100: return 1.0f;
    case EditorUiScale::Percent125: return 1.25f;
    case EditorUiScale::Percent150: return 1.5f;
    case EditorUiScale::Percent200: return 2.0f;
    }
    return 1.0f;
}

EditorThemeTokens scaledEditorTheme(const EditorUiScale scale) {
    return scaledEditorTheme(editorUiScaleFactor(scale));
}

EditorThemeTokens scaledEditorTheme(const float scaleFactor) {
    auto tokens = defaultEditorTheme();
    const auto factor = std::clamp(scaleFactor, 0.75f, 2.0f);
    tokens.spacing *= factor;
    tokens.controlHeight *= factor;
    tokens.iconSize *= factor;
    tokens.rounding *= factor;
    return tokens;
}

void applyEditorTheme(const float scaleFactor) {
#ifdef URPG_IMGUI_ENABLED
    const auto factor = std::clamp(scaleFactor, 0.75f, 2.0f);
    const auto tokens = scaledEditorTheme(factor);
    auto& style = ImGui::GetStyle();
    ImGui::GetIO().FontGlobalScale = factor;
    style.WindowPadding = {tokens.spacing * 1.5f, tokens.spacing * 1.5f};
    style.FramePadding = {tokens.spacing, (tokens.controlHeight - ImGui::GetFontSize()) * 0.5f};
    style.ItemSpacing = {tokens.spacing, tokens.spacing};
    style.FrameRounding = tokens.rounding;
    style.GrabRounding = tokens.rounding;
    style.Colors[ImGuiCol_WindowBg] = toImVec4(tokens.windowBackground);
    style.Colors[ImGuiCol_ChildBg] = toImVec4(tokens.surface);
    style.Colors[ImGuiCol_FrameBg] = toImVec4(tokens.surface);
    style.Colors[ImGuiCol_FrameBgHovered] = toImVec4(tokens.surfaceHover);
    style.Colors[ImGuiCol_Button] = toImVec4(tokens.selection);
    style.Colors[ImGuiCol_ButtonHovered] = toImVec4(tokens.surfaceHover);
    style.Colors[ImGuiCol_Header] = toImVec4(tokens.selection);
    style.Colors[ImGuiCol_HeaderHovered] = toImVec4(tokens.surfaceHover);
    style.Colors[ImGuiCol_Text] = toImVec4(tokens.text);
    style.Colors[ImGuiCol_TextDisabled] = toImVec4(tokens.mutedText);
    style.Colors[ImGuiCol_NavHighlight] = toImVec4(tokens.focus);
#else
    (void)scaleFactor;
#endif
}

void applyEditorTheme(const EditorUiScale scale) { applyEditorTheme(editorUiScaleFactor(scale)); }

} // namespace urpg::editor::ui
