#include "editor/gameplay/gameplay_recipe_panel.h"

#include <utility>

namespace urpg::editor::gameplay {

namespace {

nlohmann::json parameterToJson(const urpg::gameplay::GameplayRecipeParameter& parameter,
                               const urpg::gameplay::GameplayRecipeParameterValues& values) {
    nlohmann::json json{{"key", parameter.key}, {"label", parameter.label}};
    if (parameter.kind == urpg::gameplay::GameplayRecipeParameterKind::String) {
        json["kind"] = "string";
        json["value"] = values.strings.contains(parameter.key) ? values.strings.at(parameter.key) : parameter.default_string;
    } else {
        json["kind"] = "integer";
        json["value"] = values.integers.contains(parameter.key) ? values.integers.at(parameter.key) : parameter.default_integer;
        json["has_range"] = parameter.has_integer_range;
        json["minimum"] = parameter.minimum_integer;
        json["maximum"] = parameter.maximum_integer;
    }
    return json;
}

nlohmann::json diagnosticToJson(const urpg::gameplay::GameplayRecipeDiagnostic& diagnostic) {
    return {{"code", diagnostic.code}, {"message", diagnostic.message}, {"target_id", diagnostic.target_id}};
}

} // namespace

void GameplayRecipePanel::loadProject(urpg::gameplay::GameplayRecipeProjectDocument project) {
    project_ = std::move(project);
    project_document_dirty_ = false;
    snapshot_.status.clear();
    refresh();
}

void GameplayRecipePanel::selectRecipe(urpg::gameplay::GameplayRecipe recipe) {
    template_recipe_ = std::move(recipe);
    parameter_values_ = template_recipe_.defaultParameterValues();
    const auto parameterized = template_recipe_.parameterize(parameter_values_);
    selected_recipe_ = parameterized.recipe;
    parameter_diagnostics_ = parameterized.diagnostics;
    snapshot_.status.clear();
    refresh();
}

bool GameplayRecipePanel::setSelectedStringParameter(const std::string& key, std::string value) {
    auto candidate = parameter_values_;
    candidate.strings[key] = std::move(value);
    const auto parameterized = template_recipe_.parameterize(candidate);
    parameter_values_ = std::move(candidate);
    parameter_diagnostics_ = parameterized.diagnostics;
    if (parameterized.success) {
        selected_recipe_ = parameterized.recipe;
        snapshot_.status.clear();
    } else {
        snapshot_.status = "parameterization_rejected";
    }
    refresh();
    return parameterized.success;
}

bool GameplayRecipePanel::setSelectedIntegerParameter(const std::string& key, int32_t value) {
    auto candidate = parameter_values_;
    candidate.integers[key] = value;
    const auto parameterized = template_recipe_.parameterize(candidate);
    parameter_values_ = std::move(candidate);
    parameter_diagnostics_ = parameterized.diagnostics;
    if (parameterized.success) {
        selected_recipe_ = parameterized.recipe;
        snapshot_.status.clear();
    } else {
        snapshot_.status = "parameterization_rejected";
    }
    refresh();
    return parameterized.success;
}

bool GameplayRecipePanel::applySelectedRecipe() {
    const auto result = service_.apply(project_, selected_recipe_);
    if (result.success && !result.already_applied) {
        project_document_dirty_ = true;
    }
    snapshot_.status = result.code;
    refresh();
    return result.success;
}

bool GameplayRecipePanel::revertSelectedRecipe() {
    const auto result = service_.revert(project_, selected_recipe_.id);
    if (result.success) {
        project_document_dirty_ = true;
    }
    snapshot_.status = result.code;
    refresh();
    return result.success;
}

void GameplayRecipePanel::render() {
    refresh();
}

void GameplayRecipePanel::refresh() {
    snapshot_.has_recipe = !selected_recipe_.id.empty();
    snapshot_.recipe_id = selected_recipe_.id;
    snapshot_.recipe_version = selected_recipe_.version;
    snapshot_.parameters = nlohmann::json::array();
    for (const auto& parameter : template_recipe_.parameters) {
        snapshot_.parameters.push_back(parameterToJson(parameter, parameter_values_));
    }
    if (!snapshot_.has_recipe) {
        snapshot_.can_apply = false;
        snapshot_.can_revert = false;
        snapshot_.already_applied = false;
        snapshot_.preview = nlohmann::json::object();
        if (snapshot_.status.empty()) {
            snapshot_.status = "select_recipe";
        }
        return;
    }
    const auto preview = service_.preview(project_, selected_recipe_);
    snapshot_.can_apply = preview.can_apply && parameter_diagnostics_.empty();
    snapshot_.can_revert = project_.recipe_receipts.contains(selected_recipe_.id);
    snapshot_.already_applied = preview.already_applied;
    snapshot_.preview = urpg::gameplay::gameplayRecipePreviewToJson(preview);
    for (const auto& diagnostic : parameter_diagnostics_) {
        snapshot_.preview["diagnostics"].push_back(diagnosticToJson(diagnostic));
    }
    if (snapshot_.status.empty()) {
        snapshot_.status = preview.already_applied ? "already_applied" : (preview.can_apply ? "ready" : "diagnostics");
    }
}

} // namespace urpg::editor::gameplay
