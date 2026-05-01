#pragma once

#include "engine/core/ui/theme_registry.h"

#include <algorithm>
#include <string>
#include <vector>

namespace urpg::editor::ui {

class ThemeBuilderPanel {
public:
    struct ProjectAssetOption {
        std::string asset_id;
        std::string label;
        std::string project_path;
        std::string picker_kind;
        std::string category;
        std::vector<std::string> picker_targets;
    };

    struct ProjectAssetSelectorSnapshot {
        std::vector<ProjectAssetOption> project_asset_options;
        std::string selected_project_asset_id;
    };

    [[nodiscard]] static std::string snapshotLabel(const urpg::ui::ThemePreviewSnapshot& snapshot);
    void setProjectAssetOptions(std::vector<ProjectAssetOption> options);
    bool selectProjectAsset(std::string asset_id);
    [[nodiscard]] ProjectAssetSelectorSnapshot renderProjectAssetSelector() const;

private:
    std::vector<ProjectAssetOption> project_asset_options_;
    std::string selected_project_asset_id_;
};

} // namespace urpg::editor::ui
