#pragma once

#include "engine/core/message/dialogue_registry.h"
#include <nlohmann/json.hpp>

namespace urpg::message {

/**
 * @brief Serialization and Deserialization for Dialogue data.
 */
class DialogueSerializer {
public:
    static nlohmann::json toJson(const std::vector<DialogueNode>& nodes) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& node : nodes) {
            nlohmann::json n;
            n["id"] = node.id;
            n["body"] = node.body;
            n["next_node_id"] = node.next_node_id;
            n["command"] = node.command;
            
            // Condition
            if (!node.condition.switch_id.empty() || node.condition.check_variable) {
                n["condition"]["switch_id"] = node.condition.switch_id;
                n["condition"]["expected_value"] = node.condition.expected_value;
                n["condition"]["variable_id"] = node.condition.variable_id;
                n["condition"]["variable_min"] = node.condition.variable_min;
                n["condition"]["variable_max"] = node.condition.variable_max;
                n["condition"]["check_variable"] = node.condition.check_variable;
            }

            // Speaker variant
            n["variant"]["speaker"] = node.variant.speaker;
            n["variant"]["mode"] = static_cast<int>(node.variant.mode);
            n["variant"]["face_actor_id"] = node.variant.face_actor_id;

            // Choices
            n["choices"] = nlohmann::json::array();
            for (const auto& choice : node.choices) {
                n["choices"].push_back({
                    {"id", choice.id},
                    {"label", choice.label},
                    {"enabled", choice.enabled}
                });
            }
            j.push_back(n);
        }
        return j;
    }

    static std::vector<DialogueNode> fromJson(const nlohmann::json& j) {
        std::vector<DialogueNode> nodes;
        if (!j.is_array()) return nodes;

        for (const auto& n : j) {
            DialogueNode node;
            node.id = n.value("id", "");
            node.body = n.value("body", "");
            node.next_node_id = n.value("next_node_id", "");
            node.command = n.value("command", "");

            if (n.contains("condition")) {
                const auto& c = n["condition"];
                node.condition.switch_id = c.value("switch_id", "");
                node.condition.expected_value = c.value("expected_value", true);
                node.condition.variable_id = c.value("variable_id", "");
                node.condition.variable_min = c.value("variable_min", 0);
                node.condition.variable_max = c.value("variable_max", 0);
                node.condition.check_variable = c.value("check_variable", false);
            }

            if (n.contains("variant")) {
                const auto& v = n["variant"];
                node.variant.speaker = v.value("speaker", "");
                node.variant.mode = static_cast<MessagePresentationMode>(v.value("mode", 0));
                node.variant.face_actor_id = v.value("face_actor_id", 0);
            }

            if (n.contains("choices") && n["choices"].is_array()) {
                for (const auto& c : n["choices"]) {
                    node.choices.push_back({
                        .id = c.value("id", ""),
                        .label = c.value("label", ""),
                        .enabled = c.value("enabled", true),
                        .disabled_reason = c.value("disabled_reason", "")
                    });
                }
            }
            nodes.push_back(node);
        }
        return nodes;
    }
};

} // namespace urpg::message
