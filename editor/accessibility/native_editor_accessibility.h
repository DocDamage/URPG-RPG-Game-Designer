#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

class EditorShell;

enum class NativeAccessibilityRole {
    Text,
    Button,
    ListItem,
    Group,
    Diagnostic,
};

struct NativeAccessibilityNode {
    std::string id;
    std::string name;
    std::string description;
    std::string value;
    std::string default_action;
    NativeAccessibilityRole role = NativeAccessibilityRole::Text;
    bool enabled = true;
    bool focusable = false;
    bool selected = false;
};

struct NativeAccessibilitySnapshot {
    std::string name = "URPG Editor";
    std::string description = "URPG deterministic RPG creator";
    std::vector<NativeAccessibilityNode> nodes;
};

[[nodiscard]] NativeAccessibilitySnapshot nativeAccessibilitySnapshotForEditorShell(const EditorShell& shell);
bool activateNativeEditorAccessibilityNode(EditorShell& shell, std::string_view node_id);

} // namespace urpg::editor
