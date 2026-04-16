#include "engine/core/message/dialogue_registry.h"
#include "engine/core/message/dialogue_serializer.h"
#include <nlohmann/json.hpp>

namespace urpg::message {

void DialogueRegistry::loadFromResource(const std::string& conversation_id, const nlohmann::json& json) {
    auto nodes = DialogueSerializer::fromJson(json);
    if (!nodes.empty()) {
        registerDialogue(conversation_id, nodes);
    }
}

bool DialogueRegistry::evaluateCondition(const DialogueCondition& cond) const {
    auto& hub = GlobalStateHub::getInstance();

    // 1. Check Switch if ID is provided
    if (!cond.switch_id.empty()) {
        if (hub.getSwitch(cond.switch_id) != cond.expected_value) {
            return false;
        }
    }

    // 2. Check Variable if flagged
    if (cond.check_variable && !cond.variable_id.empty()) {
        auto val = hub.getVariable(cond.variable_id);
        // We expect an int32 for range comparisons
        if (std::holds_alternative<int32_t>(val)) {
            int32_t varVal = std::get<int32_t>(val);
            if (varVal < cond.variable_min || varVal > cond.variable_max) {
                return false;
            }
        } else {
            return false; // Type mismatch
        }
    }

    return true;
}

std::vector<DialoguePage> DialogueRegistry::flattenConversation(const std::string& conversation_id, const std::string& start_node_id) const {
    std::vector<DialoguePage> pages;
    const std::vector<DialogueNode>* conv = getConversation(conversation_id);
    if (!conv || conv->empty()) return pages;

    std::string current_id = start_node_id.empty() ? (*conv)[0].id : start_node_id;
    
    while (!current_id.empty()) {
        const DialogueNode* node = getNode(conversation_id, current_id);
        if (!node) break;

        // Skip nodes with failed conditions but allow the chain to continue if possible?
        // Actually, in most narrative systems, if a node's condition fails, 
        // that node is skipped or the branch ends.
        if (evaluateCondition(node->condition)) {
            DialoguePage page;
            page.id = node->id;
            page.body = node->body;
            page.variant = node->variant;
            
            // Map the simple choice ID to the actual target node ID if specified in choices
            // In a real data structure, ChoiceOption might need a target_id field.
            // For now, we allow the MessageFlowRunner to return the choice ID, 
            // and the caller (MapScene) can restart the flow from that node.
            page.choices = node->choices;
            page.command = node->command;
            pages.push_back(page);

            // If we have choices, this node is a branch point. 
            // The flattening stops here because the next node depends on user input.
            if (!page.choices.empty()) {
                break;
            }
        }

        current_id = node->next_node_id;
        if (pages.size() > 100) break; 
    }

    return pages;
}

} // namespace urpg::message
