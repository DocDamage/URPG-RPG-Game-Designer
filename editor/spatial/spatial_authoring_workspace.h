#pragma once

#include "editor/spatial/elevation_brush_panel.h"
#include "editor/spatial/grid_part_inspector_panel.h"
#include "editor/spatial/grid_part_palette_panel.h"
#include "editor/spatial/grid_part_placement_panel.h"
#include "editor/spatial/map_environment_preview_panel.h"
#include "editor/spatial/map_ability_binding_panel.h"
#include "editor/spatial/perspective_2d_product_workflow.h"
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
        Events = 7,
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
        bool selected_for_bulk_edit = false;
    };

    struct Perspective2DPaletteOptionSnapshot {
        std::string option_id;
        std::string label;
        std::string tileset_id;
        std::string tile_id;
        std::string asset_id;
        std::string project_path;
        std::string category_id = "";
        std::string thumbnail_path = "";
        bool selected = false;
    };

    struct Perspective2DPaletteSnapshot {
        std::string selected_option_id;
        std::string selected_tileset_id;
        std::string selected_tile_id;
        std::string search_text;
        std::string filtered_tileset_id;
        std::string filtered_category_id;
        std::string brush_shape = "rectangle";
        int brush_size = 1;
        bool has_selected_tile = false;
        bool selected_option_visible = false;
        size_t tile_option_count = 0;
        size_t visible_tile_option_count = 0;
        std::vector<Perspective2DPaletteOptionSnapshot> tile_options;
    };

    struct Perspective2DEventSnapshot {
        struct CommandSnapshot {
            std::string code;
            std::string argument;
            std::string condition_type;
            std::string condition_key;
            std::string condition_comparison;
            std::string condition_value;
            size_t true_command_count = 0;
            size_t false_command_count = 0;
            std::vector<CommandSnapshot> true_commands;
            std::vector<CommandSnapshot> false_commands;
        };

        struct ConditionSnapshot {
            std::string type;
            std::string key;
            std::string comparison = "equals";
            std::string value;
        };

        struct PageSnapshot {
            std::string page_id;
            std::string label;
            std::string trigger_id;
            int order = 0;
            bool selected = false;
            bool active_in_playtest = false;
            size_t condition_count = 0;
            size_t command_count = 0;
            std::vector<ConditionSnapshot> conditions;
            std::vector<CommandSnapshot> commands;
        };

        std::string event_id;
        std::string label;
        std::string trigger_id;
        std::string layer_id;
        // Authoring metadata for an attached image placed as an event. This is
        // intentionally not a runtime sprite/rendering contract.
        std::string asset_id;
        std::string asset_project_path;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
        bool visible_in_playtest = true;
        size_t command_count = 0;
        size_t page_count = 0;
        std::string selected_page_id;
        std::string active_page_id;
        std::vector<CommandSnapshot> commands;
        std::vector<PageSnapshot> pages;
    };

    struct Perspective2DProjectSnapshot {
        std::string map_id;
        uint32_t width = 0;
        uint32_t height = 0;
        std::string selected_layer_id;
        size_t selected_layer_count = 0;
        size_t layer_count = 0;
        size_t painted_tile_count = 0;
        size_t event_count = 0;
        bool has_unsaved_changes = false;
        bool can_save = false;
        bool can_load = true;
        bool can_playtest = false;
        bool can_export = false;
        bool layer_workflow_ready = false;
        bool palette_workflow_ready = false;
        bool event_workflow_ready = false;
        bool playtest_workflow_ready = false;
        bool export_workflow_ready = false;
        bool release_asset_gate_ready = false;
        bool creator_workflow_ready = false;
        std::string creator_next_step;
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

    // Result for one owner-scoped replacement of attached visual asset
    // references in the active Perspective 2D map. This is intentionally not
    // a project-wide replacement or deletion operation.
    struct AttachedAssetReplacementResult {
        bool success = false;
        std::string code;
        std::string message;
        size_t tile_palette_count = 0;
        size_t painted_tile_count = 0;
        size_t prop_palette_count = 0;
        size_t prop_instance_count = 0;
        size_t event_metadata_count = 0;
    };

    // A narrow native document command used by reviewed creator-intent
    // adapters. The adapter must resolve every planned tile to an existing
    // authoring palette entry before this owner accepts the command.
    struct Perspective2DNativeTileEdit {
        std::string layer_id;
        std::string tileset_id;
        std::string tile_id;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
    };

    // A reviewed creator-prop command can only reference an existing native
    // Perspective 2D prop-palette asset. The owner derives the stable instance
    // ID and records all edits as one local history entry.
    struct Perspective2DNativePropEdit {
        std::string operation_id;
        std::string asset_id;
        std::string project_path;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
    };

    // A reviewed creator event message has one fixed native behavior: a
    // visible, unlocked event/object layer receives an event with one
    // confirm-interact show-text page. Other event logic remains separate.
    struct Perspective2DNativeEventMessageEdit {
        std::string operation_id;
        std::string layer_id;
        std::string label;
        std::string message;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
    };

    struct Perspective2DNativeCommandResult {
        bool success = false;
        std::string code;
        std::string message;
        std::string document_revision;
        size_t applied_tile_count = 0;
        size_t applied_prop_count = 0;
        size_t applied_event_count = 0;
    };

    struct Perspective2DPaletteOption {
        std::string option_id;
        std::string label;
        std::string tileset_id;
        std::string tile_id;
        std::string asset_id;
        std::string project_path;
        std::string category_id = "";
        std::string thumbnail_path = "";
    };

    struct Perspective2DTilesetPage {
        std::string page_id;
        std::string label;
        std::string asset_id;
        std::string project_path;
        int columns = 0;
        int rows = 0;
        int tile_width = 48;
        int tile_height = 48;
    };

    struct Perspective2DTileDefinition {
        std::string tileset_id;
        std::string tile_id;
        std::string page_id;
        bool autotile = false;
        std::string autotile_kind;
        bool animated = false;
        std::vector<std::string> animation_frame_tile_ids;
        int animation_frame_ms = 0;
        bool passable_down = true;
        bool passable_left = true;
        bool passable_right = true;
        bool passable_up = true;
        bool collision = false;
        int terrain_tag = 0;
        int region_id = 0;
        int priority = 0;
        bool star_passability = false;
        std::string preview_path;
    };

    struct Perspective2DTilePreviewResult {
        bool success = false;
        std::string command_id = "preview_perspective_2d_tile";
        std::string message = "Perspective 2D tile preview has not run.";
        std::string layer_id;
        std::string tileset_id;
        std::string tile_id;
        std::string page_id;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
        bool autotile = false;
        std::string autotile_kind;
        bool animated = false;
        std::vector<std::string> animation_frame_tile_ids;
        int animation_frame_ms = 0;
        bool passable_down = true;
        bool passable_left = true;
        bool passable_right = true;
        bool passable_up = true;
        bool collision = false;
        int terrain_tag = 0;
        int region_id = 0;
        int priority = 0;
        bool star_passability = false;
        std::string preview_path;
        std::vector<std::string> blocker_codes;
    };

    struct Perspective2DTileSystemSnapshot {
        size_t tile_page_count = 0;
        size_t tile_definition_count = 0;
        size_t autotile_count = 0;
        size_t animated_tile_count = 0;
        size_t collision_tile_count = 0;
        size_t star_passability_tile_count = 0;
        Perspective2DTilePreviewResult latest_preview;
    };

    struct Perspective2DProjectReference {
        std::string kind;
        std::string id;
        std::string label;
        std::string target_map_id;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
    };

    struct Perspective2DProjectIntegrationResult {
        bool success = false;
        std::string command_id = "validate_perspective_2d_project_integration";
        std::string message = "Perspective 2D project integration has not run.";
        size_t actor_count = 0;
        size_t item_count = 0;
        size_t switch_count = 0;
        size_t variable_count = 0;
        size_t common_event_count = 0;
        size_t map_count = 0;
        size_t transfer_count = 0;
        size_t encounter_count = 0;
        size_t asset_count = 0;
        size_t starting_party_count = 0;
        bool save_load_enabled = false;
        std::string save_profile_id;
        std::vector<std::string> diagnostics;
    };

    struct Perspective2DProjectDatabaseSnapshot : Perspective2DProjectIntegrationResult {
        bool ready = false;
    };

    struct Perspective2DPlaytestResult {
        bool success = false;
        std::string command_id = "playtest_perspective_2d_map";
        std::string message = "Perspective 2D map playtest has not run.";
        std::string map_id;
        size_t playtest_tile_count = 0;
        size_t playtest_event_count = 0;
        std::string serialized_runtime_manifest_json;
        std::string serialized_event_execution_traces_json;
        size_t runtime_layer_count = 0;
        size_t runtime_tile_count = 0;
        size_t runtime_event_count = 0;
        size_t runtime_event_execution_trace_count = 0;
        std::vector<std::string> blocker_codes;
    };

    struct Perspective2DPackageFile {
        std::string path;
        std::string kind;
        size_t byte_count = 0;
        std::string content_hash;
    };

    struct Perspective2DExportResult {
        bool success = false;
        std::string command_id = "export_perspective_2d_map";
        std::string message = "Perspective 2D map export has not run.";
        std::string map_id;
        std::string serialized_document_json;
        std::string serialized_runtime_manifest_json;
        std::string serialized_event_execution_traces_json;
        std::string serialized_package_manifest_json;
        std::string package_signature;
        size_t exported_tile_count = 0;
        size_t exported_event_count = 0;
        size_t runtime_layer_count = 0;
        size_t runtime_tile_count = 0;
        size_t runtime_event_count = 0;
        size_t runtime_event_execution_trace_count = 0;
        std::vector<Perspective2DPackageFile> package_files;
        std::vector<std::string> blocker_codes;
    };

    struct Perspective2DEventExecutionStep {
        std::string code;
        std::string argument;
        std::string branch_path = "root";
        std::string condition_type;
        std::string condition_key;
        std::string condition_comparison;
        std::string condition_value;
        bool condition_matched = true;
    };

    struct Perspective2DEventExecutionResult {
        bool success = false;
        std::string command_id = "preview_perspective_2d_event_execution";
        std::string message = "Perspective 2D event execution preview has not run.";
        std::string map_id;
        std::string event_id;
        std::string active_page_id;
        std::string trigger_id;
        size_t executed_command_count = 0;
        std::vector<Perspective2DEventExecutionStep> executed_commands;
        std::string serialized_execution_trace_json;
        std::vector<std::string> blocker_codes;
    };

    struct Perspective2DStateEntry {
        std::string key;
        std::string value;
    };

    struct Perspective2DRuntimeResult {
        bool success = false;
        std::string command_id = "execute_perspective_2d_runtime_event";
        std::string message = "Perspective 2D runtime event has not run.";
        std::string map_id;
        std::string event_id;
        std::string active_page_id;
        std::string trigger_id;
        size_t executed_command_count = 0;
        std::vector<Perspective2DEventExecutionStep> executed_commands;
        std::vector<std::string> messages;
        std::vector<std::string> dialogue_choices;
        std::vector<Perspective2DStateEntry> switches;
        std::vector<Perspective2DStateEntry> variables;
        std::vector<Perspective2DStateEntry> self_switches;
        std::vector<Perspective2DStateEntry> inventory;
        std::vector<std::string> movement_route_steps;
        std::vector<std::string> common_events;
        std::vector<std::string> battles;
        std::vector<std::string> vendors;
        int gold = 0;
        std::string player_map_id;
        int32_t player_tile_x = 0;
        int32_t player_tile_y = 0;
        std::string serialized_runtime_state_json;
        std::vector<std::string> blocker_codes;
    };

    struct Perspective2DReleaseAssetGateResult {
        bool success = false;
        std::string command_id = "gate_perspective_2d_release_assets";
        std::string message = "Perspective 2D release asset gate has not run.";
        std::string policy_state = "not_run";
        size_t release_required_asset_count = 0;
        size_t verified_release_required_asset_count = 0;
        size_t optional_lfs_asset_count = 0;
        size_t optional_lfs_deferred_count = 0;
        std::vector<std::string> blocker_codes;
    };

    struct Perspective2DModelUiSnapshot {
        bool map_canvas_visible = false;
        bool layer_panel_visible = false;
        bool tile_palette_visible = false;
        bool event_page_tabs_visible = false;
        bool condition_editor_visible = false;
        bool command_list_visible = false;
        bool branch_tree_visible = false;
        bool command_picker_visible = false;
        bool playtest_controls_visible = false;
        bool export_controls_visible = false;
        std::string selected_event_id;
        std::string selected_page_id;
        size_t visible_layer_count = 0;
        size_t visible_event_count = 0;
        size_t page_tab_count = 0;
        size_t command_row_count = 0;
        std::vector<std::string> command_picker_options;
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
        Perspective2DTileSystemSnapshot perspective_2d_tiles;
        Perspective2DProjectDatabaseSnapshot perspective_2d_project_database;
        std::vector<Perspective2DLayerSnapshot> perspective_2d_layers;
        std::vector<Perspective2DEventSnapshot> perspective_2d_events;
        Perspective2DDraftResult last_perspective_2d_save;
        Perspective2DDraftResult last_perspective_2d_load;
        Perspective2DPlaytestResult last_perspective_2d_playtest;
        Perspective2DExportResult last_perspective_2d_export;
        Perspective2DEventExecutionResult last_perspective_2d_event_execution;
        Perspective2DRuntimeResult last_perspective_2d_runtime;
        Perspective2DReleaseAssetGateResult last_perspective_2d_release_asset_gate;
        Perspective2DModelUiSnapshot perspective_2d_ui;
        Perspective2DProductWorkflowReport perspective_2d_product;
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
    bool SelectPerspectiveLayers(const std::vector<std::string>& layer_ids);
    bool SetSelectedPerspectiveLayersVisible(bool visible);
    bool SetSelectedPerspectiveLayersLocked(bool locked);
    bool DuplicateSelectedPerspectiveLayers(const std::string& id_suffix);
    bool DeleteSelectedPerspectiveLayers();
    bool SetPerspectiveLayerVisible(const std::string& layer_id, bool visible);
    bool SetPerspectiveLayerLocked(const std::string& layer_id, bool locked);
    bool MovePerspectiveLayer(const std::string& layer_id, int new_order);
    bool DeletePerspectiveLayer(const std::string& layer_id);
    bool DuplicatePerspectiveLayer(const std::string& source_layer_id,
                                   const std::string& new_layer_id,
                                   const std::string& new_label);
    bool ClearPerspectiveLayer(const std::string& layer_id);
    void SetPerspectiveTilePaletteOptions(std::vector<Perspective2DPaletteOption> options);
    bool AddAttachedAssetToTilePalette(const std::string& asset_id, const std::string& project_path);
    bool AddAttachedAssetToPropPalette(const std::string& asset_id, const std::string& project_path);
    // Atomically registers an attached tile asset and paints it at the canvas
    // drop coordinate. One Perspective 2D undo reverses both changes.
    bool PlaceAttachedAssetTileFromScreen(const std::string& asset_id,
                                          const std::string& project_path,
                                          float screen_x,
                                          float screen_y);
    // Atomically registers an attached prop asset and places its stable asset
    // reference at the canvas drop coordinate. One Perspective 2D undo
    // reverses both the palette binding and the prop instance.
    bool PlaceAttachedAssetPropFromScreen(const std::string& asset_id,
                                          const std::string& project_path,
                                          float screen_x,
                                          float screen_y);
    // Creates one authored event from a governed attached image drop. The
    // asset reference is retained in the Perspective 2D document/history and
    // projected to the bound MapScene as a runtime sprite when its layer is visible.
    bool PlaceAttachedAssetEventFromScreen(const std::string& asset_id,
                                           const std::string& project_path,
                                           float screen_x,
                                           float screen_y);
    AttachedAssetReplacementResult replaceAttachedAssetReferences(const std::string& source_asset_id,
                                                                   const std::string& replacement_asset_id,
                                                                   const std::string& replacement_project_path);
    // Applies resolved tile edits to the live Perspective 2D document as one
    // undoable command. The caller must supply the revision it reviewed.
    std::string activePerspectiveMapId() const;
    std::string perspectiveDocumentRevision() const;
    Perspective2DNativeCommandResult applyNativeTileEdits(
        const std::string& expected_document_revision,
        const std::vector<Perspective2DNativeTileEdit>& edits);
    Perspective2DNativeCommandResult applyNativePropEdits(
        const std::string& expected_document_revision,
        const std::vector<Perspective2DNativePropEdit>& edits);
    Perspective2DNativeCommandResult applyNativeEventMessageEdits(
        const std::string& expected_document_revision,
        const std::vector<Perspective2DNativeEventMessageEdit>& edits);
    bool SetPerspectiveTilesetPages(std::vector<Perspective2DTilesetPage> pages);
    bool SetPerspectiveTileDefinition(Perspective2DTileDefinition definition);
    Perspective2DTilePreviewResult PreviewPerspectiveTileAt(int32_t tile_x, int32_t tile_y);
    void SetPerspectiveTilePaletteFilter(const std::string& search_text,
                                         const std::string& tileset_id,
                                         const std::string& category_id);
    void ClearPerspectiveTilePaletteFilter();
    bool SelectPerspectiveTilePaletteOption(const std::string& option_id);
    bool SelectPerspectiveTile(const std::string& tileset_id, const std::string& tile_id);
    bool SetPerspectiveBrushSize(int brush_size);
    bool PaintPerspectiveTileFromScreen(float screen_x, float screen_y);
    bool ErasePerspectiveTileFromScreen(float screen_x, float screen_y);
    bool UndoPerspective2D();
    bool RedoPerspective2D();
    bool canUndoPerspective2D() const { return !perspective_undo_drafts_.empty(); }
    bool canRedoPerspective2D() const { return !perspective_redo_drafts_.empty(); }
    bool AddPerspectiveEventFromScreen(const std::string& event_id,
                                       const std::string& label,
                                       const std::string& trigger_id,
                                       float screen_x,
                                       float screen_y);
    bool MovePerspectiveEventFromScreen(const std::string& event_id, float screen_x, float screen_y);
    bool AddPerspectiveEventCommand(const std::string& event_id,
                                    const std::string& command_code,
                                    const std::string& argument);
    bool AddPerspectiveEventPage(const std::string& event_id,
                                 const std::string& page_id,
                                 const std::string& label,
                                 const std::string& trigger_id);
    bool SelectPerspectiveEventPage(const std::string& event_id, const std::string& page_id);
    bool MovePerspectiveEventPage(const std::string& event_id, const std::string& page_id, int new_order);
    bool DuplicatePerspectiveEventPage(const std::string& event_id,
                                       const std::string& source_page_id,
                                       const std::string& new_page_id,
                                       const std::string& new_label);
    bool DeletePerspectiveEventPage(const std::string& event_id, const std::string& page_id);
    bool AddPerspectiveEventPageCondition(const std::string& event_id,
                                          const std::string& page_id,
                                          const std::string& condition_type,
                                          const std::string& key,
                                          const std::string& value);
    bool AddPerspectiveEventPageConditionRule(const std::string& event_id,
                                              const std::string& page_id,
                                              const std::string& condition_type,
                                              const std::string& key,
                                              const std::string& comparison,
                                              const std::string& value);
    bool UpdatePerspectiveEventPageCondition(const std::string& event_id,
                                             const std::string& page_id,
                                             size_t condition_index,
                                             const std::string& condition_type,
                                             const std::string& key,
                                             const std::string& value);
    bool RemovePerspectiveEventPageCondition(const std::string& event_id,
                                             const std::string& page_id,
                                             size_t condition_index);
    bool AddPerspectiveEventPageCommand(const std::string& event_id,
                                        const std::string& page_id,
                                        const std::string& command_code,
                                        const std::string& argument);
    bool AddPerspectiveEventPageConditionalBranch(const std::string& event_id,
                                                  const std::string& page_id,
                                                  const std::string& condition_type,
                                                  const std::string& key,
                                                  const std::string& comparison,
                                                  const std::string& value);
    bool AddPerspectiveEventPageBranchCommand(const std::string& event_id,
                                              const std::string& page_id,
                                              size_t branch_command_index,
                                              bool when_true,
                                              const std::string& command_code,
                                              const std::string& argument);
    bool UpdatePerspectiveEventPageCommand(const std::string& event_id,
                                           const std::string& page_id,
                                           size_t command_index,
                                           const std::string& command_code,
                                           const std::string& argument);
    bool RemovePerspectiveEventPageCommand(const std::string& event_id,
                                           const std::string& page_id,
                                           size_t command_index);
    bool SetPerspectiveEventConditionValue(const std::string& condition_type,
                                           const std::string& key,
                                           const std::string& value);
    bool UpdatePerspectiveEventCommand(const std::string& event_id,
                                       size_t command_index,
                                       const std::string& command_code,
                                       const std::string& argument);
    bool RemovePerspectiveEventCommand(const std::string& event_id, size_t command_index);
    // Produces a validated draft without clearing dirty state. The caller must
    // publish it, potentially alongside other map documents, before commit.
    Perspective2DDraftResult PreparePerspectiveMapDraftSave();
    void MarkPerspectiveMapDraftPersisted();
    Perspective2DDraftResult SavePerspectiveMapDraft();
    Perspective2DDraftResult LoadPerspectiveMapDraft(const std::string& serialized_document_json);
    Perspective2DPlaytestResult RunPerspectiveMapPlaytest();
    Perspective2DExportResult ExportPerspectiveMap();
    Perspective2DEventExecutionResult PreviewPerspectiveEventExecution(const std::string& event_id);
    Perspective2DRuntimeResult ExecutePerspectiveRuntimeEvent(const std::string& event_id);
    Perspective2DRuntimeResult RestorePerspectiveRuntimeState(const std::string& serialized_runtime_state_json);
    bool SetPerspectiveProjectDatabaseReferences(std::vector<Perspective2DProjectReference> references);
    bool SetPerspectiveProjectStartingParty(std::vector<std::string> actor_ids);
    bool SetPerspectiveProjectSaveLoadState(bool enabled, const std::string& save_profile_id);
    Perspective2DProjectIntegrationResult ValidatePerspectiveProjectIntegration();
    Perspective2DReleaseAssetGateResult RecordPerspectiveReleaseAssetGate(
        size_t release_required_asset_count,
        size_t verified_release_required_asset_count,
        size_t optional_lfs_asset_count,
        size_t optional_lfs_deferred_count);
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
    void syncEventSpritesToTargetScene();
    void syncAuthoredDialogueInteractionsToTargetScene();
    void syncAuthoredDialogueRuntimeStateFromTargetScene();
    void syncPanelVisibility();
    void markPerspectiveDirty();
    bool projectScreenToTile(float screen_x, float screen_y, int32_t& out_tile_x, int32_t& out_tile_y) const;
    std::vector<std::string> validatePerspectiveMapForPlaytest() const;
    std::string serializePerspectiveMapDraft() const;
    std::string serializePerspectiveRuntimeManifest(size_t& out_layer_count,
                                                    size_t& out_tile_count,
                                                    size_t& out_event_count) const;
    std::string serializePerspectiveExportPackageManifest(const Perspective2DExportResult& export_result) const;
    std::string serializePerspectiveEventExecutionTraceBundle(size_t& out_trace_count) const;
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
            std::string condition_type = "";
            std::string condition_key = "";
            std::string condition_comparison = "";
            std::string condition_value = "";
            std::vector<Command> true_commands = {};
            std::vector<Command> false_commands = {};
        };

        struct Condition {
            std::string type;
            std::string key;
            std::string comparison = "equals";
            std::string value;
        };

        struct Page {
            std::string page_id;
            std::string label;
            std::string trigger_id;
            int order = 0;
            std::vector<Condition> conditions;
            std::vector<Command> commands;
        };

        std::string event_id;
        std::string label;
        std::string trigger_id;
        std::string layer_id;
        std::string asset_id;
        std::string asset_project_path;
        int32_t tile_x = 0;
        int32_t tile_y = 0;
        std::string selected_page_id;
        std::vector<Command> commands;
        std::vector<Page> pages;
    };

    struct PerspectiveEventConditionValue {
        std::string type;
        std::string key;
        std::string value;
    };

    Perspective2DEventExecutionResult buildPerspectiveEventExecutionTrace(const PerspectiveEvent& event) const;

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
    std::vector<PerspectiveEventConditionValue> perspective_event_condition_values_;
    std::vector<Perspective2DPaletteOption> perspective_tile_palette_options_;
    std::vector<Perspective2DTilesetPage> perspective_tileset_pages_;
    std::vector<Perspective2DTileDefinition> perspective_tile_definitions_;
    std::vector<Perspective2DProjectReference> perspective_project_references_;
    std::vector<std::string> perspective_starting_party_;
    bool perspective_save_load_enabled_ = false;
    std::string perspective_save_profile_id_;
    std::string selected_perspective_layer_id_;
    std::vector<std::string> selected_perspective_layer_ids_;
    std::string selected_palette_option_id_;
    std::string palette_search_text_;
    std::string palette_filter_tileset_id_;
    std::string palette_filter_category_id_;
    std::string selected_tileset_id_;
    std::string selected_tile_id_;
    int perspective_brush_size_ = 1;
    bool perspective_has_unsaved_changes_ = false;
    bool perspective_playtest_ready_ = false;
    std::string perspective_history_checkpoint_;
    std::vector<std::string> perspective_undo_drafts_;
    std::vector<std::string> perspective_redo_drafts_;
    bool restoring_perspective_history_ = false;
    Perspective2DDraftResult last_perspective_save_result_;
    Perspective2DDraftResult last_perspective_load_result_;
    Perspective2DPlaytestResult last_perspective_playtest_result_;
    Perspective2DExportResult last_perspective_export_result_;
    Perspective2DEventExecutionResult last_perspective_event_execution_result_;
    Perspective2DRuntimeResult last_perspective_runtime_result_;
    Perspective2DTilePreviewResult last_perspective_tile_preview_result_;
    Perspective2DProjectIntegrationResult last_perspective_project_integration_result_;
    Perspective2DReleaseAssetGateResult last_perspective_release_asset_gate_result_;
    std::vector<Perspective2DStateEntry> perspective_runtime_switches_;
    std::vector<Perspective2DStateEntry> perspective_runtime_variables_;
    std::vector<Perspective2DStateEntry> perspective_runtime_self_switches_;
    uint64_t last_synced_authored_dialogue_state_revision_ = 0;
    std::vector<Perspective2DStateEntry> perspective_runtime_inventory_;
    std::vector<std::string> perspective_runtime_dialogue_choices_;
    int perspective_runtime_gold_ = 0;
    std::string perspective_runtime_player_map_id_;
    int32_t perspective_runtime_player_tile_x_ = 0;
    int32_t perspective_runtime_player_tile_y_ = 0;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
