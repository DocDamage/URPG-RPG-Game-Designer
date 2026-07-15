#include "engine/core/gameplay/gameplay_recipe_document.h"

#include <algorithm>
#include <set>
#include <utility>

namespace urpg::gameplay {
namespace {

GameplayRecipeDiagnostic diagnostic(std::string code, std::string message, std::string targetId = {}) {
    return {std::move(code), std::move(message), std::move(targetId)};
}

nlohmann::json diagnosticToJson(const GameplayRecipeDiagnostic& value) {
    return {{"code", value.code}, {"message", value.message}, {"target_id", value.target_id}};
}

std::string parameterKindToString(GameplayRecipeParameterKind kind) {
    switch (kind) {
    case GameplayRecipeParameterKind::Invalid:
        return "invalid";
    case GameplayRecipeParameterKind::String:
        return "string";
    case GameplayRecipeParameterKind::Integer:
        return "integer";
    }
    return "string";
}

GameplayRecipeParameterKind parameterKindFromString(const std::string& kind) {
    if (kind == "string") return GameplayRecipeParameterKind::String;
    if (kind == "integer") return GameplayRecipeParameterKind::Integer;
    return GameplayRecipeParameterKind::Invalid;
}

std::string parameterFieldToString(GameplayRecipeParameterField field) {
    switch (field) {
    case GameplayRecipeParameterField::Invalid:
        return "invalid";
    case GameplayRecipeParameterField::RuleTarget:
        return "rule_target";
    case GameplayRecipeParameterField::RuleEffect:
        return "rule_effect";
    case GameplayRecipeParameterField::RuleValue:
        return "rule_value";
    case GameplayRecipeParameterField::RuleDuration:
        return "rule_duration";
    case GameplayRecipeParameterField::RuleVariableWriteValue:
        return "rule_variable_write_value";
    case GameplayRecipeParameterField::RuleResourceDeltaValue:
        return "rule_resource_delta_value";
    }
    return "rule_target";
}

GameplayRecipeParameterField parameterFieldFromString(const std::string& field) {
    if (field == "rule_target") return GameplayRecipeParameterField::RuleTarget;
    if (field == "rule_effect") return GameplayRecipeParameterField::RuleEffect;
    if (field == "rule_value") return GameplayRecipeParameterField::RuleValue;
    if (field == "rule_duration") return GameplayRecipeParameterField::RuleDuration;
    if (field == "rule_variable_write_value") return GameplayRecipeParameterField::RuleVariableWriteValue;
    if (field == "rule_resource_delta_value") return GameplayRecipeParameterField::RuleResourceDeltaValue;
    return GameplayRecipeParameterField::Invalid;
}

nlohmann::json parameterToJson(const GameplayRecipeParameter& parameter) {
    return {{"key", parameter.key},
            {"label", parameter.label},
            {"kind", parameterKindToString(parameter.kind)},
            {"default_string", parameter.default_string},
            {"default_integer", parameter.default_integer},
            {"has_integer_range", parameter.has_integer_range},
            {"minimum_integer", parameter.minimum_integer},
            {"maximum_integer", parameter.maximum_integer}};
}

GameplayRecipeParameter parameterFromJson(const nlohmann::json& json) {
    GameplayRecipeParameter parameter;
    parameter.key = json.value("key", "");
    parameter.label = json.value("label", "");
    parameter.kind = parameterKindFromString(json.value("kind", "string"));
    parameter.default_string = json.value("default_string", "");
    parameter.default_integer = json.value("default_integer", int32_t{0});
    parameter.has_integer_range = json.value("has_integer_range", false);
    parameter.minimum_integer = json.value("minimum_integer", int32_t{0});
    parameter.maximum_integer = json.value("maximum_integer", int32_t{0});
    return parameter;
}

nlohmann::json parameterBindingToJson(const GameplayRecipeParameterBinding& binding) {
    return {{"parameter_key", binding.parameter_key},
            {"rule_id", binding.rule_id},
            {"field", parameterFieldToString(binding.field)},
            {"map_key", binding.map_key}};
}

GameplayRecipeParameterBinding parameterBindingFromJson(const nlohmann::json& json) {
    GameplayRecipeParameterBinding binding;
    binding.parameter_key = json.value("parameter_key", "");
    binding.rule_id = json.value("rule_id", "");
    binding.field = parameterFieldFromString(json.value("field", "rule_target"));
    binding.map_key = json.value("map_key", "");
    return binding;
}

const GameplayWysiwygRule* findRule(const GameplayWysiwygDocument& target, const std::string& ruleId) {
    const auto found = std::find_if(target.rules.begin(), target.rules.end(), [&ruleId](const auto& rule) {
        return rule.id == ruleId;
    });
    return found == target.rules.end() ? nullptr : &(*found);
}

GameplayWysiwygRule* findRule(GameplayWysiwygDocument& target, const std::string& ruleId) {
    const auto found = std::find_if(target.rules.begin(), target.rules.end(), [&ruleId](const auto& rule) {
        return rule.id == ruleId;
    });
    return found == target.rules.end() ? nullptr : &(*found);
}

bool sameDocument(const GameplayWysiwygDocument& left, const GameplayWysiwygDocument& right) {
    return left.toJson() == right.toJson();
}

GameplayRecipeReceipt receiptFromJson(const nlohmann::json& json) {
    GameplayRecipeReceipt receipt;
    receipt.recipe_id = json.value("recipe_id", "");
    receipt.recipe_version = json.value("recipe_version", "");
    receipt.target_id = json.value("target_id", "");
    receipt.applied_target = GameplayWysiwygDocument::fromJson(json.value("applied_target", nlohmann::json::object()));
    return receipt;
}

nlohmann::json receiptToJson(const GameplayRecipeReceipt& receipt) {
    return {{"recipe_id", receipt.recipe_id},
            {"recipe_version", receipt.recipe_version},
            {"target_id", receipt.target_id},
            {"applied_target", receipt.applied_target.toJson()}};
}

} // namespace

std::vector<GameplayRecipeDiagnostic> GameplayRecipe::validate() const {
    std::vector<GameplayRecipeDiagnostic> diagnostics;
    if (schema_version != "urpg.gameplay_recipe.v1") {
        diagnostics.push_back(diagnostic("invalid_schema_version", "Gameplay recipe has an unsupported schema version.", id));
    }
    if (id.empty()) {
        diagnostics.push_back(diagnostic("missing_recipe_id", "Gameplay recipe requires a stable id."));
    }
    if (version.empty()) {
        diagnostics.push_back(diagnostic("missing_recipe_version", "Gameplay recipe requires a version.", id));
    }
    if (display_name.empty()) {
        diagnostics.push_back(diagnostic("missing_display_name", "Gameplay recipe requires a display name.", id));
    }
    for (const auto& targetDiagnostic : target.validate()) {
        diagnostics.push_back(diagnostic("target_" + targetDiagnostic.code, targetDiagnostic.message, target.id));
    }

    std::map<std::string, const GameplayRecipeParameter*> parametersByKey;
    for (const auto& parameter : parameters) {
        if (parameter.key.empty()) {
            diagnostics.push_back(diagnostic("missing_parameter_key", "Gameplay recipe parameter requires a stable key.", id));
            continue;
        }
        if (!parametersByKey.emplace(parameter.key, &parameter).second) {
            diagnostics.push_back(diagnostic("duplicate_parameter_key", "Gameplay recipe parameter keys must be unique.", parameter.key));
        }
        if (parameter.label.empty()) {
            diagnostics.push_back(diagnostic("missing_parameter_label", "Gameplay recipe parameter requires a display label.", parameter.key));
        }
        if (parameter.kind == GameplayRecipeParameterKind::Invalid) {
            diagnostics.push_back(diagnostic("invalid_parameter_kind", "Gameplay recipe parameter has an unsupported kind.",
                                             parameter.key));
        }
        if (parameter.kind == GameplayRecipeParameterKind::String && parameter.default_string.empty()) {
            diagnostics.push_back(diagnostic("missing_parameter_default", "String gameplay recipe parameters require a non-empty default.",
                                             parameter.key));
        }
        if (parameter.has_integer_range && parameter.minimum_integer > parameter.maximum_integer) {
            diagnostics.push_back(diagnostic("invalid_parameter_range", "Gameplay recipe parameter minimum exceeds maximum.",
                                             parameter.key));
        }
        if (parameter.kind == GameplayRecipeParameterKind::Integer && parameter.has_integer_range &&
            (parameter.default_integer < parameter.minimum_integer || parameter.default_integer > parameter.maximum_integer)) {
            diagnostics.push_back(diagnostic("parameter_default_out_of_range",
                                             "Gameplay recipe integer parameter default is outside its declared range.",
                                             parameter.key));
        }
    }

    std::set<std::string> boundParameterKeys;
    for (const auto& binding : parameter_bindings) {
        const auto parameter = parametersByKey.find(binding.parameter_key);
        if (parameter == parametersByKey.end()) {
            diagnostics.push_back(diagnostic("unknown_parameter_binding", "Gameplay recipe binding refers to an unknown parameter.",
                                             binding.parameter_key));
            continue;
        }
        boundParameterKeys.insert(binding.parameter_key);
        const auto* rule = findRule(target, binding.rule_id);
        if (rule == nullptr) {
            diagnostics.push_back(diagnostic("unknown_parameter_rule", "Gameplay recipe binding refers to an unknown rule.",
                                             binding.rule_id));
            continue;
        }

        if (binding.field == GameplayRecipeParameterField::Invalid) {
            diagnostics.push_back(diagnostic("invalid_parameter_field",
                                             "Gameplay recipe parameter binding has an unsupported rule field.",
                                             binding.parameter_key));
            continue;
        }

        const bool stringField = binding.field == GameplayRecipeParameterField::RuleTarget ||
                                 binding.field == GameplayRecipeParameterField::RuleEffect ||
                                 binding.field == GameplayRecipeParameterField::RuleVariableWriteValue;
        const bool integerField = binding.field == GameplayRecipeParameterField::RuleValue ||
                                  binding.field == GameplayRecipeParameterField::RuleDuration ||
                                  binding.field == GameplayRecipeParameterField::RuleResourceDeltaValue;
        if ((stringField && parameter->second->kind != GameplayRecipeParameterKind::String) ||
            (integerField && parameter->second->kind != GameplayRecipeParameterKind::Integer)) {
            diagnostics.push_back(diagnostic("incompatible_parameter_binding",
                                             "Gameplay recipe parameter kind is incompatible with its bound rule field.",
                                             binding.parameter_key));
        }
        if ((binding.field == GameplayRecipeParameterField::RuleVariableWriteValue &&
             !rule->variable_writes.contains(binding.map_key)) ||
            (binding.field == GameplayRecipeParameterField::RuleResourceDeltaValue &&
             !rule->resource_delta.contains(binding.map_key))) {
            diagnostics.push_back(diagnostic("unknown_parameter_map_key",
                                             "Gameplay recipe parameter binding refers to an unknown rule map key.",
                                             binding.map_key));
        }
    }
    for (const auto& [key, parameter] : parametersByKey) {
        (void)parameter;
        if (!boundParameterKeys.contains(key)) {
            diagnostics.push_back(diagnostic("unbound_parameter",
                                             "Gameplay recipe parameter must bind to at least one named rule field.", key));
        }
    }
    return diagnostics;
}

GameplayRecipeParameterValues GameplayRecipe::defaultParameterValues() const {
    GameplayRecipeParameterValues values;
    for (const auto& parameter : parameters) {
        if (parameter.kind == GameplayRecipeParameterKind::String) {
            values.strings.emplace(parameter.key, parameter.default_string);
        } else {
            values.integers.emplace(parameter.key, parameter.default_integer);
        }
    }
    return values;
}

GameplayRecipeParameterizationResult GameplayRecipe::parameterize(const GameplayRecipeParameterValues& values) const {
    GameplayRecipeParameterizationResult result;
    result.recipe = *this;
    result.diagnostics = validate();

    std::map<std::string, const GameplayRecipeParameter*> parametersByKey;
    for (const auto& parameter : parameters) {
        parametersByKey.emplace(parameter.key, &parameter);
    }
    for (const auto& [key, value] : values.strings) {
        (void)value;
        const auto parameter = parametersByKey.find(key);
        if (parameter == parametersByKey.end() || parameter->second->kind != GameplayRecipeParameterKind::String) {
            result.diagnostics.push_back(diagnostic("unknown_string_parameter",
                                                    "Configured gameplay recipe string parameter is unknown or has a different kind.", key));
        }
    }
    for (const auto& [key, value] : values.integers) {
        (void)value;
        const auto parameter = parametersByKey.find(key);
        if (parameter == parametersByKey.end() || parameter->second->kind != GameplayRecipeParameterKind::Integer) {
            result.diagnostics.push_back(diagnostic("unknown_integer_parameter",
                                                    "Configured gameplay recipe integer parameter is unknown or has a different kind.", key));
        }
    }

    for (const auto& parameter : parameters) {
        if (parameter.kind == GameplayRecipeParameterKind::String) {
            const auto configured = values.strings.find(parameter.key);
            if (configured == values.strings.end() || configured->second.empty()) {
                result.diagnostics.push_back(diagnostic("missing_string_parameter",
                                                        "Gameplay recipe string parameter requires a non-empty value.", parameter.key));
            }
        } else {
            const auto configured = values.integers.find(parameter.key);
            if (configured == values.integers.end()) {
                result.diagnostics.push_back(diagnostic("missing_integer_parameter",
                                                        "Gameplay recipe integer parameter requires a value.", parameter.key));
            } else if (parameter.has_integer_range &&
                       (configured->second < parameter.minimum_integer || configured->second > parameter.maximum_integer)) {
                result.diagnostics.push_back(diagnostic("integer_parameter_out_of_range",
                                                        "Gameplay recipe integer parameter is outside its declared range.", parameter.key));
            }
        }
    }
    if (!result.diagnostics.empty()) return result;

    for (const auto& binding : parameter_bindings) {
        auto* rule = findRule(result.recipe.target, binding.rule_id);
        const auto parameter = parametersByKey.at(binding.parameter_key);
        if (parameter->kind == GameplayRecipeParameterKind::String) {
            const auto& value = values.strings.at(binding.parameter_key);
            switch (binding.field) {
            case GameplayRecipeParameterField::RuleTarget:
                rule->target = value;
                break;
            case GameplayRecipeParameterField::RuleEffect:
                rule->effect = value;
                break;
            case GameplayRecipeParameterField::RuleVariableWriteValue:
                rule->variable_writes.at(binding.map_key) = value;
                break;
            default:
                break;
            }
        } else {
            const auto value = values.integers.at(binding.parameter_key);
            switch (binding.field) {
            case GameplayRecipeParameterField::RuleValue:
                rule->value = value;
                break;
            case GameplayRecipeParameterField::RuleDuration:
                rule->duration = value;
                break;
            case GameplayRecipeParameterField::RuleResourceDeltaValue:
                rule->resource_delta.at(binding.map_key) = value;
                break;
            default:
                break;
            }
        }
    }
    result.diagnostics = result.recipe.validate();
    result.success = result.diagnostics.empty();
    return result;
}

nlohmann::json GameplayRecipe::toJson() const {
    nlohmann::json json{{"schema_version", schema_version},
                        {"id", id},
                        {"version", version},
                        {"display_name", display_name},
                        {"target", target.toJson()},
                        {"parameters", nlohmann::json::array()},
                        {"parameter_bindings", nlohmann::json::array()}};
    for (const auto& parameter : parameters) {
        json["parameters"].push_back(parameterToJson(parameter));
    }
    for (const auto& binding : parameter_bindings) {
        json["parameter_bindings"].push_back(parameterBindingToJson(binding));
    }
    return json;
}

GameplayRecipe GameplayRecipe::fromJson(const nlohmann::json& json) {
    GameplayRecipe recipe;
    recipe.schema_version = json.value("schema_version", "urpg.gameplay_recipe.v1");
    recipe.id = json.value("id", "");
    recipe.version = json.value("version", "");
    recipe.display_name = json.value("display_name", "");
    recipe.target = GameplayWysiwygDocument::fromJson(json.value("target", nlohmann::json::object()));
    for (const auto& parameterJson : json.value("parameters", nlohmann::json::array())) {
        recipe.parameters.push_back(parameterFromJson(parameterJson));
    }
    for (const auto& bindingJson : json.value("parameter_bindings", nlohmann::json::array())) {
        recipe.parameter_bindings.push_back(parameterBindingFromJson(bindingJson));
    }
    return recipe;
}

nlohmann::json GameplayRecipeProjectDocument::toJson() const {
    nlohmann::json featureArray = nlohmann::json::array();
    for (const auto& [id, feature] : features) {
        featureArray.push_back(feature.toJson());
    }
    nlohmann::json receiptArray = nlohmann::json::array();
    for (const auto& [id, receipt] : recipe_receipts) {
        receiptArray.push_back(receiptToJson(receipt));
    }
    return {{"schema_version", schema_version}, {"features", std::move(featureArray)}, {"recipe_receipts", std::move(receiptArray)}};
}

GameplayRecipeProjectDocument GameplayRecipeProjectDocument::fromJson(const nlohmann::json& json) {
    GameplayRecipeProjectDocument project;
    project.schema_version = json.value("schema_version", "urpg.gameplay_recipe_project.v1");
    for (const auto& featureJson : json.value("features", nlohmann::json::array())) {
        auto feature = GameplayWysiwygDocument::fromJson(featureJson);
        if (!feature.id.empty()) {
            project.features.emplace(feature.id, std::move(feature));
        }
    }
    for (const auto& receiptJson : json.value("recipe_receipts", nlohmann::json::array())) {
        auto receipt = receiptFromJson(receiptJson);
        if (!receipt.recipe_id.empty()) {
            project.recipe_receipts.emplace(receipt.recipe_id, std::move(receipt));
        }
    }
    return project;
}

std::vector<GameplayRecipeDiagnostic> GameplayRecipeProjectDocument::validate() const {
    std::vector<GameplayRecipeDiagnostic> diagnostics;
    if (schema_version != "urpg.gameplay_recipe_project.v1") {
        diagnostics.push_back(diagnostic("invalid_project_schema_version",
                                         "Gameplay recipe project has an unsupported schema version."));
    }
    for (const auto& [featureId, feature] : features) {
        if (featureId.empty() || feature.id != featureId) {
            diagnostics.push_back(diagnostic("feature_id_mismatch",
                                             "Gameplay recipe project feature map key does not match its document id.",
                                             featureId));
        }
        for (const auto& featureDiagnostic : feature.validate()) {
            diagnostics.push_back(diagnostic("feature_" + featureDiagnostic.code, featureDiagnostic.message,
                                             featureId));
        }
    }
    for (const auto& [recipeId, receipt] : recipe_receipts) {
        if (recipeId.empty() || receipt.recipe_id != recipeId) {
            diagnostics.push_back(diagnostic("receipt_id_mismatch",
                                             "Gameplay recipe receipt map key does not match its recipe id.", recipeId));
        }
        if (receipt.recipe_version.empty() || receipt.target_id.empty()) {
            diagnostics.push_back(diagnostic("receipt_incomplete",
                                             "Gameplay recipe receipt requires a version and target id.", recipeId));
            continue;
        }
        const auto feature = features.find(receipt.target_id);
        if (feature == features.end()) {
            diagnostics.push_back(diagnostic("receipt_target_missing",
                                             "Gameplay recipe receipt refers to a missing feature target.", receipt.target_id));
        } else if (!sameDocument(feature->second, receipt.applied_target)) {
            diagnostics.push_back(diagnostic("receipt_target_modified",
                                             "Gameplay recipe receipt target differs from the recorded applied document.",
                                             receipt.target_id));
        }
        for (const auto& targetDiagnostic : receipt.applied_target.validate()) {
            diagnostics.push_back(diagnostic("receipt_target_" + targetDiagnostic.code, targetDiagnostic.message,
                                             receipt.target_id));
        }
    }
    return diagnostics;
}

GameplayRecipePreview GameplayRecipeService::preview(const GameplayRecipeProjectDocument& project,
                                                     const GameplayRecipe& recipe,
                                                     const GameplayWysiwygState& state,
                                                     const std::string& trigger) const {
    GameplayRecipePreview preview;
    preview.diagnostics = project.validate();
    const auto recipeDiagnostics = recipe.validate();
    preview.diagnostics.insert(preview.diagnostics.end(), recipeDiagnostics.begin(), recipeDiagnostics.end());
    preview.runtime_preview = recipe.target.preview(state, trigger);
    const auto receipt = project.recipe_receipts.find(recipe.id);
    if (receipt != project.recipe_receipts.end()) {
        preview.already_applied = receipt->second.recipe_version == recipe.version &&
                                  sameDocument(receipt->second.applied_target, recipe.target);
        if (!preview.already_applied) {
            preview.diagnostics.push_back(diagnostic("recipe_version_conflict",
                                                     "This recipe id is already applied with different content or version.",
                                                     recipe.target.id));
        }
    }
    const auto target = project.features.find(recipe.target.id);
    if (target != project.features.end() &&
        (receipt == project.recipe_receipts.end() || !sameDocument(target->second, recipe.target))) {
        preview.diagnostics.push_back(diagnostic("target_conflict",
                                                 "The recipe target is already owned or edited by another native document.",
                                                 recipe.target.id));
    }
    preview.can_apply = preview.diagnostics.empty() && !preview.already_applied;
    return preview;
}

GameplayRecipeCommandResult GameplayRecipeService::apply(GameplayRecipeProjectDocument& project,
                                                          const GameplayRecipe& recipe) const {
    const auto previewResult = preview(project, recipe);
    GameplayRecipeCommandResult result;
    result.diagnostics = previewResult.diagnostics;
    if (previewResult.already_applied && result.diagnostics.empty()) {
        result.success = true;
        result.already_applied = true;
        result.code = "already_applied";
        return result;
    }
    if (!previewResult.can_apply) {
        result.code = "recipe_rejected";
        return result;
    }
    project.features.emplace(recipe.target.id, recipe.target);
    project.recipe_receipts.emplace(recipe.id, GameplayRecipeReceipt{recipe.id, recipe.version, recipe.target.id, recipe.target});
    result.success = true;
    result.code = "applied";
    return result;
}

GameplayRecipeCommandResult GameplayRecipeService::revert(GameplayRecipeProjectDocument& project,
                                                           const std::string& recipe_id) const {
    GameplayRecipeCommandResult result;
    const auto receipt = project.recipe_receipts.find(recipe_id);
    if (receipt == project.recipe_receipts.end()) {
        result.code = "recipe_not_applied";
        result.diagnostics.push_back(diagnostic("recipe_not_applied", "No applied gameplay recipe has this id.", recipe_id));
        return result;
    }
    const auto target = project.features.find(receipt->second.target_id);
    if (target == project.features.end()) {
        result.code = "target_missing";
        result.diagnostics.push_back(diagnostic("target_missing", "Recipe receipt target is missing; nothing was reverted.",
                                                receipt->second.target_id));
        return result;
    }
    if (!sameDocument(target->second, receipt->second.applied_target)) {
        result.code = "target_modified";
        result.diagnostics.push_back(diagnostic("target_modified", "Recipe target changed after apply and cannot be removed safely.",
                                                receipt->second.target_id));
        return result;
    }
    project.features.erase(target);
    project.recipe_receipts.erase(receipt);
    result.success = true;
    result.code = "reverted";
    return result;
}

std::vector<GameplayRecipe> builtInGameplayRecipeTemplates() {
    GameplayRecipe starter;
    starter.id = "urpg.recipe.starter_quest_choice";
    starter.version = "1.0.0";
    starter.display_name = "Starter Quest Choice";
    starter.target.id = "urpg.feature.starter_quest_choice";
    starter.target.feature_type = "quest_choice_consequence";
    starter.target.display_name = "Starter Quest Choice";
    starter.target.visual_layers = {"dialogue", "choice", "world_state"};
    starter.target.rules.push_back({"accept_help", "Accept the request", "confirm_interact", "quest_giver",
                                    "start_quest", 1, 0, {}, {"quest.starter.accepted"},
                                    {{"quest.starter.status", "accepted"}}, {{"reputation", 1}}});
    starter.parameters = {
        {"quest_status", "Quest status", GameplayRecipeParameterKind::String, "accepted"},
        {"reputation_reward", "Reputation reward", GameplayRecipeParameterKind::Integer, "", 1, true, 0, 100},
    };
    starter.parameter_bindings = {
        {"quest_status", "accept_help", GameplayRecipeParameterField::RuleVariableWriteValue, "quest.starter.status"},
        {"reputation_reward", "accept_help", GameplayRecipeParameterField::RuleResourceDeltaValue, "reputation"},
    };

    GameplayRecipe townSignal;
    townSignal.id = "urpg.recipe.town_event_signal";
    townSignal.version = "1.0.0";
    townSignal.display_name = "Town Event Signal";
    townSignal.target.id = "urpg.feature.town_event_signal";
    townSignal.target.feature_type = "world_state_timeline";
    townSignal.target.display_name = "Town Event Signal";
    townSignal.target.visual_layers = {"world_state", "timeline", "dialogue"};
    townSignal.target.rules.push_back({"broadcast_signal", "Broadcast the town signal", "confirm_interact", "town_crier",
                                       "set_world_state", 0, 0, {}, {"world.town.signal"},
                                       {{"world.town.signal", "market_open"}}, {}});
    townSignal.parameters = {
        {"signal_state", "Town signal state", GameplayRecipeParameterKind::String, "market_open"},
    };
    townSignal.parameter_bindings = {
        {"signal_state", "broadcast_signal", GameplayRecipeParameterField::RuleVariableWriteValue,
         "world.town.signal"},
    };
    return {starter, townSignal};
}

nlohmann::json gameplayRecipePreviewToJson(const GameplayRecipePreview& preview) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& value : preview.diagnostics) {
        diagnostics.push_back(diagnosticToJson(value));
    }
    return {{"can_apply", preview.can_apply},
            {"already_applied", preview.already_applied},
            {"runtime_preview", gameplayWysiwygPreviewToJson(preview.runtime_preview)},
            {"diagnostics", std::move(diagnostics)}};
}

} // namespace urpg::gameplay
