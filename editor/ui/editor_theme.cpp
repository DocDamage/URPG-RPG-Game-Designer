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

EditorThemeTokens highContrastEditorTheme() {
    auto tokens = defaultEditorTheme();
    tokens.windowBackground = {0.005f, 0.005f, 0.005f, 1.0f};
    tokens.surface = {0.035f, 0.035f, 0.035f, 1.0f};
    tokens.surfaceHover = {0.160f, 0.160f, 0.160f, 1.0f};
    tokens.selection = {0.180f, 0.570f, 1.000f, 1.0f};
    tokens.focus = {1.000f, 0.920f, 0.120f, 1.0f};
    tokens.text = {1.000f, 1.000f, 1.000f, 1.0f};
    tokens.mutedText = {0.830f, 0.830f, 0.830f, 1.0f};
    tokens.success = {0.200f, 0.920f, 0.420f, 1.0f};
    tokens.warning = {1.000f, 0.820f, 0.120f, 1.0f};
    tokens.error = {1.000f, 0.400f, 0.400f, 1.0f};
    return tokens;
}

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

EditorThemeTokens themedEditorTheme(const EditorThemePreferences& preferences) {
    auto tokens = preferences.highContrast ? highContrastEditorTheme() : defaultEditorTheme();
    const auto factor = std::clamp(preferences.uiScale, 0.75f, 2.0f);
    tokens.spacing *= factor;
    tokens.controlHeight *= factor;
    tokens.iconSize *= factor;
    tokens.rounding *= factor;
    return tokens;
}

void applyEditorTheme(const float scaleFactor) {
    applyEditorTheme(EditorThemePreferences{scaleFactor, false});
}

void applyEditorTheme(const EditorThemePreferences& preferences) {
#ifdef URPG_IMGUI_ENABLED
    const auto factor = std::clamp(preferences.uiScale, 0.75f, 2.0f);
    const auto tokens = themedEditorTheme(preferences);
    auto& style = ImGui::GetStyle();
    ImGui::GetIO().FontGlobalScale = factor;
    style.WindowPadding = {tokens.spacing * 1.5f, tokens.spacing * 1.5f};
    style.FramePadding = {tokens.spacing, (tokens.controlHeight - ImGui::GetFontSize()) * 0.5f};
    style.ItemSpacing = {tokens.spacing, tokens.spacing};
    style.FrameRounding = tokens.rounding;
    style.GrabRounding = tokens.rounding;
    style.Colors[ImGuiCol_WindowBg] = toImVec4(tokens.windowBackground);
    style.Colors[ImGuiCol_ChildBg] = toImVec4(tokens.surface);
    style.Colors[ImGuiCol_TitleBg] = toImVec4(tokens.surface);
    style.Colors[ImGuiCol_TitleBgActive] = toImVec4(tokens.surfaceHover);
    style.Colors[ImGuiCol_TitleBgCollapsed] = toImVec4(tokens.windowBackground);
    style.Colors[ImGuiCol_FrameBg] = toImVec4(tokens.surface);
    style.Colors[ImGuiCol_FrameBgHovered] = toImVec4(tokens.surfaceHover);
    style.Colors[ImGuiCol_FrameBgActive] = toImVec4(tokens.surfaceHover);
    style.Colors[ImGuiCol_Button] = toImVec4(tokens.surfaceHover);
    style.Colors[ImGuiCol_ButtonHovered] = toImVec4(tokens.selection);
    style.Colors[ImGuiCol_ButtonActive] = toImVec4(tokens.selection);
    style.Colors[ImGuiCol_Header] = toImVec4(tokens.surfaceHover);
    style.Colors[ImGuiCol_HeaderHovered] = toImVec4(tokens.selection);
    style.Colors[ImGuiCol_HeaderActive] = toImVec4(tokens.selection);
    style.Colors[ImGuiCol_Text] = toImVec4(tokens.text);
    style.Colors[ImGuiCol_TextDisabled] = toImVec4(tokens.mutedText);
    style.Colors[ImGuiCol_Border] = {tokens.mutedText.r, tokens.mutedText.g, tokens.mutedText.b, 0.55f};
    style.Colors[ImGuiCol_Separator] = {tokens.mutedText.r, tokens.mutedText.g, tokens.mutedText.b, 0.45f};
    style.Colors[ImGuiCol_CheckMark] = toImVec4(tokens.focus);
    style.Colors[ImGuiCol_NavHighlight] = toImVec4(tokens.focus);
#else
    (void)preferences;
#endif
}

void applyEditorTheme(const EditorUiScale scale) { applyEditorTheme(editorUiScaleFactor(scale)); }

} // namespace urpg::editor::ui
