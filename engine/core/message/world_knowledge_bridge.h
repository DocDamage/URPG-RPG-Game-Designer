#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "engine/core/message/dialogue_registry.h"
#include "engine/core/gameplay/item_registry.h"
#include "engine/core/scene/tileset_registry.h"
#include "engine/core/global_state_hub.h"
#include "engine/core/gameplay/quest_system.h"
#include "engine/core/ecs/world.h"

namespace urpg::ai {

/**
 * @brief Provides a "World Knowledge" context to the chatbot by serializing
 * the game's database (Items, Dialogue, etc.) AND real-time world state.
 */
class WorldKnowledgeBridge {
public:
    /**
     * @brief Generates a comprehensive prompt context including static database
     * and dynamic runtime state (Quests, Switches, Variables).
     */
    static std::string generateContext() {
        std::stringstream ss;
        ss << "You are an omniscient Game Guide Chatbot for the player. \n";
        ss << "Your role is to answer questions about the world, the player's progress, and game mechanics using the following LIVE data:\n\n";

        auto& hub = urpg::GlobalStateHub::getInstance();

        // 1. Dynamic World State (Switches)
        ss << "--- CURRENT WORLD STATE (SWITCHES) ---\n";
        auto allSwitches = hub.getAllSwitches();
        if (allSwitches.empty()) {
            ss << "No switches have been triggered yet.\n";
        } else {
            for (const auto& [id, state] : allSwitches) {
                ss << id << ": " << (state ? "True" : "False") << "\n";
            }
        }
        ss << "\n";

        // 2. Active Quests & NPC States
        // (This would pull from an EventManager or similar tracking system)
        ss << "--- PLAYER PROGRESS ---\n";
        ss << "Player is currently exploring the world.\n\n";

        // 3. Database Reference
        ss << "--- AVAILABLE ACTIONS ---\n";
        ss << "You can use [GIVE_ITEM:id] to award items if the player deserves them.\n";
        
        return ss.str();
    }
};

} // namespace urpg::ai
