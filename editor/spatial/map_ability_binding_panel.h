#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/presentation/presentation_schema.h"
#include "editor/spatial/prop_placement_panel.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace urpg::scene {
    class MapScene;
}

namespace urpg::editor {

class MapAbilityBindingPanel : public EditorPanel {
public:
    struct PaintedRegion {
        int min_x = 0;
        int min_y = 0;
        int max_x = 0;
        int max_y = 0;
        bool selected = false;
    };

    struct PlacementTarget {
        int tile_x = 0;
        int tile_y = 0;
        std::string prop_asset_id;
        int region_start_x = 0;
        int region_start_y = 0;
        int region_end_x = 0;
        int region_end_y = 0;
        size_t selected_prop_index = static_cast<size_t>(-1);
        std::vector<PaintedRegion> painted_regions;
    };

    struct AssetEntry {
        std::string relative_path;
        std::string ability_id;
        bool selected = false;
    };

    struct BindingEntry {
        std::string scope;
        std::string trigger_id;
        std::string asset_path;
        std::string ability_id;
        int tile_x = -1;
        int tile_y = -1;
        int region_min_x = -1;
        int region_min_y = -1;
        int region_max_x = -1;
        int region_max_y = -1;
        std::string prop_asset_id;
    };

    struct PropHandleEntry {
        size_t prop_index = 0;
        std::string asset_id;
        float world_x = 0.0f;
        float world_y = 0.0f;
        float world_z = 0.0f;
        int tile_x = 0;
        int tile_y = 0;
        bool selected = false;
        bool has_binding = false;
    };

    struct TileOverlayEntry {
        int tile_x = 0;
        int tile_y = 0;
        std::string trigger_id;
        std::string ability_id;
        std::string source;
        bool selected = false;
        bool pending = false;
    };

    struct RegionOverlayEntry {
        int min_x = 0;
        int min_y = 0;
        int max_x = 0;
        int max_y = 0;
        std::string trigger_id;
        std::string ability_id;
        std::string source;
        bool selected = false;
        bool pending = false;
    };

    struct RenderSnapshot {
        bool visible = true;
        bool has_target_scene = false;
        bool has_target_overlay = false;
        std::string project_root;
        std::string canonical_directory;
        size_t available_asset_count = 0;
        std::string selected_asset_path;
        std::string selected_ability_id;
        std::string selected_trigger_id;
        PlacementTarget placement;
        size_t binding_count = 0;
        size_t runtime_ability_count = 0;
        size_t prop_handle_count = 0;
        size_t tile_overlay_count = 0;
        size_t region_overlay_count = 0;
        std::string latest_ability_id;
        std::string latest_outcome;
        std::vector<AssetEntry> assets;
        std::vector<BindingEntry> bindings;
        std::vector<PropHandleEntry> prop_handles;
        std::vector<TileOverlayEntry> tile_overlays;
        std::vector<RegionOverlayEntry> region_overlays;
    };

    MapAbilityBindingPanel() : EditorPanel("Map Ability Binding") {}

    void Render(const urpg::FrameContext& context) override;
    void SetTarget(urpg::scene::MapScene* scene);
    void SetSpatialTarget(urpg::presentation::SpatialMapOverlay* overlay);
    bool SetProjectRoot(const std::string& root_path);
    bool RefreshProjectAssets();
    bool SelectAsset(size_t index);
    bool SelectAssetByAbilityId(const std::string& ability_id);
    bool SetSelectedTriggerId(const std::string& trigger_id);
    bool SetPlacementTile(int tile_x, int tile_y);
    bool SetSelectedPropAssetId(const std::string& prop_asset_id);
    bool SetActiveRegionBounds(int min_x, int min_y, int max_x, int max_y);
    bool SelectPropHandle(size_t index);
    bool BindSelectedAbilityToTileFromScreen(float screenX,
                                             float screenY,
                                             const PropPlacementPanel::ScreenProjectionSettings& settings);
    bool SelectPropFromScreen(float screenX,
                              float screenY,
                              const PropPlacementPanel::ScreenProjectionSettings& settings,
                              float max_distance = 0.75f);
    bool BeginPaintRegionFromScreen(float screenX,
                                    float screenY,
                                    const PropPlacementPanel::ScreenProjectionSettings& settings);
    bool UpdatePaintRegionFromScreen(float screenX,
                                     float screenY,
                                     const PropPlacementPanel::ScreenProjectionSettings& settings);
    bool CommitPaintedRegion();
    bool SelectPaintedRegion(size_t index);
    bool ClearPaintedRegions();
    bool BindSelectedAbilityToConfirmInteraction();
    bool BindSelectedAbilityToPlacementTile();
    bool BindSelectedAbilityToSelectedProp();
    bool BindSelectedAbilityToPaintedRegion();
    bool BindSelectedAbilityToCommittedRegions();
    bool MoveTileBinding(int from_tile_x, int from_tile_y, int to_tile_x, int to_tile_y);
    bool MovePropBinding(const std::string& from_prop_asset_id, const std::string& to_prop_asset_id);
    bool ResizeRegionBinding(int from_min_x,
                             int from_min_y,
                             int from_max_x,
                             int from_max_y,
                             int to_min_x,
                             int to_min_y,
                             int to_max_x,
                             int to_max_y);
    bool SwitchTileBindingTrigger(int tile_x, int tile_y, const std::string& from_trigger_id, const std::string& to_trigger_id);
    bool SwitchPropBindingTrigger(const std::string& prop_asset_id,
                                  const std::string& from_trigger_id,
                                  const std::string& to_trigger_id);
    bool SwitchRegionBindingTrigger(int min_x,
                                    int min_y,
                                    int max_x,
                                    int max_y,
                                    const std::string& from_trigger_id,
                                    const std::string& to_trigger_id);
    bool ReplaceTileBindingAsset(int tile_x,
                                 int tile_y,
                                 const std::string& trigger_id,
                                 const std::string& asset_path);
    bool ReplacePropBindingAsset(const std::string& prop_asset_id,
                                 const std::string& trigger_id,
                                 const std::string& asset_path);
    bool ReplaceRegionBindingAsset(int min_x,
                                   int min_y,
                                   int max_x,
                                   int max_y,
                                   const std::string& trigger_id,
                                   const std::string& asset_path);
    bool SwapTileBindingTriggers(int tile_x,
                                 int tile_y,
                                 const std::string& first_trigger_id,
                                 const std::string& second_trigger_id);
    bool SwapPropBindingTriggers(const std::string& prop_asset_id,
                                 const std::string& first_trigger_id,
                                 const std::string& second_trigger_id);
    bool SwapRegionBindingTriggers(int min_x,
                                   int min_y,
                                   int max_x,
                                   int max_y,
                                   const std::string& first_trigger_id,
                                   const std::string& second_trigger_id);
    bool SwapRegionBindingTriggersBetween(int first_min_x,
                                          int first_min_y,
                                          int first_max_x,
                                          int first_max_y,
                                          const std::string& first_trigger_id,
                                          int second_min_x,
                                          int second_min_y,
                                          int second_max_x,
                                          int second_max_y,
                                          const std::string& second_trigger_id);
    bool RemoveTileBinding(int tile_x, int tile_y, const std::string& trigger_id);
    bool RemovePropBinding(const std::string& prop_asset_id, const std::string& trigger_id);
    bool RemoveRegionBinding(int min_x, int min_y, int max_x, int max_y, const std::string& trigger_id);
    bool ActivateConfirmInteraction();
    bool ActivatePlacementTileInteraction();
    bool ActivateSelectedPropInteraction();

    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    void captureRenderSnapshot();

    urpg::scene::MapScene* m_target_scene = nullptr;
    urpg::presentation::SpatialMapOverlay* m_target_overlay = nullptr;
    std::filesystem::path m_project_root;
    std::vector<urpg::ability::AuthoredAbilityAssetRecord> m_assets;
    std::optional<size_t> m_selected_asset_index;
    std::string m_selected_trigger_id = "confirm_interact";
    std::optional<size_t> m_selected_prop_index;
    std::optional<size_t> m_selected_painted_region_index;
    PlacementTarget m_placement;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
