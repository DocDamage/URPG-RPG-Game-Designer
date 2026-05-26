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

#include <cstdint>
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
        Tiles = 6,
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

    struct Perspective2DLayerSnapshot {
        std::string id;
        std::string label;
        std::string kind;
        bool visible = true;
        bool locked = false;
        int order = 0;
        size_t tile_count = 0;
        size_t event_count = 0;
        bool selected = false;
    };

    struct Perspective2DPaletteOptionSnapshot {
        std::string option_id;
        std::string label;
        std::string tileset_id;
        std::string tile_id;
        std::string asset_id;
        std::string project_path;
        bool selected = false;
    };

    struct Perspective2DPaletteSnapshot {
        std::string selected_option_id;
        std::string selected_tileset_id;
        std::string selected_tile_id;
        std::string brush_shape = "rectangle";
        int brush_size = 1;
        bool has_selected_tile = false;
        size_t tile_option_count = 0;
        std::vector<Perspective2DPaletteOptionSnapshot> tile_options;
    };

    struct Perspective2DEventSnapshot {
        struct CommandSnapshot {
            std::string code;
            std::string argument;
        };

        std::string event_id;
        std::string label;
        std::string trigger_id;
        std::string layer_id;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
        bool visible_in_playtest = true;
        size_t command_count = 0;
        std::vector<CommandSnapshot> commands;
    };

    struct Perspective2DProjectSnapshot {
        std::string map_id;
        uint32_t width = 0;
        uint32_t height = 0;
        std::string selected_layer_id;
        size_t layer_count = 0;
        size_t painted_tile_count = 0;
        size_t event_count = 0;
        bool has_unsaved_changes = false;
        bool can_save = false;
        bool can_load = true;
        bool can_playtest = false;
        bool can_export = false;
        std::vector<std::string> diagnostics;
    };

    struct Perspective2DDraftResult {
        bool success = false;
        std::string command_id;
        std::string message;
        std::string map_id;
        std::string serialized_document_json;
        size_t layer_count = 0;
        size_t painted_tile_count = 0;
        size_t event_count = 0;
        std::vector<std::string> blocker_codes;
    };

    struct Perspective2DPaletteOption {
        std::string option_id;
        std::string label;
        std::string tileset_id;
        std::string tile_id;
        std::string asset_id;
        std::string project_path;
    };

    struct Perspective2DPlaytestResult {
        bool success = false;
        std::string command_id = "playtest_perspective_2d_map";
        std::string message = "Perspective 2D map playtest has not run.";
        std::string map_id;
        size_t playtest_tile_count = 0;
        size_t playtest_event_count = 0;
        std::vector<std::string> blocker_codes;
    };

    struct Perspective2DExportResult {
        bool success = false;
        std::string command_id = "export_perspective_2d_map";
        std::string message = "Perspective 2D map export has not run.";
        std::string map_id;
        std::string serialized_document_json;
        size_t exported_tile_count = 0;
        size_t exported_event_count = 0;
        std::vector<std::string> blocker_codes;
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
        Perspective2DProjectSnapshot perspective_2d_project;
        Perspective2DPaletteSnapshot perspective_2d_palette;
        std::vector<Perspective2DLayerSnapshot> perspective_2d_layers;
        std::vector<Perspective2DEventSnapshot> perspective_2d_events;
        Perspective2DDraftResult last_perspective_2d_save;
        Perspective2DDraftResult last_perspective_2d_load;
        Perspective2DPlaytestResult last_perspective_2d_playtest;
        Perspective2DExportResult last_perspective_2d_export;
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
    bool AddPerspectiveLayer(const std::string& layer_id, const std::string& label, const std::string& kind);
    bool SelectPerspectiveLayer(const std::string& layer_id);
    bool SetPerspectiveLayerVisible(const std::string& layer_id, bool visible);
    bool SetPerspectiveLayerLocked(const std::string& layer_id, bool locked);
    bool MovePerspectiveLayer(const std::string& layer_id, int new_order);
    bool DeletePerspectiveLayer(const std::string& layer_id);
    bool DuplicatePerspectiveLayer(const std::string& source_layer_id,
                                   const std::string& new_layer_id,
                                   const std::string& new_label);
    bool ClearPerspectiveLayer(const std::string& layer_id);
    void SetPerspectiveTilePaletteOptions(std::vector<Perspective2DPaletteOption> options);
    bool SelectPerspectiveTilePaletteOption(const std::string& option_id);
    bool SelectPerspectiveTile(const std::string& tileset_id, const std::string& tile_id);
    bool SetPerspectiveBrushSize(int brush_size);
    bool PaintPerspectiveTileFromScreen(float screen_x, float screen_y);
    bool ErasePerspectiveTileFromScreen(float screen_x, float screen_y);
    bool AddPerspectiveEventFromScreen(const std::string& event_id,
                                       const std::string& label,
                                       const std::string& trigger_id,
                                       float screen_x,
                                       float screen_y);
    bool MovePerspectiveEventFromScreen(const std::string& event_id, float screen_x, float screen_y);
    bool AddPerspectiveEventCommand(const std::string& event_id,
                                    const std::string& command_code,
                                    const std::string& argument);
    bool UpdatePerspectiveEventCommand(const std::string& event_id,
                                       size_t command_index,
                                       const std::string& command_code,
                                       const std::string& argument);
    bool RemovePerspectiveEventCommand(const std::string& event_id, size_t command_index);
    Perspective2DDraftResult SavePerspectiveMapDraft();
    Perspective2DDraftResult LoadPerspectiveMapDraft(const std::string& serialized_document_json);
    Perspective2DPlaytestResult RunPerspectiveMapPlaytest();
    Perspective2DExportResult ExportPerspectiveMap();
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
    void markPerspectiveDirty();
    bool projectScreenToTile(float screen_x, float screen_y, int32_t& out_tile_x, int32_t& out_tile_y) const;
    std::vector<std::string> validatePerspectiveMapForPlaytest() const;
    std::string serializePerspectiveMapDraft() const;
    static const char* modeName(ToolMode mode);

    struct PerspectiveLayer {
        std::string id;
        std::string label;
        std::string kind;
        bool visible = true;
        bool locked = false;
        int order = 0;
    };

    struct PerspectiveTilePaint {
        std::string layer_id;
        std::string tileset_id;
        std::string tile_id;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
    };

    struct PerspectiveEvent {
        struct Command {
            std::string code;
            std::string argument;
        };

        std::string event_id;
        std::string label;
        std::string trigger_id;
        std::string layer_id;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
        std::vector<Command> commands;
    };

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
    std::vector<PerspectiveLayer> perspective_layers_;
    std::vector<PerspectiveTilePaint> perspective_tiles_;
    std::vector<PerspectiveEvent> perspective_events_;
    std::vector<Perspective2DPaletteOption> perspective_tile_palette_options_;
    std::string selected_perspective_layer_id_;
    std::string selected_palette_option_id_;
    std::string selected_tileset_id_;
    std::string selected_tile_id_;
    int perspective_brush_size_ = 1;
    bool perspective_has_unsaved_changes_ = false;
    bool perspective_playtest_ready_ = false;
    Perspective2DDraftResult last_perspective_save_result_;
    Perspective2DDraftResult last_perspective_load_result_;
    Perspective2DPlaytestResult last_perspective_playtest_result_;
    Perspective2DExportResult last_perspective_export_result_;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
