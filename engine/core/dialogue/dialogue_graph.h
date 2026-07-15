#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <set>
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
    // Optional stable authoring reference for the visible choice label.
    // Existing graphs with freeform labels remain valid.
    std::string localization_key;
};

struct DialogueNode {
    std::string id;
    std::string speaker_id;
    std::string speaker_name;
    std::string localization_key;
    std::string text_preview;
    bool ending = false;
    std::vector<DialogueChoice> choices;
    // Optional authoring references. Runtime playback remains unchanged until
    // the dialogue execution owner consumes these stable project identifiers.
    std::string voice_asset_id;
    std::string caption_localization_key;
    int32_t canvas_x = 0;
    int32_t canvas_y = 0;
    bool has_canvas_position = false;
};

struct DialogueGraphDiagnostic {
    std::string code;
    std::string message;
    std::string node_id;
    std::string choice_id;
};

// Authoring-preview state only. It never mutates the saved dialogue graph or
// gameplay/runtime state.
struct DialoguePreviewChoiceState {
    std::string id;
    std::string label;
    std::string localization_key;
    std::string target_node_id;
    bool enabled = false;
    std::vector<DialogueGraphDiagnostic> diagnostics;
};

struct DialoguePreviewTransition {
    bool applied = false;
    std::string next_node_id;
    std::map<std::string, int> values;
    std::vector<DialogueGraphDiagnostic> diagnostics;
};

class DialogueGraph {
public:
    bool addNode(DialogueNode node);
    bool removeNode(const std::string& node_id);
    bool updateNode(const std::string& node_id, std::string speaker_id, std::string speaker_name,
                    std::string localization_key, std::string text_preview, bool ending);
    bool updateNodeMediaReferences(const std::string& node_id, std::string voice_asset_id,
                                   std::string caption_localization_key);
    bool updateNodeCanvasPosition(const std::string& node_id, int32_t canvas_x, int32_t canvas_y);
    bool addChoice(const std::string& node_id, DialogueChoice choice);
    bool removeChoice(const std::string& node_id, const std::string& choice_id);
    bool updateChoice(const std::string& node_id, const std::string& choice_id, std::string label,
                      std::string target_node_id, std::string localization_key);
    bool addChoiceCondition(const std::string& node_id, const std::string& choice_id, DialogueCondition condition);
    bool removeChoiceCondition(const std::string& node_id, const std::string& choice_id,
                               const DialogueCondition& condition);
    bool addChoiceEffect(const std::string& node_id, const std::string& choice_id, DialogueEffect effect);
    bool removeChoiceEffect(const std::string& node_id, const std::string& choice_id, const DialogueEffect& effect);
    void setStartNode(std::string node_id);
    const DialogueNode* findNode(const std::string& node_id) const;
    const std::map<std::string, DialogueNode>& nodes() const;
    const std::string& startNode() const;
    std::vector<std::string> previewRoute(std::size_t max_steps = 16) const;
    std::vector<DialoguePreviewChoiceState> previewChoices(
        const std::string& node_id, const std::map<std::string, int>& values) const;
    DialoguePreviewTransition previewChoice(const std::string& node_id, const std::string& choice_id,
                                            const std::map<std::string, int>& values) const;
    // Authoring-only diagnostics. These do not change preview traversal or
    // runtime dialogue behavior for existing graphs.
    std::vector<DialogueGraphDiagnostic> validate() const;
    std::vector<DialogueGraphDiagnostic> analyzeFlow() const;
    std::vector<DialogueGraphDiagnostic> validateLocalizationKeys(const std::set<std::string>& localization_keys) const;
    std::vector<DialogueGraphDiagnostic> validateVoiceAssetIds(const std::set<std::string>& voice_asset_ids) const;
    static std::optional<DialogueGraph> fromJson(const nlohmann::json& json);
    nlohmann::json serialize() const;

private:
    std::string start_node_id_;
    std::map<std::string, DialogueNode> nodes_;
};

} // namespace urpg::dialogue
