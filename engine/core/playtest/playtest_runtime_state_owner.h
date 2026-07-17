#pragma once

#include "engine/core/playtest/playtest_runtime_state_bridge.h"

#include <map>

namespace urpg::playtest {

// Runtime authority for disposable inspector state. Package state is captured
// once and never mutated; live synchronization reapplies admitted debug edits
// only to the private overlay.
class PlaytestRuntimeStateOwner {
public:
    explicit PlaytestRuntimeStateOwner(std::filesystem::path session_directory)
        : bridge_(std::move(session_directory)) {}

    bool start(std::string session_id, std::string checkpoint_id, nlohmann::json state,
               std::string* diagnostic = nullptr);
    bool synchronize(nlohmann::json runtime_state, std::string* diagnostic = nullptr);
    bool poll(std::string* diagnostic = nullptr);

    uint64_t revision() const { return revision_; }
    const LiveRuntimeStateSnapshot& snapshot() const { return snapshot_; }

private:
    bool applyEdit(const RuntimeStateControl& control);
    bool writeValue(nlohmann::json& state, const RuntimeStateControl& control,
                    nlohmann::json* before = nullptr) const;
    bool publish(std::string* diagnostic);
    static bool validState(const nlohmann::json& state);

    PlaytestRuntimeStateBridge bridge_;
    LiveRuntimeStateSnapshot snapshot_;
    nlohmann::json live_base_state_ = nlohmann::json::object();
    std::map<std::string, nlohmann::json> checkpoints_;
    uint64_t revision_ = 0;
};

} // namespace urpg::playtest
