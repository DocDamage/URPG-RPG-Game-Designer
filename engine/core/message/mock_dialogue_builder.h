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
        introNodes.push_back({
            .id = "start",
            .body = "Welcome to the world of URPG! Are you ready for an adventure?",
            .variant = { .speaker = "Elder" },
            .choices = { {"yes", "I am born ready!"}, {"no", "Maybe later..."} },
            .next_node_id = "" // Terminal for this segment, logic handle branch
        });
        introNodes.push_back({
            .id = "accept",
            .body = "That's the spirit! Take this wooden sword.",
            .variant = { .speaker = "Elder" },
            .command = "GIVE_ITEM:wooden_sword",
            .next_node_id = "end"
        });
        introNodes.push_back({
            .id = "reject",
            .body = "The fate of the world waits for no one...",
            .variant = { .speaker = "Elder" },
            .next_node_id = "end"
        });
        introNodes.push_back({
            .id = "end",
            .body = "Safe travels, young one.",
            .variant = { .speaker = "Elder" }
        });

        registry.registerDialogue("intro_elder", introNodes);

        // 2. Healing NPC
        std::vector<DialogueNode> healerNodes;
        healerNodes.push_back({
            .id = "healer_start",
            .body = "You look wounded. Let me heal you.",
            .variant = { .speaker = "Healer" },
            .command = "HEAL_PLAYER:100"
        });
        
        registry.registerDialogue("npc_healer", healerNodes);
    }
};

} // namespace urpg::message
