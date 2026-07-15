#pragma once

#include "engine/core/gameplay/gameplay_recipe_document.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::editor::gameplay {

struct GameplayRecipePanelSnapshot {
    bool has_recipe = false;
    bool can_apply = false;
    bool can_revert = false;
    bool already_applied = false;
    std::string recipe_id;
    std::string recipe_version;
    std::string status;
    nlohmann::json parameters = nlohmann::json::array();
    nlohmann::json preview = nlohmann::json::object();
};

class GameplayRecipePanel {
public:
    void loadProject(urpg::gameplay::GameplayRecipeProjectDocument project);
    void selectRecipe(urpg::gameplay::GameplayRecipe recipe);
    bool setSelectedStringParameter(const std::string& key, std::string value);
    bool setSelectedIntegerParameter(const std::string& key, int32_t value);
    bool applySelectedRecipe();
    bool revertSelectedRecipe();
    void render();

    [[nodiscard]] const urpg::gameplay::GameplayRecipeProjectDocument& project() const { return project_; }
    [[nodiscard]] bool hasUnsavedProjectDocument() const { return project_document_dirty_; }
    void markProjectDocumentPersisted() { project_document_dirty_ = false; }
    [[nodiscard]] const GameplayRecipePanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    urpg::gameplay::GameplayRecipeProjectDocument project_;
    urpg::gameplay::GameplayRecipe template_recipe_;
    urpg::gameplay::GameplayRecipe selected_recipe_;
    urpg::gameplay::GameplayRecipeParameterValues parameter_values_;
    std::vector<urpg::gameplay::GameplayRecipeDiagnostic> parameter_diagnostics_;
    urpg::gameplay::GameplayRecipeService service_;
    GameplayRecipePanelSnapshot snapshot_;
    bool project_document_dirty_ = false;
};

} // namespace urpg::editor::gameplay
