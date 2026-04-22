#pragma once

#include "engine/core/message/dialogue_registry.h"
#include <string>

namespace urpg::message {

/**
 * @brief Utility for populating the DialogueRegistry with mock/test data
 * until the formal JSON persistence layer is fully wired.
 */
class MockDialogueBuilder {
public:
    static void populate() {
        auto& registry = DialogueRegistry::getInstance();

        // 1. Simple Introduction Conversation
        std::vector<DialogueNode> introNodes;
        DialogueNode introStart;
        introStart.id = "start";
        introStart.body = "Welcome to the world of URPG! Are you ready for an adventure?";
        introStart.variant.speaker = "Elder";
        introStart.choices = {
            ChoiceOption{.id = "yes", .label = "I am born ready!", .enabled = true, .disabled_reason = ""},
            ChoiceOption{.id = "no", .label = "Maybe later...", .enabled = true, .disabled_reason = ""},
        };
        introStart.next_node_id = ""; // Terminal for this segment, logic handles branch selection.
        introNodes.push_back(introStart);

        DialogueNode introAccept;
        introAccept.id = "accept";
        introAccept.body = "That's the spirit! Take this wooden sword.";
        introAccept.variant.speaker = "Elder";
        introAccept.next_node_id = "end";
        introAccept.command = "GIVE_ITEM:wooden_sword";
        introNodes.push_back(introAccept);

        DialogueNode introReject;
        introReject.id = "reject";
        introReject.body = "The fate of the world waits for no one...";
        introReject.variant.speaker = "Elder";
        introReject.next_node_id = "end";
        introNodes.push_back(introReject);

        DialogueNode introEnd;
        introEnd.id = "end";
        introEnd.body = "Safe travels, young one.";
        introEnd.variant.speaker = "Elder";
        introNodes.push_back(introEnd);

        registry.registerConversation("intro_elder", introNodes);

        // 2. Healing NPC
        std::vector<DialogueNode> healerNodes;
        DialogueNode healerStart;
        healerStart.id = "healer_start";
        healerStart.body = "You look wounded. Let me heal you.";
        healerStart.variant.speaker = "Healer";
        healerStart.command = "HEAL_PLAYER:100";
        healerNodes.push_back(healerStart);
        
        registry.registerConversation("npc_healer", healerNodes);
    }
};

} // namespace urpg::message
