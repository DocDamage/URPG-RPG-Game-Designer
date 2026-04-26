#pragma once

#include "dialogue_registry.h"
#include "dialogue_serializer.h"
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::message {

/**
 * @brief Represents a single dialogue file (.json) containing multiple conversations.
 */
struct DialogueDatabaseFile {
    std::string name;
    std::map<std::string, std::vector<DialogueNode>> conversations;
};

/**
 * @brief Handles persistence and bulk loading of dialogue datasets.
 */
class DialogueDatabase {
  public:
    /**
     * @brief Exports all registered conversations to a unified project-wide JSON.
     */
    static nlohmann::json exportProjectDatabase(const DialogueRegistry& registry) {
        nlohmann::json root = nlohmann::json::object();
        auto conversations = registry.getConversations();

        for (const auto& [convId, nodes] : conversations) {
            root[convId] = DialogueSerializer::toJson(nodes);
        }

        return root;
    }

    /**
     * @brief Imports a project-wide JSON into the current registry.
     */
    static void importProjectDatabase(DialogueRegistry& registry, const nlohmann::json& json) {
        if (!json.is_object())
            return;

        for (auto it = json.begin(); it != json.end(); ++it) {
            std::string convId = it.key();
            auto nodes = DialogueSerializer::fromJson(it.value());
            registry.registerConversation(convId, nodes);
        }
    }

    /**
     * @brief Creates a minimal metadata-only export for faster lookups.
     */
    static nlohmann::json exportMetadata(const DialogueRegistry& registry) {
        nlohmann::json root = nlohmann::json::array();
        auto conversations = registry.getConversations();

        for (const auto& [convId, nodes] : conversations) {
            nlohmann::json meta;
            meta["id"] = convId;
            meta["node_count"] = nodes.size();
            if (!nodes.empty()) {
                meta["first_line"] = nodes[0].body.substr(0, 32) + "...";
            }
            root.push_back(meta);
        }

        return root;
    }
};

} // namespace urpg::message
