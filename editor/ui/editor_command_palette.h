#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

struct EditorCommandDescriptor {
    std::string id;
    std::string label;
    std::string category;
    std::vector<std::string> keywords;
    std::string shortcut;
    std::string requirement;
    std::string help;
    bool enabled = true;
};

struct EditorCommandSearchResult {
    EditorCommandDescriptor command;
    int score = 0;
    bool recent = false;
};

class EditorCommandPalette {
public:
    bool registerCommand(EditorCommandDescriptor command);
    std::vector<EditorCommandSearchResult> search(std::string_view query, std::size_t limit = 20) const;
    bool recordAction(std::string_view command_id);
    std::vector<EditorCommandDescriptor> recentActions(std::size_t limit = 10) const;
    std::vector<EditorCommandDescriptor> shortcutDiscovery() const;
    const std::vector<EditorCommandDescriptor>& commands() const { return commands_; }

private:
    std::vector<EditorCommandDescriptor> commands_;
    std::vector<std::string> recent_ids_;
};

EditorCommandPalette buildGoldenLoopCommandPalette();

} // namespace urpg::editor
