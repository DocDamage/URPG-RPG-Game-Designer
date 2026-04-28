#include "engine/core/quest/quest_objective_graph.h"

#include <algorithm>
#include <set>

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

QuestCondition conditionFromJson(const nlohmann::json& json) {
    return {json.value("type", ""), json.value("id", ""), json.value("value", 0)};
}

nlohmann::json conditionToJson(const QuestCondition& condition) {
    return {{"type", condition.type}, {"id", condition.id}, {"value", condition.value}};
}

QuestReward rewardFromJson(const nlohmann::json& json) {
    return {json.value("type", ""), json.value("id", ""), json.value("value", 0)};
}

nlohmann::json rewardToJson(const QuestReward& reward) {
    return {{"type", reward.type}, {"id", reward.id}, {"value", reward.value}};
}

std::string objectiveIdForNode(const QuestGraphNode& node) {
    return node.objective_id.empty() ? node.id : node.objective_id;
}

} // namespace

QuestObjectiveGraphDocument QuestObjectiveGraphDocument::fromJson(const nlohmann::json& json) {
    QuestObjectiveGraphDocument document;
    document.quest_id = json.value("quest_id", "");
    document.title = json.value("title", "");

    if (json.contains("nodes") && json["nodes"].is_array()) {
        for (const auto& node_json : json["nodes"]) {
            QuestGraphNode node;
            node.id = node_json.value("id", "");
            node.type = node_json.value("type", "");
            node.title = node_json.value("title", "");
            node.objective_id = node_json.value("objective_id", "");
            node.localization_key = node_json.value("localization_key", "");
            if (node_json.contains("conditions") && node_json["conditions"].is_array()) {
                for (const auto& condition_json : node_json["conditions"]) {
                    node.conditions.push_back(conditionFromJson(condition_json));
                }
            }
            if (node_json.contains("rewards") && node_json["rewards"].is_array()) {
                for (const auto& reward_json : node_json["rewards"]) {
                    node.rewards.push_back(rewardFromJson(reward_json));
                }
            }
            document.nodes.push_back(std::move(node));
        }
    }

    if (json.contains("links") && json["links"].is_array()) {
        for (const auto& link_json : json["links"]) {
            document.links.push_back({link_json.value("from", ""), link_json.value("to", "")});
        }
    }
    return document;
}

nlohmann::json QuestObjectiveGraphDocument::toJson() const {
    nlohmann::json node_array = nlohmann::json::array();
    for (const auto& node : nodes) {
        nlohmann::json conditions = nlohmann::json::array();
        for (const auto& condition : node.conditions) {
            conditions.push_back(conditionToJson(condition));
        }
        nlohmann::json rewards = nlohmann::json::array();
        for (const auto& reward : node.rewards) {
            rewards.push_back(rewardToJson(reward));
        }
        node_array.push_back({{"id", node.id},
                              {"type", node.type},
                              {"title", node.title},
                              {"objective_id", node.objective_id},
                              {"localization_key", node.localization_key},
                              {"conditions", std::move(conditions)},
                              {"rewards", std::move(rewards)}});
    }

    nlohmann::json link_array = nlohmann::json::array();
    for (const auto& link : links) {
        link_array.push_back({{"from", link.from}, {"to", link.to}});
    }
    return {{"schema_version", "urpg.quest_objective_graph.v1"},
            {"quest_id", quest_id},
            {"title", title},
            {"nodes", std::move(node_array)},
            {"links", std::move(link_array)}};
}

std::vector<QuestGraphDiagnostic> QuestObjectiveGraphDocument::validate() const {
    std::vector<QuestGraphDiagnostic> diagnostics;
    if (quest_id.empty()) {
        diagnostics.push_back({"missing_quest_id", "Quest graph requires a quest_id.", ""});
    }

    std::set<std::string> node_ids;
    bool has_start = false;
    for (const auto& node : nodes) {
        if (node.id.empty()) {
            diagnostics.push_back({"missing_node_id", "Quest graph node requires an id.", ""});
            continue;
        }
        if (!node_ids.insert(node.id).second) {
            diagnostics.push_back({"duplicate_node_id", "Quest graph node id is duplicated.", node.id});
        }
        if (node.type == "start") {
            has_start = true;
        }
        if (node.type == "objective" && objectiveIdForNode(node).empty()) {
            diagnostics.push_back({"missing_objective_id", "Objective node requires an objective id.", node.id});
        }
        for (const auto& condition : node.conditions) {
            if (condition.type.empty() || condition.id.empty()) {
                diagnostics.push_back({"invalid_condition", "Condition requires type and id.", node.id});
            }
        }
        for (const auto& reward : node.rewards) {
            if (reward.type.empty() || reward.id.empty()) {
                diagnostics.push_back({"invalid_reward", "Reward requires type and id.", node.id});
            }
        }
    }

    if (!has_start) {
        diagnostics.push_back({"missing_start_node", "Quest graph requires one start node.", ""});
    }

    for (const auto& link : links) {
        if (!node_ids.contains(link.from)) {
            diagnostics.push_back({"missing_link_source", "Quest graph link source does not exist.", link.from});
        }
        if (!node_ids.contains(link.to)) {
            diagnostics.push_back({"missing_link_target", "Quest graph link target does not exist.", link.to});
        }
    }
    return diagnostics;
}

QuestDefinition QuestObjectiveGraphDocument::toQuestDefinition() const {
    QuestDefinition quest;
    quest.id = quest_id;
    for (const auto& node : nodes) {
        if (node.type != "objective") {
            continue;
        }
        quest.objectives.push_back({objectiveIdForNode(node), ObjectiveState::Locked, node.conditions, ""});
    }
    return quest;
}

QuestGraphPreview QuestObjectiveGraphDocument::preview(const QuestWorldState& world) const {
    QuestGraphPreview preview;
    preview.quest_id = quest_id;
    preview.diagnostics = validate();
    if (!preview.diagnostics.empty()) {
        return preview;
    }

    for (const auto& node : nodes) {
        if (node.type != "objective") {
            continue;
        }
        const bool ready = std::all_of(node.conditions.begin(), node.conditions.end(),
                                       [&](const QuestCondition& condition) { return conditionMet(condition, world); });
        if (ready) {
            preview.ready_node_ids.push_back(node.id);
            preview.completed_objective_ids.push_back(objectiveIdForNode(node));
        } else {
            preview.blocked_node_ids.push_back(node.id);
        }
    }
    return preview;
}

QuestGraphPreview QuestObjectiveGraphDocument::applyReadyObjectives(QuestRegistry& registry, const QuestWorldState& world,
                                                                    const std::string& timestamp) const {
    QuestGraphPreview result = preview(world);
    if (!result.diagnostics.empty()) {
        return result;
    }

    if (registry.findQuest(quest_id) == nullptr) {
        registry.registerQuest(toQuestDefinition());
    }

    result.completed_objective_ids.clear();
    for (const auto& node_id : result.ready_node_ids) {
        const auto it = std::find_if(nodes.begin(), nodes.end(), [&](const QuestGraphNode& node) {
            return node.id == node_id && node.type == "objective";
        });
        if (it != nodes.end() && registry.evaluateObjective(quest_id, objectiveIdForNode(*it), world, timestamp)) {
            result.completed_objective_ids.push_back(objectiveIdForNode(*it));
        }
    }
    return result;
}

nlohmann::json questGraphDiagnosticToJson(const QuestGraphDiagnostic& diagnostic) {
    return {{"code", diagnostic.code}, {"message", diagnostic.message}, {"node_id", diagnostic.node_id}};
}

nlohmann::json questGraphPreviewToJson(const QuestGraphPreview& preview) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        diagnostics.push_back(questGraphDiagnosticToJson(diagnostic));
    }
    return {{"quest_id", preview.quest_id},
            {"ready_node_ids", preview.ready_node_ids},
            {"blocked_node_ids", preview.blocked_node_ids},
            {"completed_objective_ids", preview.completed_objective_ids},
            {"diagnostics", std::move(diagnostics)}};
}

} // namespace urpg::quest
