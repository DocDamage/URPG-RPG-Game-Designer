#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace urpg::gameplay {

struct GameplayWysiwygDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct GameplayWysiwygRule {
    std::string id;
    std::string label;
    std::string trigger;
    std::string target;
    std::string effect;
    int32_t value = 0;
    int32_t duration = 0;
    std::vector<std::string> required_flags;
    std::vector<std::string> grants_flags;
    std::map<std::string, std::string> variable_writes;
    std::map<std::string, int32_t> resource_delta;
};

struct GameplayWysiwygState {
    std::set<std::string> flags;
    std::map<std::string, std::string> variables;
    std::map<std::string, int32_t> resources;
};

struct GameplayWysiwygEvent {
    std::string rule_id;
    std::string target;
    std::string effect;
    int32_t value = 0;
    int32_t duration = 0;

    bool operator==(const GameplayWysiwygEvent& other) const = default;
};

struct GameplayWysiwygPreview {
    std::string feature_id;
    std::string feature_type;
    std::string trigger;
    std::vector<GameplayWysiwygRule> active_rules;
    std::vector<GameplayWysiwygEvent> events;
    GameplayWysiwygState resulting_state;
    std::vector<GameplayWysiwygDiagnostic> diagnostics;
};

class GameplayWysiwygDocument {
public:
    std::string schema_version = "urpg.gameplay_wysiwyg.v1";
    std::string id;
    std::string feature_type;
    std::string display_name;
    std::vector<std::string> visual_layers;
    std::vector<GameplayWysiwygRule> rules;

    [[nodiscard]] std::vector<GameplayWysiwygDiagnostic> validate() const;
    [[nodiscard]] GameplayWysiwygPreview preview(const GameplayWysiwygState& state, const std::string& trigger) const;
    [[nodiscard]] GameplayWysiwygPreview execute(GameplayWysiwygState& state, const std::string& trigger) const;
    [[nodiscard]] nlohmann::json toJson() const;

    static GameplayWysiwygDocument fromJson(const nlohmann::json& json);
};

std::vector<std::string> gameplayWysiwygFeatureTypes();
nlohmann::json gameplayWysiwygPreviewToJson(const GameplayWysiwygPreview& preview);

} // namespace urpg::gameplay
