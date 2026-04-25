#include "engine/core/dialogue/dialogue_graph.h"

#include <set>
#include <utility>

namespace urpg::dialogue {

bool DialogueGraph::addNode(DialogueNode node) {
    if (node.id.empty() || nodes_.contains(node.id)) {
        return false;
    }
    const auto id = node.id;
    nodes_[id] = std::move(node);
    if (start_node_id_.empty()) {
        start_node_id_ = id;
    }
    return true;
}

void DialogueGraph::setStartNode(std::string node_id) {
    start_node_id_ = std::move(node_id);
}

const DialogueNode* DialogueGraph::findNode(const std::string& node_id) const {
    const auto it = nodes_.find(node_id);
    return it == nodes_.end() ? nullptr : &it->second;
}

const std::map<std::string, DialogueNode>& DialogueGraph::nodes() const {
    return nodes_;
}

const std::string& DialogueGraph::startNode() const {
    return start_node_id_;
}

std::vector<std::string> DialogueGraph::previewRoute(std::size_t max_steps) const {
    std::vector<std::string> route;
    std::set<std::string> visited;
    std::string current = start_node_id_;
    while (!current.empty() && route.size() < max_steps && !visited.contains(current)) {
        const auto* node = findNode(current);
        if (!node) {
            break;
        }
        visited.insert(current);
        route.push_back(current);
        if (node->ending || node->choices.empty()) {
            break;
        }
        current = node->choices.front().target_node_id;
    }
    return route;
}

nlohmann::json DialogueGraph::serialize() const {
    nlohmann::json serialized_nodes = nlohmann::json::array();
    for (const auto& [id, node] : nodes_) {
        nlohmann::json choices = nlohmann::json::array();
        for (const auto& choice : node.choices) {
            nlohmann::json conditions = nlohmann::json::array();
            for (const auto& condition : choice.conditions) {
                nlohmann::json condition_json;
                condition_json["key"] = condition.key;
                condition_json["op"] = condition.op;
                condition_json["value"] = condition.value;
                conditions.push_back(condition_json);
            }
            nlohmann::json effects = nlohmann::json::array();
            for (const auto& effect : choice.effects) {
                nlohmann::json effect_json;
                effect_json["key"] = effect.key;
                effect_json["delta"] = effect.delta;
                effects.push_back(effect_json);
            }
            nlohmann::json choice_json;
            choice_json["id"] = choice.id;
            choice_json["label"] = choice.label;
            choice_json["target_node_id"] = choice.target_node_id;
            choice_json["conditions"] = conditions;
            choice_json["effects"] = effects;
            choices.push_back(choice_json);
        }
        nlohmann::json node_json;
        node_json["id"] = id;
        node_json["speaker_id"] = node.speaker_id;
        node_json["speaker_name"] = node.speaker_name;
        node_json["localization_key"] = node.localization_key;
        node_json["text_preview"] = node.text_preview;
        node_json["ending"] = node.ending;
        node_json["choices"] = choices;
        serialized_nodes.push_back(node_json);
    }
    nlohmann::json result;
    result["schema_version"] = "urpg.dialogue_graph.v1";
    result["start_node_id"] = start_node_id_;
    result["nodes"] = serialized_nodes;
    return result;
}

} // namespace urpg::dialogue
