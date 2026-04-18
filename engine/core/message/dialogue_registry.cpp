#include "engine/core/message/dialogue_registry.h"
#include "engine/core/message/dialogue_serializer.h"
#include <nlohmann/json.hpp>

namespace urpg::message {

void DialogueRegistry::loadFromResource(const std::string& conversation_id, const nlohmann::json& json) {
    auto nodes = DialogueSerializer::fromJson(json);
    if (!nodes.empty()) {
        registerConversation(conversation_id, nodes);
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

} // namespace urpg::message
