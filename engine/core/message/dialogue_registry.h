#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <nlohmann/json_fwd.hpp>
#include "engine/core/message/message_core.h"
#include "engine/core/global_state_hub.h"

namespace urpg::message {

/**
 * @brief Represents a condition for dialogue transitions.
 */
struct DialogueCondition {
    std::string switch_id;
    bool expected_value = true;
    std::string variable_id;
    int32_t variable_min = 0;
    int32_t variable_max = 0;
    bool check_variable = false;
};

/**
 * @brief Represents a single node in a dialogue database entry.
 */
struct DialogueNode {
    std::string id;
    std::string body;
    MessagePresentationVariant variant;
    std::vector<ChoiceOption> choices;
    std::string next_node_id;
    std::string command; // "Tool usage" hook (e.g., "GIVE_ITEM:potion")
    DialogueCondition condition; // Logic hook for conditional entry/branching
};

/**
 * @brief Registry for loading and retrieving dialogue trees from data.
 */
class DialogueRegistry {
public:
    static DialogueRegistry& getInstance() {
        static DialogueRegistry instance;
        return instance;
    }

    void registerConversation(const std::string& conversation_id, const std::vector<DialogueNode>& nodes) {
        m_conversations[conversation_id] = nodes;
    }

    const std::map<std::string, std::vector<DialogueNode>>& getConversations() const {
        return m_conversations;
    }

    const std::vector<DialogueNode>* getConversation(const std::string& id) const {
        auto it = m_conversations.find(id);
        return (it != m_conversations.end()) ? &it->second : nullptr;
    }

    const DialogueNode* getNode(const std::string& conversation_id, const std::string& node_id) const {
        if (auto conversation = getConversation(conversation_id)) {
            for (const auto& node : *conversation) {
                if (node.id == node_id) return &node;
            }
        }
        return nullptr;
    }

    /**
     * @brief Loads a conversation from a JSON object.
     */
    void loadFromResource(const std::string& conversation_id, const nlohmann::json& json);

    /**
     * @brief Evaluates a dialogue condition against the GlobalStateHub.
     */
    bool evaluateCondition(const DialogueCondition& condition) const;

    /**
     * @brief Converts a sequence of conversation nodes starting from a node_id
     * into a linear set of DialoguePage objects for the MessageFlowRunner.
     */
    std::vector<DialoguePage> flattenConversation(const std::string& conversation_id, const std::string& start_node_id = "") const {
        std::vector<DialoguePage> pages;
        const std::vector<DialogueNode>* conv = getConversation(conversation_id);
        if (!conv || conv->empty()) return pages;

        std::string current_id = start_node_id.empty() ? (*conv)[0].id : start_node_id;
        
        // Simple linear crawler for now. Branching logic usually handled by external state machine 
        // or by restarting begin() with a new subset of nodes.
        while (!current_id.empty()) {
            const DialogueNode* node = getNode(conversation_id, current_id);
            if (!node) break;

            DialoguePage page;
            page.id = node->id;
            page.body = node->body;
            page.variant = node->variant;
            page.choices = node->choices;
            page.command = node->command;
            pages.push_back(page);

            current_id = node->next_node_id;
            // Prevent infinite loops in data
            if (pages.size() > 100) break; 
        }

        return pages;
    }

private:
    DialogueRegistry() = default;
    std::map<std::string, std::vector<DialogueNode>> m_conversations;
};

/**
 * @brief Processes commands triggered by dialogue transitions.
 */
class DialogueCommandProcessor {
public:
    using CommandHandler = std::function<void(const std::string& arg)>;

    void registerHandler(const std::string& command_prefix, CommandHandler handler) {
        m_handlers[command_prefix] = handler;
    }

    void execute(const std::string& full_command) {
        if (full_command.empty()) return;
        
        size_t colon = full_command.find(':');
        std::string prefix = full_command.substr(0, colon);
        std::string arg = (colon != std::string::npos) ? full_command.substr(colon + 1) : "";

        if (m_handlers.count(prefix)) {
            m_handlers[prefix](arg);
        }
    }

private:
    std::map<std::string, CommandHandler> m_handlers;
};

} // namespace urpg::message
