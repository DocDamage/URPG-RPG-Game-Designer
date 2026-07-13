#include "engine/core/dialogue/dialogue_graph.h"

#include <set>
#include <stdexcept>
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

DialogueGraph DialogueGraph::fromJson(const nlohmann::json& json) {
    if (!json.is_object() || json.value("schema_version", "") != "urpg.dialogue_graph.v1" ||
        !json.contains("nodes") || !json.at("nodes").is_array()) {
        throw std::invalid_argument("DialogueGraph JSON has an unsupported schema or missing nodes.");
    }
    DialogueGraph graph;
    for (const auto& source_node : json.at("nodes")) {
        if (!source_node.is_object()) {
            throw std::invalid_argument("DialogueGraph node must be an object.");
        }
        DialogueNode node;
        node.id = source_node.value("id", "");
        node.speaker_id = source_node.value("speaker_id", "");
        node.speaker_name = source_node.value("speaker_name", "");
        node.localization_key = source_node.value("localization_key", "");
        node.text_preview = source_node.value("text_preview", "");
        node.ending = source_node.value("ending", false);
        for (const auto& source_choice : source_node.value("choices", nlohmann::json::array())) {
            DialogueChoice choice;
            choice.id = source_choice.value("id", "");
            choice.label = source_choice.value("label", "");
            choice.target_node_id = source_choice.value("target_node_id", "");
            for (const auto& source_condition : source_choice.value("conditions", nlohmann::json::array())) {
                choice.conditions.push_back({source_condition.value("key", ""), source_condition.value("op", ""),
                                             source_condition.value("value", 0)});
            }
            for (const auto& source_effect : source_choice.value("effects", nlohmann::json::array())) {
                choice.effects.push_back({source_effect.value("key", ""), source_effect.value("delta", 0)});
            }
            node.choices.push_back(std::move(choice));
        }
        if (!graph.addNode(std::move(node))) {
            throw std::invalid_argument("DialogueGraph contains an empty or duplicate node id.");
        }
    }
    graph.setStartNode(json.value("start_node_id", ""));
    if (!graph.startNode().empty() && graph.findNode(graph.startNode()) == nullptr) {
        throw std::invalid_argument("DialogueGraph start node does not exist.");
    }
    return graph;
}

} // namespace urpg::dialogue
