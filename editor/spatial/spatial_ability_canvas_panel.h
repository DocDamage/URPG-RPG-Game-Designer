#pragma once

#include "editor/spatial/map_ability_binding_panel.h"
#include "editor/spatial/prop_placement_panel.h"
#include "editor/ui/editor_panel.h"
#include "engine/core/presentation/presentation_schema.h"

#include <string>
#include <vector>

namespace urpg::editor {

class SpatialAbilityCanvasPanel : public EditorPanel {
public:
    enum class ConflictPolicy {
        ExclusiveTarget = 0,
        SharedTarget = 1,
        LayeredRegion = 2,
    };

    enum class SelectionKind {
        None = 0,
        Tile = 1,
        Prop = 2,
        Region = 3,
    };

    struct CanvasSelection {
        SelectionKind kind = SelectionKind::None;
        int tile_x = -1;
        int tile_y = -1;
        std::string prop_asset_id;
        int region_min_x = -1;
        int region_min_y = -1;
        int region_max_x = -1;
        int region_max_y = -1;
        std::string source;
        std::string trigger_id;
        std::string ability_id;
    };

    struct InlineBadge {
        std::string label;
        std::string kind;
        std::string trigger_id;
        std::string ability_id;
        int tile_x = -1;
        int tile_y = -1;
        int region_min_x = -1;
        int region_min_y = -1;
        int region_max_x = -1;
        int region_max_y = -1;
        std::string prop_asset_id;
        bool selected = false;
    };

    struct HoverPreview {
        bool active = false;
        SelectionKind kind = SelectionKind::None;
        int tile_x = -1;
        int tile_y = -1;
        std::string prop_asset_id;
        int region_min_x = -1;
        int region_min_y = -1;
        int region_max_x = -1;
        int region_max_y = -1;
        std::string trigger_id;
        std::string ability_id;
        bool would_conflict = false;
    };

    struct ConflictWarning {
        std::string kind;
        std::string message;
        std::string severity;
        std::string policy;
        std::string primary_kind;
        std::string primary_trigger_id;
        std::string primary_ability_id;
        std::string primary_asset_path;
        std::string secondary_kind;
        std::string secondary_trigger_id;
        std::string secondary_ability_id;
        std::string secondary_asset_path;
        bool can_keep_primary = false;
        bool can_keep_secondary = false;
        bool can_swap_triggers = false;
        bool can_replace_secondary = false;
        std::string trigger_id;
        std::string ability_id;
        std::string other_trigger_id;
        std::string other_ability_id;
        std::string other_kind;
        int tile_x = -1;
        int tile_y = -1;
        std::string prop_asset_id;
        int region_min_x = -1;
        int region_min_y = -1;
        int region_max_x = -1;
        int region_max_y = -1;
        int other_tile_x = -1;
        int other_tile_y = -1;
        std::string other_prop_asset_id;
        int other_region_min_x = -1;
        int other_region_min_y = -1;
        int other_region_max_x = -1;
        int other_region_max_y = -1;
    };

    struct TriggerMenuEntry {
        std::string trigger_id;
        std::string label;
        bool recommended = false;
        bool selected = false;
        bool allowed = true;
    };

    struct RenderSnapshot {
        bool visible = true;
        bool has_target_overlay = false;
        bool has_binding_panel = false;
        std::string selected_ability_id;
        std::string selected_trigger_id;
        std::vector<std::string> available_triggers;
        std::vector<TriggerMenuEntry> selection_trigger_menu;
        size_t badge_count = 0;
        size_t conflict_count = 0;
        bool has_conflicts = false;
        size_t prop_handle_count = 0;
        size_t tile_overlay_count = 0;
        size_t region_overlay_count = 0;
        CanvasSelection selection;
        HoverPreview hover_preview;
        std::vector<InlineBadge> badges;
        std::vector<ConflictWarning> conflicts;
        std::vector<MapAbilityBindingPanel::PropHandleEntry> prop_handles;
        std::vector<MapAbilityBindingPanel::TileOverlayEntry> tile_overlays;
        std::vector<MapAbilityBindingPanel::RegionOverlayEntry> region_overlays;
    };

    SpatialAbilityCanvasPanel() : EditorPanel("Spatial Ability Canvas") {}

    void Render(const urpg::FrameContext& context) override;
    void SetSpatialTarget(urpg::presentation::SpatialMapOverlay* overlay);
    void SetBindingPanel(MapAbilityBindingPanel* panel);
    void SetProjectionSettings(const PropPlacementPanel::ScreenProjectionSettings& settings);
    void SetAvailableTriggers(std::vector<std::string> trigger_ids);
    bool ClickAtScreen(float screen_x, float screen_y);
    bool HoverAtScreen(float screen_x, float screen_y);
    bool DragSelectionToScreen(float screen_x, float screen_y);
    bool ResizeSelectedRegionToScreen(float screen_x, float screen_y);
    bool SetSelectionTriggerId(const std::string& trigger_id);
    bool ResolveConflictByRemovingSecondary(size_t index);
    bool ResolveConflictByRemovingPrimary(size_t index);
    bool SwapConflictTriggers(size_t index);
    bool ReplaceSecondaryWithPrimaryAsset(size_t index);
    bool ResolveConflictWithSuggestion(size_t index);
    bool ApplySelectedAssetToSelection();

    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    void captureRenderSnapshot();

    urpg::presentation::SpatialMapOverlay* m_target_overlay = nullptr;
    MapAbilityBindingPanel* m_binding_panel = nullptr;
    PropPlacementPanel::ScreenProjectionSettings m_projection_settings;
    std::vector<std::string> m_available_triggers{"confirm_interact", "touch_interact", "inspect_prop", "enter_region"};
    CanvasSelection m_selection;
    HoverPreview m_hover_preview;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
