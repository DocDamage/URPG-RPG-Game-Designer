#pragma once

#include "engine/core/playtest/playtest_runtime_state_bridge.h"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

struct PlaytestEntityState {
    std::string id;
    std::string type;
    std::map<std::string, nlohmann::json> fields;
};

struct PlaytestQuestRuntimeState {
    std::string id;
    std::string state;
    std::map<std::string, std::string> objectives;
};

struct PlaytestRuntimeState {
    std::map<std::string, bool> switches;
    std::map<std::string, int64_t> variables;
    std::map<std::string, bool> self_switches;
    std::map<std::string, PlaytestEntityState> entities;
    std::map<std::string, PlaytestQuestRuntimeState> quests;
    std::map<std::string, int64_t> inventory;
};

enum class PlaytestDebugValueKind : uint8_t { Switch, Variable, SelfSwitch, EntityField, QuestState, Inventory };

struct PlaytestDebugValueAddress {
    PlaytestDebugValueKind kind = PlaytestDebugValueKind::Switch;
    std::string id;
    std::string field;

    bool operator==(const PlaytestDebugValueAddress&) const = default;
};

struct PlaytestDebugMutation {
    std::string mutation_id;
    PlaytestDebugValueAddress address;
    nlohmann::json before;
    nlohmann::json after;
    bool temporary = true;
    bool packaged = false;
};

struct PlaytestWatchedValue {
    PlaytestDebugValueAddress address;
    bool available = false;
    bool debug_modified = false;
    nlohmann::json value;
};

struct PlaytestDebugEditResult {
    bool success = false;
    std::string code;
    std::string message;
};

class PlaytestRuntimeStateInspector {
public:
    bool beginSession(std::string checkpoint_id, PlaytestRuntimeState state);
    bool addCheckpoint(std::string checkpoint_id, PlaytestRuntimeState state);
    bool resetToCheckpoint(std::string_view checkpoint_id);

    bool watch(PlaytestDebugValueAddress address);
    std::vector<PlaytestWatchedValue> watchedValues() const;
    PlaytestDebugEditResult applyTemporaryEdit(std::string mutation_id, PlaytestDebugValueAddress address,
                                               nlohmann::json value);
    void bindLiveBridge(playtest::PlaytestRuntimeStateBridge* bridge);
    bool refreshLive();
    bool requestLiveTemporaryEdit(std::string mutation_id, PlaytestDebugValueAddress address,
                                  nlohmann::json value);
    bool requestLiveReset(std::string checkpoint_id);

    const PlaytestRuntimeState& state() const { return current_; }
    const PlaytestRuntimeState& packageState() const { return package_state_; }
    const std::vector<PlaytestDebugMutation>& mutations() const { return mutations_; }
    std::string_view activeCheckpointId() const { return active_checkpoint_id_; }
    nlohmann::json snapshotView() const;
    nlohmann::json exportDisposableOverlay() const;
    bool liveConnected() const { return live_connected_; }
    uint64_t liveRevision() const { return live_revision_; }
    const std::string& lastLiveControlCode() const { return last_live_control_code_; }

    static constexpr std::size_t kMaxCheckpoints = 32;
    static constexpr std::size_t kMaxWatches = 256;
    static constexpr std::size_t kMaxMutations = 4096;

private:
    std::optional<nlohmann::json> read(const PlaytestRuntimeState& state,
                                       const PlaytestDebugValueAddress& address) const;
    PlaytestDebugEditResult write(const PlaytestDebugValueAddress& address, const nlohmann::json& value);
    bool isModified(const PlaytestDebugValueAddress& address) const;

    PlaytestRuntimeState package_state_;
    PlaytestRuntimeState current_;
    std::map<std::string, PlaytestRuntimeState> checkpoints_;
    std::string active_checkpoint_id_;
    std::vector<PlaytestDebugValueAddress> watches_;
    std::vector<PlaytestDebugMutation> mutations_;
    playtest::PlaytestRuntimeStateBridge* live_bridge_ = nullptr;
    uint64_t live_revision_ = 0;
    uint64_t next_live_control_id_ = 1;
    bool live_connected_ = false;
    std::string last_live_control_code_;
};

} // namespace urpg::editor
