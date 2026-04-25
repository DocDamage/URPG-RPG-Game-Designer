#pragma once

#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <vector>

namespace urpg::quest {

enum class ObjectiveState {
    Locked,
    Active,
    Completed,
    Failed,
    Hidden,
};

struct QuestCondition {
    std::string type;
    std::string id;
    int value = 0;
};

struct QuestObjective {
    std::string id;
    ObjectiveState state = ObjectiveState::Locked;
    std::vector<QuestCondition> dependencies;
    std::string updated_at;
};

struct QuestDefinition {
    std::string id;
    std::vector<QuestObjective> objectives;
};

struct QuestWorldState {
    std::map<std::string, bool> switches;
    std::map<std::string, int> variables;
    std::vector<std::string> items;
    std::vector<std::string> battles;
    std::vector<std::string> dialogue_choices;
    std::map<std::string, int> reputation;
};

class QuestRegistry {
public:
    bool registerQuest(QuestDefinition definition);
    bool evaluateObjective(const std::string& quest_id, const std::string& objective_id,
                           const QuestWorldState& world, const std::string& timestamp);
    const QuestDefinition* findQuest(const std::string& quest_id) const;
    nlohmann::json serialize() const;
    static QuestRegistry deserialize(const nlohmann::json& json);

private:
    std::map<std::string, QuestDefinition> quests_;
};

std::string ToString(ObjectiveState state);
ObjectiveState ObjectiveStateFromString(const std::string& value);

} // namespace urpg::quest
