#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

enum class EditorPanelExposure {
    TopLevel,
    NestedWorkspace,
    Internal,
    Disabled,
};

struct EditorPanelRegistryEntry {
    std::string id;
    std::string title;
    std::string category;
    EditorPanelExposure exposure = EditorPanelExposure::Internal;
    std::string owner;
    std::string reason;
};

const std::vector<EditorPanelRegistryEntry>& editorPanelRegistry();
std::vector<EditorPanelRegistryEntry> topLevelEditorPanels();
std::vector<std::string> requiredTopLevelPanelIds();
const EditorPanelRegistryEntry* findEditorPanelRegistryEntry(std::string_view id);
bool hiddenEditorPanelEntriesHaveReasons();

} // namespace urpg::editor
