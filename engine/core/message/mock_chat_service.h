#pragma once

#include "engine/core/message/chatbot_component.h"

namespace urpg::ai {

/**
 * @brief A simple mock AI that responds with scripted variations.
 * Used to test the chatbot pipeline without requiring an internet connection or API keys.
 */
class MockChatService : public IChatService {
public:
    void requestResponse(const std::vector<ChatMessage>& history, ChatCallback callback) override {
        const std::string& lastUserMsg = history.back().content;
        
        std::string response = "I hear you say: '" + lastUserMsg + "'. Truly fascinating.";
        std::string command = "";

        // Simple keyword logic for demonstration
        if (lastUserMsg.find("heal") != std::string::npos) {
            response = "You look tired. Let me mend those wounds.";
            command = "HEAL_PLAYER:100";
        } else if (lastUserMsg.find("sword") != std::string::npos) {
            response = "A warrior's request! Take this blade.";
            command = "GIVE_ITEM:iron_sword";
        }

        // Simulate async delay
        callback(response, command);
    }
};

} // namespace urpg::ai
