#pragma once

#include "engine/core/quest/quest_registry.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <set>
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
    int32_t canvas_x = 0;
    int32_t canvas_y = 0;
    bool has_canvas_position = false;
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

    bool addNode(QuestGraphNode node);
    bool removeNode(const std::string& node_id);
    bool reorderNode(const std::string& node_id, std::size_t new_index);
    bool connect(std::string from, std::string to);
    bool disconnect(const std::string& from, const std::string& to);
    bool updateNodeCanvasPosition(const std::string& node_id, int32_t canvas_x, int32_t canvas_y);

    std::vector<QuestGraphDiagnostic> validate() const;
    std::vector<QuestGraphDiagnostic> validateLocalizationKeys(const std::set<std::string>& localization_keys) const;
    // Authoring-only topology diagnostics. This preserves the compatibility
    // validator above while exposing unreachable nodes and completion softlocks
    // to native quest editors before a graph is saved or played.
    std::vector<QuestGraphDiagnostic> analyzeFlow() const;
    QuestDefinition toQuestDefinition() const;
    QuestGraphPreview preview(const QuestWorldState& world) const;
    QuestGraphPreview applyReadyObjectives(QuestRegistry& registry, const QuestWorldState& world,
                                           const std::string& timestamp) const;
};

nlohmann::json questGraphDiagnosticToJson(const QuestGraphDiagnostic& diagnostic);
nlohmann::json questGraphPreviewToJson(const QuestGraphPreview& preview);

} // namespace urpg::quest
