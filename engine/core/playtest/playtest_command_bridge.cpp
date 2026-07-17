#include "engine/core/playtest/playtest_command_bridge.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace urpg::playtest {
namespace {
constexpr size_t kMaximumCommandBytes = 64U * 1024U;

bool safeMapId(const std::string& value) {
    const std::filesystem::path path(value);
    return !value.empty() && !path.is_absolute() && path.filename().string() == value && value != "." && value != "..";
}
} // namespace

PlaytestCommandBridge::PlaytestCommandBridge(std::filesystem::path session_directory)
    : session_directory_(std::move(session_directory)) {}

CommandPollResult PlaytestCommandBridge::poll(const Handler& handler, const size_t max_commands) {
    CommandPollResult result;
    if (!handler || max_commands == 0 || session_directory_.empty()) return result;
    const auto commandsPath = session_directory_ / "editor_commands.jsonl";
    std::error_code filesystemError;
    if (!std::filesystem::is_regular_file(commandsPath, filesystemError)) {
        if (filesystemError) {
            result.io_error = true;
            result.error = filesystemError.message();
        }
        return result;
    }
    const auto fileSize = std::filesystem::file_size(commandsPath, filesystemError);
    if (filesystemError) {
        result.io_error = true;
        result.error = filesystemError.message();
        return result;
    }
    if (consumed_bytes_ > fileSize) consumed_bytes_ = 0;
    std::ifstream input(commandsPath, std::ios::binary);
    if (!input) {
        result.io_error = true;
        result.error = "Could not open the editor command stream.";
        return result;
    }
    bool hasTrailingNewline = fileSize == 0;
    if (fileSize > 0) {
        input.seekg(static_cast<std::streamoff>(fileSize - 1));
        hasTrailingNewline = input.get() == '\n';
        input.clear();
    }
    input.seekg(static_cast<std::streamoff>(consumed_bytes_));
    while (result.processed < max_commands) {
        const auto lineStart = consumed_bytes_;
        std::string line;
        if (!std::getline(input, line)) break;
        const auto next = input.tellg();
        consumed_bytes_ = next < 0 ? fileSize : static_cast<uintmax_t>(next);
        // A writer may still be appending the last line. Leave it for the next
        // poll rather than treating a partial JSON value as rejected.
        if (consumed_bytes_ == fileSize && !hasTrailingNewline) {
            consumed_bytes_ = lineStart;
            break;
        }
        ++result.processed;
        EditorCommand command;
        std::string code = "playtest_command_invalid";
        bool valid = line.size() <= kMaximumCommandBytes;
        const auto json = valid ? nlohmann::json::parse(line, nullptr, false) : nlohmann::json{};
        valid = valid && json.is_object();
        if (valid) {
            command.command_id = json.value("command_id", uint64_t{0});
            command.command = json.value("command", "");
            command.map_id = json.value("map_id", "");
            command.tile_x = json.value("tile_x", -1);
            command.tile_y = json.value("tile_y", -1);
            command.selected_object_id = json.value("selected_object_id", "");
            command.expected_revision = json.value("expected_revision", uint64_t{0});
            command.state_policy = json.value("state_policy", "");
            const bool teleport = command.command == "teleport_here" && command.tile_x >= 0 && command.tile_y >= 0;
            const bool reload = command.command == "hot_reload_map" && command.state_policy == "reset_affected";
            valid = json.value("version", 0) == 1 && command.command_id > 0 && safeMapId(command.map_id) &&
                    (teleport || reload);
        }
        bool applied = false;
        if (valid) {
            applied = handler(command);
            code = applied ? "playtest_command_applied" : "playtest_command_rejected";
        }
        std::string acknowledgementError;
        if (!appendAcknowledgement(command, applied, code, &acknowledgementError)) {
            result.io_error = true;
            result.error = acknowledgementError;
            return result;
        }
        if (applied) ++result.applied;
        else ++result.rejected;
    }
    return result;
}

bool PlaytestCommandBridge::appendAcknowledgement(const EditorCommand& command, const bool applied,
                                                  const std::string& code, std::string* error) const {
    std::ofstream output(session_directory_ / "runtime_acknowledgements.jsonl", std::ios::app | std::ios::binary);
    if (!output) {
        if (error) *error = "Could not open the runtime acknowledgement stream.";
        return false;
    }
    output << nlohmann::json{{"version", 1}, {"command_id", command.command_id}, {"command", command.command},
                             {"status", applied ? "applied" : "rejected"}, {"code", code},
                             {"map_id", command.map_id}, {"tile_x", command.tile_x}, {"tile_y", command.tile_y},
                             {"selected_object_id", command.selected_object_id},
                             {"expected_revision", command.expected_revision},
                             {"state_policy", command.state_policy}}.dump() << '\n';
    if (!output) {
        if (error) *error = "Could not write the runtime acknowledgement stream.";
        return false;
    }
    return true;
}

} // namespace urpg::playtest
