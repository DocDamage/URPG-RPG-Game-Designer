#include "engine/core/codex/bestiary_registry.h"

#include <algorithm>

namespace urpg::codex {

void BestiaryRegistry::addEntry(BestiaryEntry entry) {
    entries_.push_back(std::move(entry));
}

void BestiaryRegistry::markSeen(const std::string& enemy_id) {
    auto state = stateFor(enemy_id);
    state.seen = true;
    states_.push_back({enemy_id, state});
}

void BestiaryRegistry::markScanned(const std::string& enemy_id) {
    auto state = stateFor(enemy_id);
    state.scanned = true;
    states_.push_back({enemy_id, state});
}

void BestiaryRegistry::markDefeated(const std::string& enemy_id) {
    auto state = stateFor(enemy_id);
    state.defeated = true;
    states_.push_back({enemy_id, state});
}

BestiaryState BestiaryRegistry::stateFor(const std::string& enemy_id) const {
    for (auto it = states_.rbegin(); it != states_.rend(); ++it) {
        if (it->first == enemy_id) {
            return it->second;
        }
    }
    return {};
}

double BestiaryRegistry::completionRatio() const {
    if (entries_.empty()) {
        return 1.0;
    }
    int completed = 0;
    for (const auto& entry : entries_) {
        const auto state = stateFor(entry.enemy_id);
        if (state.seen && state.scanned && state.defeated) {
            ++completed;
        }
    }
    return static_cast<double>(completed) / static_cast<double>(entries_.size());
}

std::vector<BestiaryDiagnostic> BestiaryRegistry::validate(const std::vector<std::string>& known_enemy_ids) const {
    std::vector<BestiaryDiagnostic> diagnostics;
    for (const auto& entry : entries_) {
        if (std::ranges::find(known_enemy_ids, entry.enemy_id) == known_enemy_ids.end()) {
            diagnostics.push_back({"missing_enemy_reference", "bestiary entry references a missing enemy"});
        }
    }
    return diagnostics;
}

nlohmann::json BestiaryRegistry::toJson() const {
    nlohmann::json json;
    json["entries"] = nlohmann::json::array();
    for (const auto& entry : entries_) {
        json["entries"].push_back({{"enemyId", entry.enemy_id}, {"name", entry.name}, {"weaknesses", entry.weaknesses}, {"drops", entry.drops}, {"loreKey", entry.lore_key}});
    }
    json["states"] = nlohmann::json::array();
    for (const auto& [enemy_id, state] : states_) {
        json["states"].push_back({{"enemyId", enemy_id}, {"seen", state.seen}, {"scanned", state.scanned}, {"defeated", state.defeated}});
    }
    return json;
}

BestiaryRegistry BestiaryRegistry::fromJson(const nlohmann::json& json) {
    BestiaryRegistry registry;
    for (const auto& entry : json.value("entries", nlohmann::json::array())) {
        registry.addEntry({entry.value("enemyId", ""), entry.value("name", ""), entry.value("weaknesses", std::vector<std::string>{}), entry.value("drops", std::vector<std::string>{}), entry.value("loreKey", "")});
    }
    for (const auto& state : json.value("states", nlohmann::json::array())) {
        registry.states_.push_back({state.value("enemyId", ""), {state.value("seen", false), state.value("scanned", false), state.value("defeated", false)}});
    }
    return registry;
}

} // namespace urpg::codex
