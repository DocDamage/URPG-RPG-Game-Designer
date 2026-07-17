#pragma once

#include <filesystem>
#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace urpg::playtest {

struct LiveRuntimeStateSnapshot {
    uint64_t revision = 0;
    std::string session_id;
    std::string checkpoint_id;
    bool disposable = true;
    bool packaged = false;
    nlohmann::json state = nlohmann::json::object();
    nlohmann::json package_state = nlohmann::json::object();
    nlohmann::json mutations = nlohmann::json::array();
    uint64_t last_control_id = 0;
    std::string last_control_code;
};

enum class RuntimeStateControlAction : uint8_t { TemporaryEdit, ResetCheckpoint };

struct RuntimeStateControl {
    uint64_t control_id = 0;
    uint64_t expected_revision = 0;
    RuntimeStateControlAction action = RuntimeStateControlAction::TemporaryEdit;
    std::string mutation_id;
    std::string kind;
    std::string id;
    std::string field;
    nlohmann::json value;
    std::string checkpoint_id;
};

struct RuntimeStateControlPollResult {
    size_t processed = 0;
    size_t applied = 0;
    size_t rejected = 0;
    bool io_error = false;
    std::string error;
};

class PlaytestRuntimeStateBridge {
public:
    using ControlHandler = std::function<bool(const RuntimeStateControl&)>;

    explicit PlaytestRuntimeStateBridge(std::filesystem::path session_directory)
        : session_directory_(std::move(session_directory)) {}

    bool publish(const LiveRuntimeStateSnapshot& snapshot, std::string* diagnostic = nullptr) const;
    std::optional<LiveRuntimeStateSnapshot> readAfter(uint64_t revision,
                                                      std::string* diagnostic = nullptr) const;
    bool appendControl(const RuntimeStateControl& control, std::string* diagnostic = nullptr) const;
    RuntimeStateControlPollResult pollControls(const ControlHandler& handler, size_t max_controls = 8);

private:
    std::filesystem::path session_directory_;
    uintmax_t consumed_control_bytes_ = 0;
};

} // namespace urpg::playtest
