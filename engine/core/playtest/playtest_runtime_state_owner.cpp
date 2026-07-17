#include "engine/core/playtest/playtest_runtime_state_owner.h"

#include <algorithm>
#include <set>

namespace urpg::playtest {
namespace {

bool safeId(const std::string& value) {
    return !value.empty() && value.find("..") == std::string::npos && value.find('\\') == std::string::npos;
}

bool validQuestState(const std::string& value) {
    static const std::set<std::string> values{"locked", "active", "completed", "failed", "hidden"};
    return values.contains(value);
}

} // namespace

bool PlaytestRuntimeStateOwner::start(std::string session_id, std::string checkpoint_id,
                                      nlohmann::json state, std::string* diagnostic) {
    if (session_id.empty() || !safeId(checkpoint_id) || !validState(state)) {
        if (diagnostic) *diagnostic = "runtime_state_owner_start_invalid";
        return false;
    }
    revision_ = 1;
    live_base_state_ = state;
    checkpoints_.clear();
    checkpoints_.emplace(checkpoint_id, state);
    snapshot_ = {revision_, std::move(session_id), std::move(checkpoint_id), true, false,
                 state, state, nlohmann::json::array(), 0, {}};
    return publish(diagnostic);
}

bool PlaytestRuntimeStateOwner::synchronize(nlohmann::json runtime_state, std::string* diagnostic) {
    if (revision_ == 0 || !validState(runtime_state)) {
        if (diagnostic) *diagnostic = "runtime_state_owner_sync_invalid";
        return false;
    }
    if (runtime_state == live_base_state_) return true;
    live_base_state_ = std::move(runtime_state);
    auto next = live_base_state_;
    for (const auto& mutation : snapshot_.mutations) {
        RuntimeStateControl control;
        control.kind = mutation.value("kind", "");
        control.id = mutation.value("id", "");
        control.field = mutation.value("field", "");
        control.value = mutation.value("after", nlohmann::json{});
        if (!writeValue(next, control)) {
            if (diagnostic) *diagnostic = "runtime_state_owner_mutation_replay_failed";
            return false;
        }
    }
    snapshot_.state = std::move(next);
    snapshot_.revision = ++revision_;
    return publish(diagnostic);
}

bool PlaytestRuntimeStateOwner::poll(std::string* diagnostic) {
    if (revision_ == 0) return false;
    bool handledControl = false;
    const auto result = bridge_.pollControls([&](const RuntimeStateControl& control) {
        handledControl = true;
        snapshot_.last_control_id = control.control_id;
        if (control.expected_revision != revision_) {
            snapshot_.last_control_code = "runtime_state_control_revision_rejected";
            return false;
        }
        if (control.action == RuntimeStateControlAction::ResetCheckpoint) {
            const auto found = checkpoints_.find(control.checkpoint_id);
            if (found == checkpoints_.end()) {
                snapshot_.last_control_code = "runtime_state_checkpoint_missing";
                return false;
            }
            live_base_state_ = found->second;
            snapshot_.state = found->second;
            snapshot_.checkpoint_id = found->first;
            snapshot_.mutations = nlohmann::json::array();
            snapshot_.last_control_code = "runtime_state_checkpoint_reset_applied";
            return true;
        }
        const bool applied = applyEdit(control);
        snapshot_.last_control_code = applied ? "runtime_state_temporary_edit_applied"
                                              : "runtime_state_temporary_edit_rejected";
        return applied;
    });
    if (result.io_error) {
        if (diagnostic) *diagnostic = result.error;
        return false;
    }
    if (!handledControl) return true;
    snapshot_.revision = ++revision_;
    return publish(diagnostic);
}

bool PlaytestRuntimeStateOwner::applyEdit(const RuntimeStateControl& control) {
    if (!safeId(control.mutation_id) || snapshot_.mutations.size() >= 4096 ||
        std::any_of(snapshot_.mutations.begin(), snapshot_.mutations.end(), [&](const auto& mutation) {
            return mutation.value("mutation_id", "") == control.mutation_id;
        })) return false;
    nlohmann::json before;
    if (!writeValue(snapshot_.state, control, &before)) return false;
    snapshot_.mutations.push_back({{"mutation_id", control.mutation_id}, {"kind", control.kind},
                                   {"id", control.id}, {"field", control.field}, {"before", before},
                                   {"after", control.value}, {"temporary", true}, {"packaged", false}});
    return true;
}

bool PlaytestRuntimeStateOwner::writeValue(nlohmann::json& state, const RuntimeStateControl& control,
                                           nlohmann::json* before) const {
    if (!safeId(control.id)) return false;
    nlohmann::json* target = nullptr;
    if (control.kind == "switch" || control.kind == "self_switch") {
        if (!control.value.is_boolean()) return false;
        const char* family = control.kind == "switch" ? "switches" : "self_switches";
        if (!state[family].contains(control.id)) return false;
        target = &state[family][control.id];
    } else if (control.kind == "variable" || control.kind == "inventory") {
        if (!control.value.is_number_integer()) return false;
        const auto number = control.value.get<int64_t>();
        if (number < -1'000'000'000LL || number > 1'000'000'000LL ||
            (control.kind == "inventory" && number < 0)) return false;
        const char* family = control.kind == "variable" ? "variables" : "inventory";
        if (!state[family].contains(control.id)) return false;
        target = &state[family][control.id];
    } else if (control.kind == "entity") {
        if (control.field.empty() || control.value.is_null() || control.value.is_array() || control.value.is_object() ||
            !state["entities"].contains(control.id) ||
            !state["entities"][control.id]["fields"].contains(control.field)) return false;
        target = &state["entities"][control.id]["fields"][control.field];
    } else if (control.kind == "quest") {
        if (!control.value.is_string() || !state["quests"].contains(control.id)) return false;
        if (control.field.empty() || control.field == "state") {
            if (!validQuestState(control.value.get<std::string>())) return false;
            target = &state["quests"][control.id]["state"];
        } else {
            if (!state["quests"][control.id]["objectives"].contains(control.field)) return false;
            target = &state["quests"][control.id]["objectives"][control.field];
        }
    } else {
        return false;
    }
    if (before) *before = *target;
    *target = control.value;
    return true;
}

bool PlaytestRuntimeStateOwner::publish(std::string* diagnostic) {
    return bridge_.publish(snapshot_, diagnostic);
}

bool PlaytestRuntimeStateOwner::validState(const nlohmann::json& state) {
    if (!state.is_object()) return false;
    for (const char* family : {"switches", "variables", "self_switches", "entities", "quests", "inventory"}) {
        if (!state.contains(family) || !state[family].is_object()) return false;
    }
    return true;
}

} // namespace urpg::playtest
