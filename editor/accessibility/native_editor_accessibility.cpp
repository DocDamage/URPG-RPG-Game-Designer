#include "editor/accessibility/native_editor_accessibility.h"

#include "engine/core/editor/editor_shell.h"

namespace urpg::editor {
namespace {

constexpr std::string_view kPanelPrefix = "panel.";

} // namespace

NativeAccessibilitySnapshot nativeAccessibilitySnapshotForEditorShell(const EditorShell& shell) {
    const auto editor = shell.snapshot();
    NativeAccessibilitySnapshot result;
    result.nodes.reserve(editor.panels.size() + 1);

    result.nodes.push_back({
        "project.current",
        "Current project",
        editor.project_root.empty() ? "No project is open." : "Project root for the current editor session.",
        editor.project_root.empty() ? "No project" : editor.project_root.generic_string(),
        {},
        NativeAccessibilityRole::Text,
        true,
        false,
        false,
    });

    for (const auto& panel : editor.panels) {
        result.nodes.push_back({
            std::string(kPanelPrefix) + panel.id,
            panel.title.empty() ? panel.id : panel.title,
            panel.category.empty() ? "Editor workspace" : panel.category + " editor workspace",
            panel.id == editor.active_panel_id ? "Active" : "Available",
            "Open",
            NativeAccessibilityRole::Button,
            panel.enabled,
            panel.enabled,
            panel.id == editor.active_panel_id,
        });
    }
    return result;
}

bool activateNativeEditorAccessibilityNode(EditorShell& shell, const std::string_view node_id) {
    if (!node_id.starts_with(kPanelPrefix)) return false;
    const auto panel_id = node_id.substr(kPanelPrefix.size());
    return !panel_id.empty() && shell.openPanel(panel_id);
}

} // namespace urpg::editor
