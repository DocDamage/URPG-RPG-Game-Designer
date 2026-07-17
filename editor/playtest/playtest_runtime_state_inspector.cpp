#include "editor/playtest/playtest_runtime_state_inspector.h"

#include <algorithm>
#include <set>

namespace urpg::editor {

namespace {

bool safeId(std::string_view id) {
    return !id.empty() && id.find("..") == std::string_view::npos && id.find('\\') == std::string_view::npos;
}

bool validQuestState(const std::string& state) {
    static const std::set<std::string> allowed{"locked", "active", "completed", "failed", "hidden"};
    return allowed.contains(state);
}

nlohmann::json runtimeStateToJson(const PlaytestRuntimeState& state) {
    nlohmann::json entities = nlohmann::json::object();
    for (const auto& [id, entity] : state.entities)
        entities[id] = {{"type", entity.type}, {"fields", entity.fields}};
    nlohmann::json quests = nlohmann::json::object();
    for (const auto& [id, quest] : state.quests)
        quests[id] = {{"state", quest.state}, {"objectives", quest.objectives}};
    return {{"switches", state.switches}, {"variables", state.variables},
            {"self_switches", state.self_switches}, {"entities", std::move(entities)},
            {"quests", std::move(quests)}, {"inventory", state.inventory}};
}

std::string kindName(PlaytestDebugValueKind kind) {
    switch (kind) {
    case PlaytestDebugValueKind::Switch: return "switch";
    case PlaytestDebugValueKind::Variable: return "variable";
    case PlaytestDebugValueKind::SelfSwitch: return "self_switch";
    case PlaytestDebugValueKind::EntityField: return "entity";
    case PlaytestDebugValueKind::QuestState: return "quest";
    case PlaytestDebugValueKind::Inventory: return "inventory";
    }
    return "unknown";
}

std::optional<PlaytestDebugValueKind> parseKind(const std::string& kind) {
    if (kind == "switch") return PlaytestDebugValueKind::Switch;
    if (kind == "variable") return PlaytestDebugValueKind::Variable;
    if (kind == "self_switch") return PlaytestDebugValueKind::SelfSwitch;
    if (kind == "entity") return PlaytestDebugValueKind::EntityField;
    if (kind == "quest") return PlaytestDebugValueKind::QuestState;
    if (kind == "inventory") return PlaytestDebugValueKind::Inventory;
    return std::nullopt;
}

std::optional<PlaytestRuntimeState> runtimeStateFromJson(const nlohmann::json& value) {
    if (!value.is_object()) return std::nullopt;
    try {
        PlaytestRuntimeState state;
        state.switches = value.at("switches").get<std::map<std::string, bool>>();
        state.variables = value.at("variables").get<std::map<std::string, int64_t>>();
        state.self_switches = value.at("self_switches").get<std::map<std::string, bool>>();
        state.inventory = value.at("inventory").get<std::map<std::string, int64_t>>();
        for (const auto& [id, entity] : value.at("entities").items()) {
            state.entities.emplace(id, PlaytestEntityState{id, entity.value("type", ""),
                entity.at("fields").get<std::map<std::string, nlohmann::json>>()});
        }
        for (const auto& [id, quest] : value.at("quests").items()) {
            state.quests.emplace(id, PlaytestQuestRuntimeState{id, quest.value("state", ""),
                quest.at("objectives").get<std::map<std::string, std::string>>()});
        }
        return state;
    } catch (const nlohmann::json::exception&) {
        return std::nullopt;
    }
}

} // namespace

bool PlaytestRuntimeStateInspector::beginSession(std::string checkpoint_id, PlaytestRuntimeState state) {
    if (!safeId(checkpoint_id)) return false;
    package_state_ = state;
    current_ = std::move(state);
    checkpoints_.clear();
    checkpoints_.emplace(checkpoint_id, current_);
    active_checkpoint_id_ = std::move(checkpoint_id);
    watches_.clear();
    mutations_.clear();
    return true;
}

bool PlaytestRuntimeStateInspector::addCheckpoint(std::string checkpoint_id, PlaytestRuntimeState state) {
    if (!safeId(checkpoint_id) || checkpoints_.contains(checkpoint_id) || checkpoints_.size() >= kMaxCheckpoints)
        return false;
    checkpoints_.emplace(std::move(checkpoint_id), std::move(state));
    return true;
}

bool PlaytestRuntimeStateInspector::resetToCheckpoint(std::string_view checkpoint_id) {
    const auto found = checkpoints_.find(std::string(checkpoint_id));
    if (found == checkpoints_.end()) return false;
    current_ = found->second;
    active_checkpoint_id_ = found->first;
    mutations_.clear();
    return true;
}

bool PlaytestRuntimeStateInspector::watch(PlaytestDebugValueAddress address) {
    if (!safeId(address.id) || watches_.size() >= kMaxWatches ||
        std::find(watches_.begin(), watches_.end(), address) != watches_.end()) return false;
    watches_.push_back(std::move(address));
    return true;
}

std::vector<PlaytestWatchedValue> PlaytestRuntimeStateInspector::watchedValues() const {
    std::vector<PlaytestWatchedValue> result;
    result.reserve(watches_.size());
    for (const auto& address : watches_) {
        const auto value = read(current_, address);
        result.push_back({address, value.has_value(), isModified(address), value.value_or(nullptr)});
    }
    return result;
}

PlaytestDebugEditResult PlaytestRuntimeStateInspector::applyTemporaryEdit(
    std::string mutation_id, PlaytestDebugValueAddress address, nlohmann::json value) {
    if (!safeId(mutation_id) || mutations_.size() >= kMaxMutations ||
        std::any_of(mutations_.begin(), mutations_.end(), [&](const auto& mutation) {
            return mutation.mutation_id == mutation_id;
        })) return {false, "playtest_debug_mutation_invalid", "Temporary edit requires a unique bounded ID."};
    const auto before = read(current_, address);
    if (!before) return {false, "playtest_debug_address_missing", "Debug state address does not exist."};
    const auto written = write(address, value);
    if (!written.success) return written;
    mutations_.push_back({std::move(mutation_id), std::move(address), *before, std::move(value), true, false});
    return {true, "playtest_debug_mutation_applied", "Temporary edit applied to disposable playtest state."};
}

void PlaytestRuntimeStateInspector::bindLiveBridge(playtest::PlaytestRuntimeStateBridge* bridge) {
    live_bridge_ = bridge;
    live_revision_ = 0;
    next_live_control_id_ = 1;
    live_connected_ = false;
    last_live_control_code_.clear();
}

bool PlaytestRuntimeStateInspector::refreshLive() {
    if (live_bridge_ == nullptr) {
        live_connected_ = false;
        return false;
    }
    std::string diagnostic;
    auto live = live_bridge_->readAfter(live_revision_, &diagnostic);
    if (!diagnostic.empty()) {
        last_live_control_code_ = std::move(diagnostic);
        return false;
    }
    if (!live) return live_connected_;
    auto state = runtimeStateFromJson(live->state);
    auto packageState = runtimeStateFromJson(live->package_state);
    if (!state || !packageState) {
        last_live_control_code_ = "runtime_state_snapshot_type_invalid";
        return false;
    }
    std::vector<PlaytestDebugMutation> mutations;
    for (const auto& value : live->mutations) {
        const auto kind = parseKind(value.value("kind", ""));
        if (!kind || value.value("temporary", false) != true || value.value("packaged", true) != false) {
            last_live_control_code_ = "runtime_state_snapshot_mutation_invalid";
            return false;
        }
        mutations.push_back({value.value("mutation_id", ""),
                             {*kind, value.value("id", ""), value.value("field", "")},
                             value.value("before", nlohmann::json{}),
                             value.value("after", nlohmann::json{}), true, false});
    }
    current_ = std::move(*state);
    package_state_ = std::move(*packageState);
    active_checkpoint_id_ = live->checkpoint_id;
    mutations_ = std::move(mutations);
    live_revision_ = live->revision;
    live_connected_ = true;
    last_live_control_code_ = live->last_control_code.empty() ? "runtime_state_snapshot_acknowledged"
                                                              : live->last_control_code;
    return true;
}

bool PlaytestRuntimeStateInspector::requestLiveTemporaryEdit(
    std::string mutation_id, PlaytestDebugValueAddress address, nlohmann::json value) {
    if (live_bridge_ == nullptr || !live_connected_) {
        last_live_control_code_ = "runtime_state_bridge_not_connected";
        return false;
    }
    playtest::RuntimeStateControl control;
    control.control_id = next_live_control_id_++;
    control.expected_revision = live_revision_;
    control.action = playtest::RuntimeStateControlAction::TemporaryEdit;
    control.mutation_id = std::move(mutation_id);
    control.kind = kindName(address.kind);
    control.id = std::move(address.id);
    control.field = std::move(address.field);
    control.value = std::move(value);
    std::string diagnostic;
    const bool queued = live_bridge_->appendControl(control, &diagnostic);
    last_live_control_code_ = queued ? "runtime_state_edit_queued" : std::move(diagnostic);
    return queued;
}

bool PlaytestRuntimeStateInspector::requestLiveReset(std::string checkpoint_id) {
    if (live_bridge_ == nullptr || !live_connected_) {
        last_live_control_code_ = "runtime_state_bridge_not_connected";
        return false;
    }
    playtest::RuntimeStateControl control;
    control.control_id = next_live_control_id_++;
    control.expected_revision = live_revision_;
    control.action = playtest::RuntimeStateControlAction::ResetCheckpoint;
    control.checkpoint_id = std::move(checkpoint_id);
    std::string diagnostic;
    const bool queued = live_bridge_->appendControl(control, &diagnostic);
    last_live_control_code_ = queued ? "runtime_state_reset_queued" : std::move(diagnostic);
    return queued;
}

std::optional<nlohmann::json> PlaytestRuntimeStateInspector::read(
    const PlaytestRuntimeState& state, const PlaytestDebugValueAddress& address) const {
    if (address.kind == PlaytestDebugValueKind::Switch) {
        const auto found = state.switches.find(address.id);
        if (found != state.switches.end()) return found->second;
    } else if (address.kind == PlaytestDebugValueKind::Variable) {
        const auto found = state.variables.find(address.id);
        if (found != state.variables.end()) return found->second;
    } else if (address.kind == PlaytestDebugValueKind::SelfSwitch) {
        const auto found = state.self_switches.find(address.id);
        if (found != state.self_switches.end()) return found->second;
    } else if (address.kind == PlaytestDebugValueKind::Inventory) {
        const auto found = state.inventory.find(address.id);
        if (found != state.inventory.end()) return found->second;
    } else if (address.kind == PlaytestDebugValueKind::QuestState) {
        const auto found = state.quests.find(address.id);
        if (found != state.quests.end()) {
            if (address.field.empty() || address.field == "state") return found->second.state;
            const auto objective = found->second.objectives.find(address.field);
            if (objective != found->second.objectives.end()) return objective->second;
        }
    } else {
        const auto found = state.entities.find(address.id);
        if (found != state.entities.end()) {
            const auto field = found->second.fields.find(address.field);
            if (field != found->second.fields.end()) return field->second;
        }
    }
    return std::nullopt;
}

PlaytestDebugEditResult PlaytestRuntimeStateInspector::write(const PlaytestDebugValueAddress& address,
                                                             const nlohmann::json& value) {
    if (address.kind == PlaytestDebugValueKind::Switch || address.kind == PlaytestDebugValueKind::SelfSwitch) {
        if (!value.is_boolean()) return {false, "playtest_debug_type_mismatch", "Switch edits require a boolean."};
        auto& target = address.kind == PlaytestDebugValueKind::Switch ? current_.switches : current_.self_switches;
        target[address.id] = value.get<bool>();
    } else if (address.kind == PlaytestDebugValueKind::Variable || address.kind == PlaytestDebugValueKind::Inventory) {
        if (!value.is_number_integer())
            return {false, "playtest_debug_type_mismatch", "Numeric debug edits require an integer."};
        const auto number = value.get<int64_t>();
        if (number < -1'000'000'000LL || number > 1'000'000'000LL ||
            (address.kind == PlaytestDebugValueKind::Inventory && number < 0))
            return {false, "playtest_debug_value_out_of_range", "Debug edit exceeds its safe numeric range."};
        if (address.kind == PlaytestDebugValueKind::Variable) current_.variables[address.id] = number;
        else current_.inventory[address.id] = number;
    } else if (address.kind == PlaytestDebugValueKind::QuestState) {
        if (!value.is_string() || !validQuestState(value.get<std::string>()))
            return {false, "playtest_debug_quest_state_invalid", "Quest state edit is unsupported."};
        auto& quest = current_.quests[address.id];
        if (address.field.empty() || address.field == "state") quest.state = value.get<std::string>();
        else quest.objectives[address.field] = value.get<std::string>();
    } else {
        if (value.is_array() || value.is_object() || value.is_null())
            return {false, "playtest_debug_entity_value_unsafe", "Entity edits are limited to scalar fields."};
        current_.entities[address.id].fields[address.field] = value;
    }
    return {true, "playtest_debug_value_written", "Disposable value updated."};
}

bool PlaytestRuntimeStateInspector::isModified(const PlaytestDebugValueAddress& address) const {
    return std::any_of(mutations_.begin(), mutations_.end(), [&](const auto& mutation) {
        return mutation.address == address;
    });
}

nlohmann::json PlaytestRuntimeStateInspector::snapshotView() const {
    auto view = runtimeStateToJson(current_);
    view["checkpoint_id"] = active_checkpoint_id_;
    view["debug_mutations"] = nlohmann::json::array();
    for (const auto& mutation : mutations_) {
        view["debug_mutations"].push_back({{"mutation_id", mutation.mutation_id},
            {"kind", kindName(mutation.address.kind)}, {"id", mutation.address.id},
            {"field", mutation.address.field}, {"before", mutation.before}, {"after", mutation.after},
            {"temporary", true}, {"packaged", false}});
    }
    return view;
}

nlohmann::json PlaytestRuntimeStateInspector::exportDisposableOverlay() const {
    return {{"schema", "urpg.playtest_debug_overlay.v1"}, {"disposable", true}, {"packaged", false},
            {"checkpoint_id", active_checkpoint_id_}, {"state", runtimeStateToJson(current_)},
            {"package_state_unchanged", runtimeStateToJson(package_state_)},
            {"mutations", snapshotView()["debug_mutations"]}};
}

} // namespace urpg::editor
