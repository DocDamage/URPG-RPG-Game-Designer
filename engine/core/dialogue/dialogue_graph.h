#pragma once

#include <nlohmann/json.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace urpg::dialogue {

struct DialogueCondition {
    std::string key;
    std::string op;
    int value = 0;
};

struct DialogueEffect {
    std::string key;
    int delta = 0;
};

struct DialogueChoice {
    std::string id;
    std::string label;
    std::string target_node_id;
    std::vector<DialogueCondition> conditions;
    std::vector<DialogueEffect> effects;
};

struct DialogueNode {
    std::string id;
    std::string speaker_id;
    std::string speaker_name;
    std::string localization_key;
    std::string text_preview;
    bool ending = false;
    std::vector<DialogueChoice> choices;
};

class DialogueGraph {
public:
    bool addNode(DialogueNode node);
    void setStartNode(std::string node_id);
    const DialogueNode* findNode(const std::string& node_id) const;
    const std::map<std::string, DialogueNode>& nodes() const;
    const std::string& startNode() const;
    std::vector<std::string> previewRoute(std::size_t max_steps = 16) const;
    nlohmann::json serialize() const;

private:
    std::string start_node_id_;
    std::map<std::string, DialogueNode> nodes_;
};

} // namespace urpg::dialogue
