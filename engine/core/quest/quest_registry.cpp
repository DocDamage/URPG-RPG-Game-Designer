#include "engine/core/quest/quest_registry.h"

#include <algorithm>

namespace urpg::quest {

namespace {

bool hasValue(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

bool conditionMet(const QuestCondition& condition, const QuestWorldState& world) {
    if (condition.type == "switch") {
        const auto it = world.switches.find(condition.id);
        return it != world.switches.end() && it->second;
    }
    if (condition.type == "variable") {
        const auto it = world.variables.find(condition.id);
        return it != world.variables.end() && it->second >= condition.value;
    }
    if (condition.type == "item") {
        return hasValue(world.items, condition.id);
    }
    if (condition.type == "battle") {
        return hasValue(world.battles, condition.id);
    }
    if (condition.type == "dialogue_choice") {
        return hasValue(world.dialogue_choices, condition.id);
    }
    if (condition.type == "reputation") {
        const auto it = world.reputation.find(condition.id);
        return it != world.reputation.end() && it->second >= condition.value;
    }
    return false;
}

nlohmann::json conditionToJson(const QuestCondition& condition) {
    return {{"type", condition.type}, {"id", condition.id}, {"value", condition.value}};
}

QuestCondition conditionFromJson(const nlohmann::json& json) {
    return {json.value("type", ""), json.value("id", ""), json.value("value", 0)};
}

} // namespace

std::string ToString(ObjectiveState state) {
    switch (state) {
    case ObjectiveState::Locked: return "locked";
    case ObjectiveState::Active: return "active";
    case ObjectiveState::Completed: return "completed";
    case ObjectiveState::Failed: return "failed";
    case ObjectiveState::Hidden: return "hidden";
    }
    return "locked";
}

ObjectiveState ObjectiveStateFromString(const std::string& value) {
    if (value == "active") return ObjectiveState::Active;
    if (value == "completed") return ObjectiveState::Completed;
    if (value == "failed") return ObjectiveState::Failed;
    if (value == "hidden") return ObjectiveState::Hidden;
    return ObjectiveState::Locked;
}

bool QuestRegistry::registerQuest(QuestDefinition definition) {
    if (definition.id.empty() || quests_.contains(definition.id)) {
        return false;
    }
    quests_[definition.id] = std::move(definition);
    return true;
}

bool QuestRegistry::evaluateObjective(const std::string& quest_id, const std::string& objective_id,
                                      const QuestWorldState& world, const std::string& timestamp) {
    auto quest_it = quests_.find(quest_id);
    if (quest_it == quests_.end()) {
        return false;
    }
    for (auto& objective : quest_it->second.objectives) {
        if (objective.id != objective_id || objective.state == ObjectiveState::Completed ||
            objective.state == ObjectiveState::Failed) {
            continue;
        }
        const bool ready = std::all_of(objective.dependencies.begin(), objective.dependencies.end(),
                                       [&](const QuestCondition& condition) { return conditionMet(condition, world); });
        objective.state = ready ? ObjectiveState::Completed : ObjectiveState::Active;
        objective.updated_at = timestamp;
        return ready;
    }
    return false;
}

const QuestDefinition* QuestRegistry::findQuest(const std::string& quest_id) const {
    const auto it = quests_.find(quest_id);
    return it == quests_.end() ? nullptr : &it->second;
}

nlohmann::json QuestRegistry::serialize() const {
    nlohmann::json quests = nlohmann::json::array();
    for (const auto& [id, definition] : quests_) {
        nlohmann::json objectives = nlohmann::json::array();
        for (const auto& objective : definition.objectives) {
            nlohmann::json dependencies = nlohmann::json::array();
            for (const auto& dependency : objective.dependencies) {
                dependencies.push_back(conditionToJson(dependency));
            }
            objectives.push_back({
                {"id", objective.id},
                {"state", ToString(objective.state)},
                {"updated_at", objective.updated_at},
                {"dependencies", dependencies},
            });
        }
        quests.push_back({{"id", id}, {"objectives", objectives}});
    }
    return {{"schema_version", "urpg.quest_registry.v1"}, {"quests", quests}};
}

QuestRegistry QuestRegistry::deserialize(const nlohmann::json& json) {
    QuestRegistry registry;
    if (!json.contains("quests") || !json["quests"].is_array()) {
        return registry;
    }
    for (const auto& quest_json : json["quests"]) {
        QuestDefinition quest;
        quest.id = quest_json.value("id", "");
        if (quest_json.contains("objectives") && quest_json["objectives"].is_array()) {
            for (const auto& objective_json : quest_json["objectives"]) {
                QuestObjective objective;
                objective.id = objective_json.value("id", "");
                objective.state = ObjectiveStateFromString(objective_json.value("state", "locked"));
                objective.updated_at = objective_json.value("updated_at", "");
                if (objective_json.contains("dependencies") && objective_json["dependencies"].is_array()) {
                    for (const auto& dependency_json : objective_json["dependencies"]) {
                        objective.dependencies.push_back(conditionFromJson(dependency_json));
                    }
                }
                quest.objectives.push_back(std::move(objective));
            }
        }
        registry.registerQuest(std::move(quest));
    }
    return registry;
}

} // namespace urpg::quest
