#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/assets/asset_relink_service.h"
#include <filesystem>
#include <map>
#include <vector>

namespace urpg::editor {

class AssetRelinkPanel : public EditorPanel {
public:
    AssetRelinkPanel() : EditorPanel("Asset Relink Manager") {}

    void Render(const urpg::FrameContext& context) override;
    void setProjectRoot(const std::filesystem::path& root);

private:
    std::filesystem::path project_root_;
    urpg::assets::AssetRelinkService relink_service_;
    std::vector<urpg::assets::MissingAsset> missing_assets_;
    std::map<std::string, size_t> selected_candidates_;
    std::string last_relinked_asset_id_;
    std::string status_message_;
    bool scanned_ = false;
};

} // namespace urpg::editor
