#pragma once

#include "editor/spatial/prop_placement_panel.h"
#include "editor/ui/editor_panel.h"
#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_commands.h"
#include "engine/core/map/grid_part_validator.h"
#include "engine/core/presentation/presentation_schema.h"

#include <string>

namespace urpg::editor {

class GridPartPlacementPanel : public EditorPanel {
  public:
    struct RenderSnapshot {
        bool visible = true;
        bool has_document = false;
        bool has_catalog = false;
        bool has_spatial_overlay = false;

        std::string selected_part_id;

        bool hover_active = false;
        bool hover_valid = false;
        std::string hover_reason;

        int32_t hover_x = -1;
        int32_t hover_y = -1;
        int32_t footprint_width = 1;
        int32_t footprint_height = 1;

        size_t placed_count = 0;
        size_t diagnostic_count = 0;

        bool can_undo = false;
        bool can_redo = false;
    };

    GridPartPlacementPanel() : EditorPanel("Grid Part Placement") {}

    void Render(const urpg::FrameContext& context) override;

    void SetTargets(urpg::map::GridPartDocument* document, const urpg::map::GridPartCatalog* catalog,
                    urpg::presentation::SpatialMapOverlay* overlay);
    void SetProjectionSettings(const PropPlacementPanel::ScreenProjectionSettings& settings);
    bool SetSelectedPartId(const std::string& part_id);
    bool HoverSelectedPartAtGrid(int32_t grid_x, int32_t grid_y);
    bool HoverSelectedPartFromScreen(float screen_x, float screen_y);
    bool PlaceSelectedPartAtGrid(int32_t grid_x, int32_t grid_y);
    bool PlaceSelectedPartFromScreen(float screen_x, float screen_y);
    bool FillSelectedPartRectangle(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y);
    bool Undo();
    bool Redo();

    const std::string& selectedPartId() const { return selected_part_id_; }
    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

  private:
    void captureRenderSnapshot();
    bool projectScreenToGrid(float screen_x, float screen_y, int32_t& out_grid_x, int32_t& out_grid_y) const;
    urpg::map::PlacedPartInstance makeInstance(const urpg::map::GridPartDefinition& definition, int32_t grid_x,
                                               int32_t grid_y) const;
    std::string makeInstanceId(const std::string& part_id, int32_t grid_x, int32_t grid_y) const;

    urpg::map::GridPartDocument* document_ = nullptr;
    const urpg::map::GridPartCatalog* catalog_ = nullptr;
    urpg::presentation::SpatialMapOverlay* overlay_ = nullptr;
    PropPlacementPanel::ScreenProjectionSettings projection_settings_;
    urpg::map::GridPartCommandHistory history_;
    std::string selected_part_id_;
    bool hover_active_ = false;
    bool hover_valid_ = false;
    std::string hover_reason_;
    int32_t hover_x_ = -1;
    int32_t hover_y_ = -1;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
