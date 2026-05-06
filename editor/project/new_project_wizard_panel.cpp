#include "editor/project/new_project_wizard_panel.h"

#include <utility>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {

void NewProjectWizardPanel::bindModel(NewProjectWizardModel* model) {
    model_ = model;
}

void NewProjectWizardPanel::setTemplateStartCallback(TemplateStartCallback callback) {
    template_start_callback_ = std::move(callback);
}

bool NewProjectWizardPanel::startSelectedTemplate(std::string* error_message) {
    if (model_ == nullptr) {
        if (error_message != nullptr) {
            *error_message = "new_project_wizard_model_missing";
        }
        return false;
    }
    const auto manifestPath = model_->selectedGameMakerTemplateManifestPath();
    if (!manifestPath.has_value()) {
        if (error_message != nullptr) {
            *error_message = "game_maker_template_not_selected";
        }
        return false;
    }
    if (!template_start_callback_) {
        if (error_message != nullptr) {
            *error_message = "game_maker_template_start_callback_missing";
        }
        return false;
    }
    return template_start_callback_(*manifestPath, error_message);
}

void NewProjectWizardPanel::render() {
    if (!model_) {
        snapshot_ = {
            {"panel", "new_project_wizard"},
            {"status", "disabled"},
            {"disabled_reason", "No NewProjectWizardModel is bound."},
            {"owner", "editor/project"},
            {"unlock_condition", "Bind NewProjectWizardModel before rendering the new project wizard."},
        };
        return;
    }
    snapshot_ = {
        {"panel", "new_project_wizard"},
        {"status", "ready"},
        {"model", model_->snapshot()},
    };
#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui::SetNextWindowSize(ImVec2(760.0f, 560.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("New Project")) {
            const auto modelSnapshot = snapshot_["model"];
            ImGui::TextUnformatted("New Project");
            ImGui::SeparatorText("Templates");
            for (const auto& manifest : modelSnapshot.value("game_maker_templates", nlohmann::json::array())) {
                const auto templateId = manifest.value("templateId", "");
                const auto displayName = manifest.value("displayName", templateId);
                const bool selected =
                    modelSnapshot.value("selected_game_maker_template", nlohmann::json::object())
                        .value("templateId", "") == templateId;
                if (ImGui::Selectable(displayName.c_str(), selected)) {
                    (void)model_->selectGameMakerTemplate(templateId);
                }
            }
            std::string error;
            if (ImGui::Button("Create From Template", ImVec2(-1.0f, 0.0f))) {
                (void)startSelectedTemplate(&error);
            }
            if (!error.empty()) {
                ImGui::TextDisabled("%s", error.c_str());
            }
        }
        ImGui::End();
    }
#endif
}

nlohmann::json NewProjectWizardPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
