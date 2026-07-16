#pragma once

#include "editor/accessibility/native_editor_accessibility.h"
#include "editor/accessibility/semantic_editor_command_surface.h"

#include <string>
#include <string_view>

namespace urpg::editor {

struct NativeSemanticEditorCommandResult {
    bool applied = false;
    bool document_changed = false;
};

void appendNativeSemanticEditorAccessibility(NativeAccessibilitySnapshot& tree, std::string_view domain,
                                             std::string_view label, SemanticEditorCommandSurface& surface,
                                             std::string_view selected_id);

NativeSemanticEditorCommandResult activateNativeSemanticEditorAccessibilityNode(
    std::string_view domain, std::string_view node_id, SemanticEditorCommandSurface& surface,
    std::string* selected_id);

NativeSemanticEditorCommandResult setNativeSemanticEditorAccessibilityValue(
    std::string_view domain, std::string_view node_id, std::string_view value,
    SemanticEditorCommandSurface& surface, std::string* selected_id);

} // namespace urpg::editor
