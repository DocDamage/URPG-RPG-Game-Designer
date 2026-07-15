#include "editor/project/creator_checklist.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace urpg::editor {

namespace {

bool containsJsonFile(const std::filesystem::path& root) {
    std::error_code error;
    if (!std::filesystem::is_directory(root, error) || error) {
        return false;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root, error)) {
        if (error) {
            return false;
        }
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            return true;
        }
    }
    return false;
}

bool starterMapHasSpawn(const std::filesystem::path& project_root) {
    const auto maps = project_root / "content" / "maps";
    std::error_code error;
    if (!std::filesystem::is_directory(maps, error) || error) {
        return false;
    }
    for (const auto& entry : std::filesystem::directory_iterator(maps, error)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        std::ifstream input(entry.path(), std::ios::binary);
        const auto json = nlohmann::json::parse(input, nullptr, false);
        if (json.is_object() && json.contains("spawn") && json["spawn"].is_object()) {
            return true;
        }
    }
    return false;
}

bool mapHasAuthoredEvent(const std::filesystem::path& project_root) {
    const auto maps = project_root / "content" / "maps";
    std::error_code error;
    if (!std::filesystem::is_directory(maps, error) || error) {
        return false;
    }
    for (const auto& entry : std::filesystem::directory_iterator(maps, error)) {
        if (error || !entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        std::ifstream input(entry.path(), std::ios::binary);
        const auto json = nlohmann::json::parse(input, nullptr, false);
        if (!json.is_object()) {
            continue;
        }
        const auto events = json.find("events");
        if (events != json.end() && events->is_array() &&
            std::any_of(events->begin(), events->end(), [](const auto& event) {
                return event.is_object() && !event.value("event_id", "").empty();
            })) {
            return true;
        }
    }
    return false;
}

bool writeChecklistStateAtomically(const std::filesystem::path& path, const nlohmann::json& state,
                                   std::string* error) {
    std::error_code filesystem_error;
    std::filesystem::create_directories(path.parent_path(), filesystem_error);
    if (filesystem_error) {
        if (error) *error = filesystem_error.message();
        return false;
    }
    const auto temporary = path.parent_path() / ("." + path.filename().string() + ".tmp");
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        output << state.dump(2) << '\n';
        if (!output) {
            if (error) *error = "Unable to write the temporary creator checklist state.";
            std::filesystem::remove(temporary, filesystem_error);
            return false;
        }
    }
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        if (error) *error = "Unable to atomically publish the creator checklist state.";
        std::filesystem::remove(temporary, filesystem_error);
        return false;
    }
#else
    std::filesystem::rename(temporary, path, filesystem_error);
    if (filesystem_error) {
        if (error) *error = "Unable to atomically publish the creator checklist state: " + filesystem_error.message();
        std::filesystem::remove(temporary, filesystem_error);
        return false;
    }
#endif
    return true;
}

} // namespace

std::filesystem::path CreatorChecklist::statePath(const std::filesystem::path& project_root) {
    return project_root / ".urpg" / "creator" / "checklist.json";
}

CreatorChecklistSnapshot CreatorChecklist::inspect(const std::filesystem::path& project_root) const {
    CreatorChecklistSnapshot snapshot;
    std::ifstream state(statePath(project_root), std::ios::binary);
    const auto state_json = nlohmann::json::parse(state, nullptr, false);
    snapshot.dismissed = state_json.is_object() && state_json.value("dismissed", false);
    snapshot.items = {
        {"hero_art", "Choose hero art", containsJsonFile(project_root / "content" / "assets" / "manifests")},
        {"map", "Paint or edit the map", containsJsonFile(project_root / "content" / "maps")},
        {"player_start", "Place player start", starterMapHasSpawn(project_root)},
        {"npc_event", "Create an NPC event",
         containsJsonFile(project_root / "content" / "events") || mapHasAuthoredEvent(project_root)},
        {"dialogue", "Preview dialogue", containsJsonFile(project_root / "content" / "dialogue")},
        {"playtest", "Playtest", std::filesystem::is_regular_file(project_root / ".urpg" / "playtest" / "last_completed.json")},
        {"save", "Save", std::filesystem::is_regular_file(project_root / ".urpg" / "creator" / "last_manual_save.json")},
        {"validate", "Validate", std::filesystem::is_regular_file(project_root / ".urpg" / "reports" / "validation.json")},
    };
    return snapshot;
}

bool CreatorChecklist::setDismissed(const std::filesystem::path& project_root, bool dismissed, std::string* error) const {
    const auto path = statePath(project_root);
    return writeChecklistStateAtomically(
        path, nlohmann::json{{"schema", "urpg.creator_checklist.v1"}, {"dismissed", dismissed}}, error);
}

} // namespace urpg::editor
