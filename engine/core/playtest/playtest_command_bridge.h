#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

namespace urpg::playtest {

struct EditorCommand {
    uint64_t command_id = 0;
    std::string command;
    std::string map_id;
    int32_t tile_x = -1;
    int32_t tile_y = -1;
    std::string selected_object_id;
    uint64_t expected_revision = 0;
    std::string state_policy;
};

struct CommandPollResult {
    size_t processed = 0;
    size_t applied = 0;
    size_t rejected = 0;
    bool io_error = false;
    std::string error;
};

// Bounded reader for the editor-owned append-only playtest command stream.
// Complete lines are consumed once and every consumed command is acknowledged.
class PlaytestCommandBridge {
  public:
    using Handler = std::function<bool(const EditorCommand&)>;

    explicit PlaytestCommandBridge(std::filesystem::path session_directory);
    CommandPollResult poll(const Handler& handler, size_t max_commands = 8);
    uintmax_t consumedBytes() const { return consumed_bytes_; }

  private:
    bool appendAcknowledgement(const EditorCommand& command, bool applied, const std::string& code,
                               std::string* error) const;

    std::filesystem::path session_directory_;
    uintmax_t consumed_bytes_ = 0;
};

} // namespace urpg::playtest
