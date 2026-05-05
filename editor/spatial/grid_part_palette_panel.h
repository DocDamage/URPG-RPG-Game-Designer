#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/map/grid_part_catalog.h"

#include <optional>
#include <unordered_map>
#include <string>
#include <vector>

namespace urpg::editor {

class GridPartPalettePanel : public EditorPanel {
  public:
    struct EntrySnapshot {
        std::string part_id;
        std::string display_name;
        std::string category;
        std::string asset_id;
        std::string prefab_path;
        std::string preview_path;
        std::string source_image_path;
        std::string source_pack;
        int32_t atlas_x = 0;
        int32_t atlas_y = 0;
        int32_t atlas_width = 0;
        int32_t atlas_height = 0;
        int32_t tile_id = 0;
        bool selected = false;
    };

    struct RenderSnapshot {
        bool visible = true;
        bool has_catalog = false;
        std::string search_query;
        std::string active_category = "all";
        std::string active_source = "all";
        std::string selected_part_id;
        size_t part_count = 0;
        bool entry_limit_reached = false;
        std::unordered_map<std::string, size_t> category_counts;
        std::unordered_map<std::string, size_t> source_counts;
        std::vector<EntrySnapshot> entries;
    };

    GridPartPalettePanel() : EditorPanel("Grid Part Palette") {}

    void Render(const urpg::FrameContext& context) override;

    void SetCatalog(const urpg::map::GridPartCatalog* catalog);
    void SetSearchQuery(std::string query);
    void SetCategoryFilter(std::optional<urpg::map::GridPartCategory> category);
    void SetSourceFilter(std::string source);
    bool SelectPart(const std::string& part_id);
    bool SelectPartByIndex(size_t index);

    const std::string& selectedPartId() const { return selected_part_id_; }
    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

  private:
    void captureRenderSnapshot();
    static std::string categoryName(urpg::map::GridPartCategory category);
    static std::string sourcePackName(const urpg::map::GridPartDefinition& definition);

    const urpg::map::GridPartCatalog* catalog_ = nullptr;
    std::optional<urpg::map::GridPartCategory> category_filter_;
    std::string source_filter_;
    std::string search_query_;
    std::string selected_part_id_;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
