#include "editor/ui/editor_command_palette.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace urpg::editor {
namespace {

std::string normalized(std::string_view value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return result;
}

int matchScore(const EditorCommandDescriptor& command, const std::string& query) {
    if (query.empty()) return 1;
    const auto id = normalized(command.id);
    const auto label = normalized(command.label);
    const auto category = normalized(command.category);
    if (id == query || label == query) return 100;
    if (label.starts_with(query)) return 80;
    if (id.starts_with(query)) return 70;
    if (label.find(query) != std::string::npos) return 60;
    if (id.find(query) != std::string::npos) return 50;
    if (category.find(query) != std::string::npos) return 40;
    for (const auto& keyword : command.keywords) {
        if (normalized(keyword).find(query) != std::string::npos) return 30;
    }
    if (normalized(command.help).find(query) != std::string::npos) return 20;
    if (normalized(command.requirement).find(query) != std::string::npos) return 10;
    return 0;
}

} // namespace

bool EditorCommandPalette::registerCommand(EditorCommandDescriptor command) {
    if (command.id.empty() || command.label.empty() || command.category.empty() || command.help.empty() ||
        std::any_of(commands_.begin(), commands_.end(), [&](const auto& existing) { return existing.id == command.id; })) {
        return false;
    }
    commands_.push_back(std::move(command));
    return true;
}

std::vector<EditorCommandSearchResult> EditorCommandPalette::search(const std::string_view query,
                                                                     const std::size_t limit) const {
    const auto needle = normalized(query);
    std::vector<EditorCommandSearchResult> results;
    for (const auto& command : commands_) {
        const int score = matchScore(command, needle);
        if (score == 0) continue;
        const bool recent = std::find(recent_ids_.begin(), recent_ids_.end(), command.id) != recent_ids_.end();
        results.push_back({command, score + (recent ? 5 : 0), recent});
    }
    std::sort(results.begin(), results.end(), [](const auto& left, const auto& right) {
        if (left.score != right.score) return left.score > right.score;
        if (left.command.label != right.command.label) return left.command.label < right.command.label;
        return left.command.id < right.command.id;
    });
    if (results.size() > limit) results.resize(limit);
    return results;
}

bool EditorCommandPalette::recordAction(const std::string_view command_id) {
    const auto command = std::find_if(commands_.begin(), commands_.end(),
                                      [&](const auto& candidate) { return candidate.id == command_id; });
    if (command == commands_.end() || !command->enabled) return false;
    recent_ids_.erase(std::remove(recent_ids_.begin(), recent_ids_.end(), command_id), recent_ids_.end());
    recent_ids_.insert(recent_ids_.begin(), std::string(command_id));
    if (recent_ids_.size() > 20) recent_ids_.resize(20);
    return true;
}

std::vector<EditorCommandDescriptor> EditorCommandPalette::recentActions(const std::size_t limit) const {
    std::vector<EditorCommandDescriptor> result;
    for (const auto& id : recent_ids_) {
        const auto command = std::find_if(commands_.begin(), commands_.end(), [&](const auto& row) { return row.id == id; });
        if (command != commands_.end()) result.push_back(*command);
        if (result.size() == limit) break;
    }
    return result;
}

std::vector<EditorCommandDescriptor> EditorCommandPalette::shortcutDiscovery() const {
    std::vector<EditorCommandDescriptor> result;
    std::copy_if(commands_.begin(), commands_.end(), std::back_inserter(result),
                 [](const auto& command) { return !command.shortcut.empty(); });
    std::sort(result.begin(), result.end(), [](const auto& left, const auto& right) {
        return left.shortcut != right.shortcut ? left.shortcut < right.shortcut : left.id < right.id;
    });
    return result;
}

EditorCommandPalette buildGoldenLoopCommandPalette() {
    EditorCommandPalette palette;
    (void)palette.registerCommand({"project.create", "Create Project", "Project", {"new", "template"}, "Ctrl+Shift+N",
                                   "Startup shell is open.", "Create a project from a certified template."});
    (void)palette.registerCommand({"map.open", "Open Map", "Map", {"level", "canvas"}, "Ctrl+Alt+M",
                                   "A project is open.", "Open the unified Map authoring workspace."});
    (void)palette.registerCommand({"event.create", "Create Event", "Event", {"dialogue", "interaction"}, "Ctrl+Shift+E",
                                   "A writable map is open.", "Create an event on the active map."});
    (void)palette.registerCommand({"project.save", "Save Project", "Project", {"write", "persist"}, "Ctrl+S",
                                   "A project is open.", "Save changed project documents through their owners."});
    (void)palette.registerCommand({"playtest.current_map", "Playtest Current Map", "Playtest", {"run", "preview"}, "F6",
                                   "The active map has no blocking diagnostics.", "Launch a private current-map playtest overlay."});
    (void)palette.registerCommand({"project.health", "Open Project Health", "Diagnostics", {"validate", "fix"}, "Ctrl+Shift+H",
                                   "A project is open.", "Review blockers, warnings, and suggested fixes."});
    (void)palette.registerCommand({"export.validate", "Validate Export", "Export", {"package", "release"}, "Ctrl+Shift+B",
                                   "A project and export target are selected.", "Run export readiness checks without packaging."});
    return palette;
}

} // namespace urpg::editor
