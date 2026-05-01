#include "editor/ui/theme_builder_panel.h"

namespace urpg::editor::ui {

std::string ThemeBuilderPanel::snapshotLabel(const urpg::ui::ThemePreviewSnapshot& snapshot) {
    return snapshot.screens.empty() ? "theme:empty" : "theme:preview-ready";
}

void ThemeBuilderPanel::setProjectAssetOptions(std::vector<ProjectAssetOption> options) {
    project_asset_options_.clear();
    for (auto& option : options) {
        const bool targets_ui =
            std::find(option.picker_targets.begin(), option.picker_targets.end(), "ui_theme_selector") !=
            option.picker_targets.end();
        if (targets_ui || option.picker_kind == "ui") {
            project_asset_options_.push_back(std::move(option));
        }
    }
    if (!selected_project_asset_id_.empty()) {
        const auto selected = std::find_if(project_asset_options_.begin(), project_asset_options_.end(),
                                           [&](const auto& option) {
                                               return option.asset_id == selected_project_asset_id_;
                                           });
        if (selected == project_asset_options_.end()) {
            selected_project_asset_id_.clear();
        }
    }
}

bool ThemeBuilderPanel::selectProjectAsset(std::string asset_id) {
    const auto selected = std::find_if(project_asset_options_.begin(), project_asset_options_.end(),
                                       [&](const auto& option) {
                                           return option.asset_id == asset_id;
                                       });
    if (selected == project_asset_options_.end()) {
        return false;
    }
    selected_project_asset_id_ = std::move(asset_id);
    return true;
}

ThemeBuilderPanel::ProjectAssetSelectorSnapshot ThemeBuilderPanel::renderProjectAssetSelector() const {
    return {project_asset_options_, selected_project_asset_id_};
}

} // namespace urpg::editor::ui
