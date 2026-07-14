#pragma once

#include <array>

namespace urpg::editor::ui {

// Renderer-independent values keep the editor's visual language testable in
// headless builds. ImGui conversion happens only at the application boundary.
struct EditorColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct EditorThemeTokens {
    EditorColor windowBackground{0.055f, 0.065f, 0.085f, 1.0f};
    EditorColor surface{0.090f, 0.105f, 0.135f, 1.0f};
    EditorColor surfaceHover{0.135f, 0.160f, 0.205f, 1.0f};
    EditorColor selection{0.180f, 0.430f, 0.800f, 1.0f};
    EditorColor focus{0.980f, 0.720f, 0.220f, 1.0f};
    EditorColor text{0.940f, 0.955f, 0.980f, 1.0f};
    EditorColor mutedText{0.690f, 0.735f, 0.800f, 1.0f};
    EditorColor success{0.280f, 0.770f, 0.460f, 1.0f};
    EditorColor warning{0.980f, 0.690f, 0.210f, 1.0f};
    EditorColor error{0.920f, 0.310f, 0.330f, 1.0f};
    float spacing = 8.0f;
    float controlHeight = 30.0f;
    float iconSize = 18.0f;
    float rounding = 5.0f;
};

struct EditorThemePreferences {
    float uiScale = 1.0f;
    bool highContrast = false;
};

enum class EditorUiScale { Percent100, Percent125, Percent150, Percent200 };

[[nodiscard]] EditorThemeTokens defaultEditorTheme();
[[nodiscard]] EditorThemeTokens highContrastEditorTheme();
[[nodiscard]] float editorUiScaleFactor(EditorUiScale scale);
[[nodiscard]] EditorThemeTokens scaledEditorTheme(EditorUiScale scale);
[[nodiscard]] EditorThemeTokens scaledEditorTheme(float scaleFactor);
[[nodiscard]] EditorThemeTokens themedEditorTheme(const EditorThemePreferences& preferences);

// Safe to call after ImGui::CreateContext(). It is intentionally a no-op for
// the deterministic headless/no-ImGui configurations.
void applyEditorTheme(float scaleFactor = 1.0f);
void applyEditorTheme(EditorUiScale scale);
void applyEditorTheme(const EditorThemePreferences& preferences);

} // namespace urpg::editor::ui
