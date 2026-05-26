#pragma once

#include "editor/spatial/elevation_brush_panel.h"
#include "editor/spatial/grid_part_inspector_panel.h"
#include "editor/spatial/grid_part_palette_panel.h"
#include "editor/spatial/grid_part_placement_panel.h"
#include "editor/spatial/map_environment_preview_panel.h"
#include "editor/spatial/map_ability_binding_panel.h"
#include "editor/spatial/procedural_map_panel.h"
#include "editor/spatial/prop_placement_panel.h"
#include "editor/spatial/region_rules_panel.h"
#include "editor/spatial/spatial_ability_canvas_panel.h"
#include "editor/spatial/terrain_brush_panel.h"
#include "editor/ui/editor_panel.h"

#include <string>
#include <vector>

namespace urpg::scene {
class MapScene;
}

namespace urpg::editor {

class SpatialAuthoringWorkspace : public EditorPanel {
  public:
    enum class ToolMode {
        Composite = 0,
        Elevation = 1,
        Props = 2,
        Abilities = 3,
        Parts = 4,
        Worldbuilding = 5,
    };

    struct ToolbarAction {
        std::string id;
        std::string label;
        bool active = false;
        bool enabled = true;
    };

    struct ToolbarSnapshot {
        std::string active_mode;
        std::string selected_trigger_id;
        std::string selected_ability_id;
        std::string selected_prop_asset_id;
        int placement_tile_x = 0;
        int placement_tile_y = 0;
        bool has_conflicts = false;
        size_t conflict_count = 0;
        bool can_apply_suggested_conflict_resolution = false;
        std::vector<ToolbarAction> actions;
    };

    struct RenderSnapshot {
        std::string status = "disabled";
        std::string message = "No Perspective 2D map targets are bound.";
        std::string remediation = "Bind a MapScene and SpatialMapOverlay before using Perspective 2D tools.";
        bool visible = true;
        bool has_target_scene = false;
        bool has_target_overlay = false;
        ToolbarSnapshot toolbar;
        ElevationBrushPanel::RenderSnapshot elevation;
        PropPlacementPanel::RenderSnapshot props;
        GridPartPalettePanel::RenderSnapshot parts_palette;
        GridPartPlacementPanel::RenderSnapshot parts_placement;
        GridPartInspectorPanel::RenderSnapshot parts_inspector;
        TerrainBrushPanelSnapshot worldbuilding_terrain;
        RegionRulesPanelSnapshot worldbuilding_regions;
        ProceduralMapPanelSnapshot worldbuilding_procedural;
        MapEnvironmentPreviewPanelSnapshot worldbuilding_environment;
        MapAbilityBindingPanel::RenderSnapshot bindings;
        SpatialAbilityCanvasPanel::RenderSnapshot canvas;
    };

    SpatialAuthoringWorkspace() : EditorPanel("Perspective 2D Map Editor") {}

    void Render(const urpg::FrameContext& context) override;
    void SetTargets(urpg::scene::MapScene* scene, urpg::presentation::SpatialMapOverlay* overlay);
    void SetGridPartTargets(urpg::map::GridPartDocument* document, const urpg::map::GridPartCatalog* catalog);
    bool SetProjectRoot(const std::string& root_path);
    void SetProjectionSettings(const PropPlacementPanel::ScreenProjectionSettings& settings);
    void SetAvailableTriggers(std::vector<std::string> trigger_ids);
    void PreviewTerrainBrush(const urpg::map::TerrainBrush& brush, int32_t x, int32_t y, uint32_t seed);
    void LoadRegionRules(std::vector<urpg::map::MapRegionRule> rules);
    void GenerateProceduralMap(const urpg::map::ProceduralMapProfile& profile);
    void LoadEnvironmentPreview(urpg::map::MapEnvironmentPreviewDocument document);
    void SelectEnvironmentTile(int32_t x, int32_t y);
    void SetActiveMode(ToolMode mode);
    bool ActivateToolbarAction(const std::string& action_id);
    bool ActivateCanvasAction(const std::string& action_id);
    bool RouteCanvasPrimaryAction(float screen_x, float screen_y);
    bool RouteCanvasSecondaryAction(float screen_x, float screen_y);
    bool RouteCanvasHover(float screen_x, float screen_y);
    bool SelectGridPart(const std::string& part_id);
    ToolMode activeMode() const { return active_mode_; }

    ElevationBrushPanel& elevationPanel() { return elevation_panel_; }
    PropPlacementPanel& propPanel() { return prop_panel_; }
    GridPartPalettePanel& gridPartPalettePanel() { return grid_part_palette_panel_; }
    GridPartPlacementPanel& gridPartPlacementPanel() { return grid_part_placement_panel_; }
    GridPartInspectorPanel& gridPartInspectorPanel() { return grid_part_inspector_panel_; }
    TerrainBrushPanel& terrainBrushPanel() { return terrain_brush_panel_; }
    RegionRulesPanel& regionRulesPanel() { return region_rules_panel_; }
    ProceduralMapPanel& proceduralMapPanel() { return procedural_map_panel_; }
    MapEnvironmentPreviewPanel& environmentPreviewPanel() { return environment_preview_panel_; }
    MapAbilityBindingPanel& bindingPanel() { return binding_panel_; }
    SpatialAbilityCanvasPanel& canvasPanel() { return canvas_panel_; }

    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

  private:
    void captureRenderSnapshot();
    void syncPanelVisibility();
    static const char* modeName(ToolMode mode);

    urpg::scene::MapScene* m_target_scene = nullptr;
    urpg::presentation::SpatialMapOverlay* m_target_overlay = nullptr;
    urpg::map::GridPartDocument* grid_part_document_ = nullptr;
    const urpg::map::GridPartCatalog* grid_part_catalog_ = nullptr;
    ToolMode active_mode_ = ToolMode::Composite;
    ElevationBrushPanel elevation_panel_;
    PropPlacementPanel prop_panel_;
    GridPartPalettePanel grid_part_palette_panel_;
    GridPartPlacementPanel grid_part_placement_panel_;
    GridPartInspectorPanel grid_part_inspector_panel_;
    TerrainBrushPanel terrain_brush_panel_;
    RegionRulesPanel region_rules_panel_;
    ProceduralMapPanel procedural_map_panel_;
    MapEnvironmentPreviewPanel environment_preview_panel_;
    MapAbilityBindingPanel binding_panel_;
    SpatialAbilityCanvasPanel canvas_panel_;
    PropPlacementPanel::ScreenProjectionSettings projection_settings_;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
