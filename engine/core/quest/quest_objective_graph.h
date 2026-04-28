#pragma once

#include "engine/core/quest/quest_registry.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::quest {

struct QuestReward {
    std::string type;
    std::string id;
    int value = 0;
};

struct QuestGraphNode {
    std::string id;
    std::string type;
    std::string title;
    std::string objective_id;
    std::string localization_key;
    std::vector<QuestCondition> conditions;
    std::vector<QuestReward> rewards;
};

struct QuestGraphLink {
    std::string from;
    std::string to;
};

struct QuestGraphDiagnostic {
    std::string code;
    std::string message;
    std::string node_id;
};

struct QuestGraphPreview {
    std::string quest_id;
    std::vector<std::string> ready_node_ids;
    std::vector<std::string> blocked_node_ids;
    std::vector<std::string> completed_objective_ids;
    std::vector<QuestGraphDiagnostic> diagnostics;
};

class QuestObjectiveGraphDocument {
public:
    std::string quest_id;
    std::string title;
    std::vector<QuestGraphNode> nodes;
    std::vector<QuestGraphLink> links;

    static QuestObjectiveGraphDocument fromJson(const nlohmann::json& json);
    nlohmann::json toJson() const;

    std::vector<QuestGraphDiagnostic> validate() const;
    QuestDefinition toQuestDefinition() const;
    QuestGraphPreview preview(const QuestWorldState& world) const;
    QuestGraphPreview applyReadyObjectives(QuestRegistry& registry, const QuestWorldState& world,
                                           const std::string& timestamp) const;
};

nlohmann::json questGraphDiagnosticToJson(const QuestGraphDiagnostic& diagnostic);
nlohmann::json questGraphPreviewToJson(const QuestGraphPreview& preview);

} // namespace urpg::quest
