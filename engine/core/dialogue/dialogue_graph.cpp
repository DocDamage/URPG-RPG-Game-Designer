#include "engine/core/dialogue/dialogue_graph.h"

#include <algorithm>
#include <set>
#include <utility>

namespace urpg::dialogue {
namespace {

std::optional<bool> evaluatePreviewCondition(const DialogueCondition& condition,
                                             const std::map<std::string, int>& values) {
    const auto found = values.find(condition.key);
    const int actual = found == values.end() ? 0 : found->second;
    if (condition.op == "=" || condition.op == "==") return actual == condition.value;
    if (condition.op == "!=") return actual != condition.value;
    if (condition.op == ">") return actual > condition.value;
    if (condition.op == ">=") return actual >= condition.value;
    if (condition.op == "<") return actual < condition.value;
    if (condition.op == "<=") return actual <= condition.value;
    return std::nullopt;
}

const DialogueChoice* findChoice(const DialogueNode& node, const std::string& choice_id) {
    const auto choice = std::find_if(node.choices.begin(), node.choices.end(),
                                     [&](const DialogueChoice& candidate) { return candidate.id == choice_id; });
    return choice == node.choices.end() ? nullptr : &(*choice);
}

} // namespace

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

bool DialogueGraph::removeNode(const std::string& node_id) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end() || node_id == start_node_id_) {
        return false;
    }
    nodes_.erase(node);
    for (auto& [_, candidate] : nodes_) {
        candidate.choices.erase(
            std::remove_if(candidate.choices.begin(), candidate.choices.end(), [&](const DialogueChoice& choice) {
                return choice.target_node_id == node_id;
            }),
            candidate.choices.end());
    }
    return true;
}

bool DialogueGraph::updateNode(const std::string& node_id, std::string speaker_id, std::string speaker_name,
                               std::string localization_key, std::string text_preview, const bool ending) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end() || speaker_id.empty() || localization_key.empty()) {
        return false;
    }
    auto& current = node->second;
    if (current.speaker_id == speaker_id && current.speaker_name == speaker_name &&
        current.localization_key == localization_key && current.text_preview == text_preview && current.ending == ending) {
        return false;
    }
    current.speaker_id = std::move(speaker_id);
    current.speaker_name = std::move(speaker_name);
    current.localization_key = std::move(localization_key);
    current.text_preview = std::move(text_preview);
    current.ending = ending;
    return true;
}

bool DialogueGraph::updateNodeMediaReferences(const std::string& node_id, std::string voice_asset_id,
                                              std::string caption_localization_key) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end()) {
        return false;
    }
    auto& current = node->second;
    if (current.voice_asset_id == voice_asset_id && current.caption_localization_key == caption_localization_key) {
        return false;
    }
    current.voice_asset_id = std::move(voice_asset_id);
    current.caption_localization_key = std::move(caption_localization_key);
    return true;
}

bool DialogueGraph::updateNodeCanvasPosition(const std::string& node_id, const int32_t canvas_x,
                                             const int32_t canvas_y) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end()) {
        return false;
    }
    auto& current = node->second;
    if (current.has_canvas_position && current.canvas_x == canvas_x && current.canvas_y == canvas_y) {
        return false;
    }
    current.canvas_x = canvas_x;
    current.canvas_y = canvas_y;
    current.has_canvas_position = true;
    return true;
}

bool DialogueGraph::addChoice(const std::string& node_id, DialogueChoice choice) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end() || choice.id.empty() || choice.target_node_id.empty() ||
        !nodes_.contains(choice.target_node_id)) {
        return false;
    }
    const auto duplicate = std::any_of(node->second.choices.begin(), node->second.choices.end(),
                                       [&](const DialogueChoice& candidate) { return candidate.id == choice.id; });
    if (duplicate) {
        return false;
    }
    node->second.choices.push_back(std::move(choice));
    return true;
}

bool DialogueGraph::removeChoice(const std::string& node_id, const std::string& choice_id) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end() || choice_id.empty()) {
        return false;
    }
    const auto choice = std::find_if(node->second.choices.begin(), node->second.choices.end(),
                                     [&](const DialogueChoice& candidate) { return candidate.id == choice_id; });
    if (choice == node->second.choices.end()) {
        return false;
    }
    node->second.choices.erase(choice);
    return true;
}

bool DialogueGraph::updateChoice(const std::string& node_id, const std::string& choice_id, std::string label,
                                 std::string target_node_id, std::string localization_key) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end() || choice_id.empty() || label.empty() || target_node_id.empty() ||
        !nodes_.contains(target_node_id)) {
        return false;
    }
    const auto choice = std::find_if(node->second.choices.begin(), node->second.choices.end(),
                                     [&](const DialogueChoice& candidate) { return candidate.id == choice_id; });
    if (choice == node->second.choices.end() ||
        (choice->label == label && choice->target_node_id == target_node_id &&
         choice->localization_key == localization_key)) {
        return false;
    }
    choice->label = std::move(label);
    choice->target_node_id = std::move(target_node_id);
    choice->localization_key = std::move(localization_key);
    return true;
}

bool DialogueGraph::addChoiceCondition(const std::string& node_id, const std::string& choice_id,
                                       DialogueCondition condition) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end() || choice_id.empty() || condition.key.empty() || condition.op.empty()) {
        return false;
    }
    const auto choice = std::find_if(node->second.choices.begin(), node->second.choices.end(),
                                     [&](const DialogueChoice& candidate) { return candidate.id == choice_id; });
    if (choice == node->second.choices.end()) {
        return false;
    }
    const auto duplicate = std::any_of(choice->conditions.begin(), choice->conditions.end(),
                                       [&](const DialogueCondition& candidate) {
                                           return candidate.key == condition.key && candidate.op == condition.op &&
                                                  candidate.value == condition.value;
                                       });
    if (duplicate) {
        return false;
    }
    choice->conditions.push_back(std::move(condition));
    return true;
}

bool DialogueGraph::removeChoiceCondition(const std::string& node_id, const std::string& choice_id,
                                          const DialogueCondition& condition) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end() || choice_id.empty()) {
        return false;
    }
    const auto choice = std::find_if(node->second.choices.begin(), node->second.choices.end(),
                                     [&](const DialogueChoice& candidate) { return candidate.id == choice_id; });
    if (choice == node->second.choices.end()) {
        return false;
    }
    const auto condition_it = std::find_if(choice->conditions.begin(), choice->conditions.end(),
                                           [&](const DialogueCondition& candidate) {
                                               return candidate.key == condition.key && candidate.op == condition.op &&
                                                      candidate.value == condition.value;
                                           });
    if (condition_it == choice->conditions.end()) {
        return false;
    }
    choice->conditions.erase(condition_it);
    return true;
}

bool DialogueGraph::addChoiceEffect(const std::string& node_id, const std::string& choice_id, DialogueEffect effect) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end() || choice_id.empty() || effect.key.empty()) {
        return false;
    }
    const auto choice = std::find_if(node->second.choices.begin(), node->second.choices.end(),
                                     [&](const DialogueChoice& candidate) { return candidate.id == choice_id; });
    if (choice == node->second.choices.end()) {
        return false;
    }
    const auto duplicate = std::any_of(choice->effects.begin(), choice->effects.end(), [&](const DialogueEffect& candidate) {
        return candidate.key == effect.key && candidate.delta == effect.delta;
    });
    if (duplicate) {
        return false;
    }
    choice->effects.push_back(std::move(effect));
    return true;
}

bool DialogueGraph::removeChoiceEffect(const std::string& node_id, const std::string& choice_id,
                                       const DialogueEffect& effect) {
    const auto node = nodes_.find(node_id);
    if (node == nodes_.end() || choice_id.empty()) {
        return false;
    }
    const auto choice = std::find_if(node->second.choices.begin(), node->second.choices.end(),
                                     [&](const DialogueChoice& candidate) { return candidate.id == choice_id; });
    if (choice == node->second.choices.end()) {
        return false;
    }
    const auto effect_it = std::find_if(choice->effects.begin(), choice->effects.end(), [&](const DialogueEffect& candidate) {
        return candidate.key == effect.key && candidate.delta == effect.delta;
    });
    if (effect_it == choice->effects.end()) {
        return false;
    }
    choice->effects.erase(effect_it);
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

std::vector<DialoguePreviewChoiceState> DialogueGraph::previewChoices(
    const std::string& node_id, const std::map<std::string, int>& values) const {
    std::vector<DialoguePreviewChoiceState> result;
    const auto* node = findNode(node_id);
    if (node == nullptr) {
        return result;
    }
    for (const auto& choice : node->choices) {
        DialoguePreviewChoiceState state;
        state.id = choice.id;
        state.label = choice.label;
        state.localization_key = choice.localization_key;
        state.target_node_id = choice.target_node_id;
        state.enabled = true;
        if (choice.target_node_id.empty() || findNode(choice.target_node_id) == nullptr) {
            state.enabled = false;
            state.diagnostics.push_back({"preview_choice_target_missing",
                                         "Dialogue preview choice target does not exist.", node_id, choice.id});
        }
        for (const auto& condition : choice.conditions) {
            const auto matches = evaluatePreviewCondition(condition, values);
            if (!matches.has_value()) {
                state.enabled = false;
                state.diagnostics.push_back({"preview_condition_operator_unsupported",
                                             "Dialogue preview does not support this condition operator.",
                                             node_id, choice.id});
            } else if (!*matches) {
                state.enabled = false;
                state.diagnostics.push_back({"preview_choice_condition_unmet",
                                             "Dialogue choice conditions are not met by the current preview values.",
                                             node_id, choice.id});
            }
        }
        result.push_back(std::move(state));
    }
    return result;
}

DialoguePreviewTransition DialogueGraph::previewChoice(const std::string& node_id, const std::string& choice_id,
                                                       const std::map<std::string, int>& values) const {
    DialoguePreviewTransition result;
    result.values = values;
    const auto* node = findNode(node_id);
    if (node == nullptr) {
        result.diagnostics.push_back({"preview_node_missing", "Dialogue preview node does not exist.", node_id, choice_id});
        return result;
    }
    const auto* choice = findChoice(*node, choice_id);
    if (choice == nullptr) {
        result.diagnostics.push_back({"preview_choice_missing", "Dialogue preview choice does not exist.", node_id, choice_id});
        return result;
    }
    const auto states = previewChoices(node_id, values);
    const auto state = std::find_if(states.begin(), states.end(), [&](const DialoguePreviewChoiceState& candidate) {
        return candidate.id == choice_id;
    });
    if (state == states.end() || !state->enabled) {
        if (state != states.end()) {
            result.diagnostics = state->diagnostics;
        }
        return result;
    }
    for (const auto& effect : choice->effects) {
        result.values[effect.key] += effect.delta;
    }
    result.next_node_id = choice->target_node_id;
    result.applied = true;
    return result;
}

std::vector<DialogueGraphDiagnostic> DialogueGraph::validate() const {
    std::vector<DialogueGraphDiagnostic> diagnostics;
    if (nodes_.empty()) {
        diagnostics.push_back({"missing_nodes", "Dialogue graph requires at least one node.", "", ""});
        return diagnostics;
    }
    if (start_node_id_.empty() || !nodes_.contains(start_node_id_)) {
        diagnostics.push_back({"missing_start_node", "Dialogue graph requires an existing start node.", "", ""});
    }
    for (const auto& [node_id, node] : nodes_) {
        std::set<std::string> choice_ids;
        for (const auto& choice : node.choices) {
            if (choice.id.empty()) {
                diagnostics.push_back(
                    {"missing_choice_id", "Dialogue choice requires an id.", node_id, ""});
            } else if (!choice_ids.insert(choice.id).second) {
                diagnostics.push_back(
                    {"duplicate_choice_id", "Dialogue choice id is duplicated within its node.", node_id, choice.id});
            }
            if (choice.target_node_id.empty() || !nodes_.contains(choice.target_node_id)) {
                diagnostics.push_back({"missing_choice_target", "Dialogue choice target does not exist.", node_id,
                                       choice.id});
            }
        }
    }
    return diagnostics;
}

std::vector<DialogueGraphDiagnostic> DialogueGraph::analyzeFlow() const {
    if (!validate().empty()) {
        return {};
    }

    std::vector<std::string> endings;
    for (const auto& [node_id, node] : nodes_) {
        if (node.ending) {
            endings.push_back(node_id);
        }
    }

    std::set<std::string> reachable;
    std::vector<std::string> pending = {start_node_id_};
    for (size_t index = 0; index < pending.size(); ++index) {
        const auto& current = pending[index];
        if (!reachable.insert(current).second) {
            continue;
        }
        const auto& node = nodes_.at(current);
        for (const auto& choice : node.choices) {
            pending.push_back(choice.target_node_id);
        }
    }

    std::set<std::string> can_end;
    pending = endings;
    for (size_t index = 0; index < pending.size(); ++index) {
        const auto& current = pending[index];
        if (!can_end.insert(current).second) {
            continue;
        }
        for (const auto& [node_id, node] : nodes_) {
            if (std::any_of(node.choices.begin(), node.choices.end(), [&](const DialogueChoice& choice) {
                    return choice.target_node_id == current;
                })) {
                pending.push_back(node_id);
            }
        }
    }

    std::vector<DialogueGraphDiagnostic> diagnostics;
    if (endings.empty()) {
        diagnostics.push_back({"missing_ending_node", "Dialogue graph has no ending node.", "", ""});
    }
    for (const auto& [node_id, node] : nodes_) {
        if (!reachable.contains(node_id)) {
            diagnostics.push_back({"unreachable_node", "Dialogue node is unreachable from the start node.", node_id, ""});
        } else if (!can_end.contains(node_id)) {
            diagnostics.push_back(
                {"no_ending_path", "Dialogue node cannot reach an ending node.", node_id, ""});
        }
        if (!node.ending && node.choices.empty()) {
            diagnostics.push_back({"dead_end_node", "Non-ending dialogue node has no choices.", node_id, ""});
        }
    }
    return diagnostics;
}

std::vector<DialogueGraphDiagnostic> DialogueGraph::validateLocalizationKeys(
    const std::set<std::string>& localization_keys) const {
    std::vector<DialogueGraphDiagnostic> diagnostics;
    if (localization_keys.empty()) {
        return diagnostics;
    }
    for (const auto& [node_id, node] : nodes_) {
        if (!node.localization_key.empty() && !localization_keys.contains(node.localization_key)) {
            diagnostics.push_back({"missing_localization_key",
                                   "Dialogue node localization key is not present in the active project catalog.",
                                   node_id, ""});
        }
        if (!node.caption_localization_key.empty() && !localization_keys.contains(node.caption_localization_key)) {
            diagnostics.push_back({"missing_caption_localization_key",
                                   "Dialogue node caption localization key is not present in the active project catalog.",
                                   node_id, ""});
        }
        for (const auto& choice : node.choices) {
            if (!choice.localization_key.empty() && !localization_keys.contains(choice.localization_key)) {
                diagnostics.push_back({"missing_choice_localization_key",
                                       "Dialogue choice localization key is not present in the active project catalog.",
                                       node_id, choice.id});
            }
        }
    }
    return diagnostics;
}

std::vector<DialogueGraphDiagnostic> DialogueGraph::validateVoiceAssetIds(
    const std::set<std::string>& voice_asset_ids) const {
    std::vector<DialogueGraphDiagnostic> diagnostics;
    for (const auto& [node_id, node] : nodes_) {
        if (!node.voice_asset_id.empty() && !voice_asset_ids.contains(node.voice_asset_id)) {
            diagnostics.push_back({"missing_voice_asset",
                                   "Dialogue node voice asset is not an attached audio asset in the active project.",
                                   node_id, ""});
        }
    }
    return diagnostics;
}

std::optional<DialogueGraph> DialogueGraph::fromJson(const nlohmann::json& json) {
    if (!json.is_object() || !json.contains("schema_version") || !json["schema_version"].is_string() ||
        json["schema_version"] != "urpg.dialogue_graph.v1" || !json.contains("start_node_id") ||
        !json["start_node_id"].is_string() || !json.contains("nodes") || !json["nodes"].is_array()) {
        return std::nullopt;
    }

    try {
        DialogueGraph graph;
        for (const auto& node_json : json["nodes"]) {
            if (!node_json.is_object() || !node_json.contains("id") || !node_json["id"].is_string() ||
                !node_json.contains("speaker_id") || !node_json["speaker_id"].is_string() ||
                !node_json.contains("speaker_name") || !node_json["speaker_name"].is_string() ||
                !node_json.contains("localization_key") || !node_json["localization_key"].is_string() ||
                !node_json.contains("text_preview") || !node_json["text_preview"].is_string() ||
                !node_json.contains("ending") || !node_json["ending"].is_boolean() || !node_json.contains("choices") ||
                !node_json["choices"].is_array()) {
                return std::nullopt;
            }
            if ((node_json.contains("voice_asset_id") && !node_json["voice_asset_id"].is_string()) ||
                (node_json.contains("caption_localization_key") &&
                 !node_json["caption_localization_key"].is_string()) ||
                (node_json.contains("canvas_x") && !node_json["canvas_x"].is_number_integer()) ||
                (node_json.contains("canvas_y") && !node_json["canvas_y"].is_number_integer()) ||
                (node_json.contains("has_canvas_position") && !node_json["has_canvas_position"].is_boolean())) {
                return std::nullopt;
            }
            DialogueNode node{node_json["id"].get<std::string>(), node_json["speaker_id"].get<std::string>(),
                              node_json["speaker_name"].get<std::string>(),
                              node_json["localization_key"].get<std::string>(),
                              node_json["text_preview"].get<std::string>(), node_json["ending"].get<bool>(), {},
                              node_json.value("voice_asset_id", ""),
                              node_json.value("caption_localization_key", ""), node_json.value("canvas_x", 0),
                              node_json.value("canvas_y", 0), node_json.value("has_canvas_position", false)};
            for (const auto& choice_json : node_json["choices"]) {
                if (!choice_json.is_object() || !choice_json.contains("id") || !choice_json["id"].is_string() ||
                    !choice_json.contains("label") || !choice_json["label"].is_string() ||
                    !choice_json.contains("target_node_id") || !choice_json["target_node_id"].is_string() ||
                    !choice_json.contains("conditions") || !choice_json["conditions"].is_array() ||
                    !choice_json.contains("effects") || !choice_json["effects"].is_array()) {
                    return std::nullopt;
                }
                if (choice_json.contains("localization_key") && !choice_json["localization_key"].is_string()) {
                    return std::nullopt;
                }
                DialogueChoice choice{choice_json["id"].get<std::string>(), choice_json["label"].get<std::string>(),
                                      choice_json["target_node_id"].get<std::string>(), {}, {}};
                choice.localization_key = choice_json.value("localization_key", "");
                for (const auto& condition_json : choice_json["conditions"]) {
                    if (!condition_json.is_object() || !condition_json.contains("key") ||
                        !condition_json["key"].is_string() || !condition_json.contains("op") ||
                        !condition_json["op"].is_string() || !condition_json.contains("value") ||
                        !condition_json["value"].is_number_integer()) {
                        return std::nullopt;
                    }
                    choice.conditions.push_back({condition_json["key"].get<std::string>(),
                                                 condition_json["op"].get<std::string>(),
                                                 condition_json["value"].get<int>()});
                }
                for (const auto& effect_json : choice_json["effects"]) {
                    if (!effect_json.is_object() || !effect_json.contains("key") || !effect_json["key"].is_string() ||
                        !effect_json.contains("delta") || !effect_json["delta"].is_number_integer()) {
                        return std::nullopt;
                    }
                    choice.effects.push_back(
                        {effect_json["key"].get<std::string>(), effect_json["delta"].get<int>()});
                }
                node.choices.push_back(std::move(choice));
            }
            if (!graph.addNode(std::move(node))) {
                return std::nullopt;
            }
        }
        graph.setStartNode(json["start_node_id"].get<std::string>());
        return graph;
    } catch (const nlohmann::json::exception&) {
        return std::nullopt;
    }
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
            if (!choice.localization_key.empty()) {
                choice_json["localization_key"] = choice.localization_key;
            }
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
        if (!node.voice_asset_id.empty()) {
            node_json["voice_asset_id"] = node.voice_asset_id;
        }
        if (!node.caption_localization_key.empty()) {
            node_json["caption_localization_key"] = node.caption_localization_key;
        }
        if (node.has_canvas_position) {
            node_json["canvas_x"] = node.canvas_x;
            node_json["canvas_y"] = node.canvas_y;
            node_json["has_canvas_position"] = true;
        }
        serialized_nodes.push_back(node_json);
    }
    nlohmann::json result;
    result["schema_version"] = "urpg.dialogue_graph.v1";
    result["start_node_id"] = start_node_id_;
    result["nodes"] = serialized_nodes;
    return result;
}

} // namespace urpg::dialogue
