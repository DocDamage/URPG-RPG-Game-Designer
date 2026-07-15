#pragma once

#include "engine/core/gameplay/gameplay_wysiwyg_system.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace urpg::gameplay {

struct GameplayRecipeDiagnostic {
    std::string code;
    std::string message;
    std::string target_id;
};

enum class GameplayRecipeParameterKind {
    Invalid,
    String,
    Integer,
};

enum class GameplayRecipeParameterField {
    Invalid,
    RuleTarget,
    RuleEffect,
    RuleValue,
    RuleDuration,
    RuleVariableWriteValue,
    RuleResourceDeltaValue,
};

struct GameplayRecipeParameter {
    std::string key;
    std::string label;
    GameplayRecipeParameterKind kind = GameplayRecipeParameterKind::String;
    std::string default_string;
    int32_t default_integer = 0;
    bool has_integer_range = false;
    int32_t minimum_integer = 0;
    int32_t maximum_integer = 0;
};

struct GameplayRecipeParameterBinding {
    std::string parameter_key;
    std::string rule_id;
    GameplayRecipeParameterField field = GameplayRecipeParameterField::RuleTarget;
    std::string map_key;
};

struct GameplayRecipeParameterValues {
    std::map<std::string, std::string> strings;
    std::map<std::string, int32_t> integers;
};

struct GameplayRecipeParameterizationResult;

struct GameplayRecipe {
    std::string schema_version = "urpg.gameplay_recipe.v1";
    std::string id;
    std::string version;
    std::string display_name;
    GameplayWysiwygDocument target;
    std::vector<GameplayRecipeParameter> parameters;
    std::vector<GameplayRecipeParameterBinding> parameter_bindings;

    [[nodiscard]] std::vector<GameplayRecipeDiagnostic> validate() const;
    [[nodiscard]] GameplayRecipeParameterValues defaultParameterValues() const;
    [[nodiscard]] GameplayRecipeParameterizationResult parameterize(const GameplayRecipeParameterValues& values) const;
    [[nodiscard]] nlohmann::json toJson() const;
    static GameplayRecipe fromJson(const nlohmann::json& json);
};

struct GameplayRecipeParameterizationResult {
    bool success = false;
    GameplayRecipe recipe;
    std::vector<GameplayRecipeDiagnostic> diagnostics;
};

struct GameplayRecipeReceipt {
    std::string recipe_id;
    std::string recipe_version;
    std::string target_id;
    GameplayWysiwygDocument applied_target;
};

class GameplayRecipeProjectDocument {
public:
    std::string schema_version = "urpg.gameplay_recipe_project.v1";
    std::map<std::string, GameplayWysiwygDocument> features;
    std::map<std::string, GameplayRecipeReceipt> recipe_receipts;

    [[nodiscard]] std::vector<GameplayRecipeDiagnostic> validate() const;
    [[nodiscard]] nlohmann::json toJson() const;
    static GameplayRecipeProjectDocument fromJson(const nlohmann::json& json);
};

struct GameplayRecipePreview {
    bool can_apply = false;
    bool already_applied = false;
    GameplayWysiwygPreview runtime_preview;
    std::vector<GameplayRecipeDiagnostic> diagnostics;
};

struct GameplayRecipeCommandResult {
    bool success = false;
    bool already_applied = false;
    std::string code;
    std::vector<GameplayRecipeDiagnostic> diagnostics;
};

class GameplayRecipeService {
public:
    [[nodiscard]] GameplayRecipePreview preview(const GameplayRecipeProjectDocument& project,
                                                const GameplayRecipe& recipe,
                                                const GameplayWysiwygState& state = {},
                                                const std::string& trigger = "default") const;
    GameplayRecipeCommandResult apply(GameplayRecipeProjectDocument& project, const GameplayRecipe& recipe) const;
    GameplayRecipeCommandResult revert(GameplayRecipeProjectDocument& project, const std::string& recipe_id) const;
};

std::vector<GameplayRecipe> builtInGameplayRecipeTemplates();
nlohmann::json gameplayRecipePreviewToJson(const GameplayRecipePreview& preview);

} // namespace urpg::gameplay
