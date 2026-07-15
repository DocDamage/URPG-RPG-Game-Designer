#include "editor/accessibility/accessibility_spatial_adapter.h"

namespace urpg::editor {

using namespace urpg::accessibility;

std::vector<UiElementSnapshot> AccessibilitySpatialAdapter::ingest(
    const ElevationBrushPanel::RenderSnapshot& elevationSnapshot,
    const PropPlacementPanel::RenderSnapshot& propSnapshot,
    const MapAuthoringWorkspaceSnapshot* mapSnapshot) {
    return ingest(elevationSnapshot, propSnapshot, GridPartPlacementPanel::RenderSnapshot{}, mapSnapshot);
}

std::vector<UiElementSnapshot> AccessibilitySpatialAdapter::ingest(
    const ElevationBrushPanel::RenderSnapshot& elevationSnapshot,
    const PropPlacementPanel::RenderSnapshot& propSnapshot,
    const GridPartPlacementPanel::RenderSnapshot& gridPartSnapshot,
    const MapAuthoringWorkspaceSnapshot* mapSnapshot) {
    std::vector<UiElementSnapshot> elements;

    // Element 1: elevation brush panel presence
    {
        UiElementSnapshot el;
        el.id = "spatial.elevation_brush";
        el.label = "Elevation Brush";
        el.hasFocus = elevationSnapshot.visible && elevationSnapshot.has_target;
        el.focusOrder = 1;
        el.contrastRatio = 0.0f;
        el.sourceContext = "editor/spatial/elevation_brush_panel.h";
        elements.push_back(std::move(el));
    }

    // Element 2: prop placement panel presence
    {
        UiElementSnapshot el;
        el.id = "spatial.prop_placement";
        el.label = "Prop Placement";
        el.hasFocus = propSnapshot.visible && propSnapshot.has_target;
        el.focusOrder = 2;
        el.contrastRatio = 0.0f;
        el.sourceContext = "editor/spatial/prop_placement_panel.h";
        elements.push_back(std::move(el));
    }

    // Element 3: selected asset binding (empty if no asset selected)
    {
        UiElementSnapshot el;
        el.id = "spatial.selected_asset";
        el.label = propSnapshot.selected_asset_id;
        el.hasFocus = propSnapshot.has_target && !propSnapshot.selected_asset_id.empty();
        el.focusOrder = 3;
        el.contrastRatio = 0.0f;
        el.sourceContext = "editor/spatial/prop_placement_panel.h";
        elements.push_back(std::move(el));
    }

    // Element 4: last placed prop (only when a prop was recently added)
    if (propSnapshot.last_added_asset_id.has_value()) {
        UiElementSnapshot el;
        el.id = "spatial.last_added_prop";
        el.label = *propSnapshot.last_added_asset_id;
        el.hasFocus = false;
        el.focusOrder = 0;
        el.contrastRatio = 0.0f;
        el.sourceContext = "editor/spatial/prop_placement_panel.h";
        elements.push_back(std::move(el));
    }

    // Grid Parts and the catalog-backed smart-prefab route have explicit
    // labelled review/apply alternatives. They are reported independently of
    // the canvas so audit consumers do not infer canvas-only operation.
    if (gridPartSnapshot.has_document || gridPartSnapshot.has_catalog || !gridPartSnapshot.selected_smart_prefab_id.empty()) {
        UiElementSnapshot el;
        el.id = "spatial.grid_part_placement";
        el.label = "Grid Part Placement";
        el.hasFocus = gridPartSnapshot.visible && gridPartSnapshot.has_document && gridPartSnapshot.has_catalog;
        el.focusOrder = 4;
        el.contrastRatio = 0.0f;
        el.sourceContext = "editor/spatial/grid_part_placement_panel.h";
        elements.push_back(std::move(el));
    }
    if (!gridPartSnapshot.selected_smart_prefab_id.empty()) {
        UiElementSnapshot review;
        review.id = "spatial.smart_prefab.review";
        review.label = "Review native smart prefab: " + gridPartSnapshot.selected_smart_prefab_id;
        review.hasFocus = gridPartSnapshot.visible && gridPartSnapshot.has_document && gridPartSnapshot.has_catalog;
        review.focusOrder = 5;
        review.contrastRatio = 0.0f;
        review.sourceContext = "editor/spatial/grid_part_placement_panel.h";
        elements.push_back(std::move(review));

        UiElementSnapshot apply;
        apply.id = "spatial.smart_prefab.apply";
        apply.label = "Apply reviewed native smart prefab";
        apply.hasFocus = gridPartSnapshot.visible && gridPartSnapshot.has_document && gridPartSnapshot.has_catalog;
        apply.focusOrder = 6;
        apply.contrastRatio = 0.0f;
        apply.sourceContext = "editor/spatial/grid_part_placement_panel.h";
        elements.push_back(std::move(apply));
    }
    if (!gridPartSnapshot.selected_part_id.empty()) {
        UiElementSnapshot review;
        review.id = "spatial.grid_part_rectangle.review";
        review.label = "Review selected Grid Part rectangle fill: " + gridPartSnapshot.selected_part_id;
        review.hasFocus = gridPartSnapshot.visible && gridPartSnapshot.has_document && gridPartSnapshot.has_catalog;
        review.focusOrder = 7;
        review.contrastRatio = 0.0f;
        review.sourceContext = "editor/spatial/grid_part_placement_panel.h";
        elements.push_back(std::move(review));

        UiElementSnapshot apply;
        apply.id = "spatial.grid_part_rectangle.apply";
        apply.label = "Apply reviewed Grid Part rectangle fill";
        apply.hasFocus = gridPartSnapshot.visible && gridPartSnapshot.has_document && gridPartSnapshot.has_catalog;
        apply.focusOrder = 8;
        apply.contrastRatio = 0.0f;
        apply.sourceContext = "editor/spatial/grid_part_placement_panel.h";
        elements.push_back(std::move(apply));
    }

    // The coordinator is the native owner of the creator-facing Map mode
    // route. Surface its currently available modes as structured alternatives
    // to interacting with an unlabelled canvas-only control.
    if (mapSnapshot != nullptr) {
        int focusOrder = 10;
        for (const auto& mode : mapSnapshot->modes) {
            UiElementSnapshot el;
            el.id = "map.mode." + mode.id;
            el.label = mode.label;
            el.hasFocus = mode.active && mode.available;
            el.focusOrder = focusOrder++;
            el.contrastRatio = 0.0f;
            el.sourceContext = "editor/spatial/map_authoring_workspace.h";
            elements.push_back(std::move(el));
        }

        UiElementSnapshot context;
        context.id = "map.context";
        context.label = mapSnapshot->context.activeMapId.empty()
                            ? "Map context unavailable"
                            : "Map context: " + mapSnapshot->context.activeMapId;
        context.hasFocus = false;
        context.focusOrder = 0;
        context.contrastRatio = 0.0f;
        context.sourceContext = "editor/spatial/map_authoring_workspace.h";
        elements.push_back(std::move(context));
    }

    return elements;
}

std::vector<UiElementSnapshot> AccessibilitySpatialAdapter::ingestCreatorCommand(
    const nlohmann::json& creator_command_snapshot) {
    std::vector<UiElementSnapshot> elements;
    if (!creator_command_snapshot.is_object()) {
        return elements;
    }
    const auto plan = creator_command_snapshot.value("plan", nlohmann::json::object());
    const auto preview = creator_command_snapshot.value("apply_preview", nlohmann::json::object());
    if (!plan.is_object() || !preview.is_object()) {
        return elements;
    }
    const std::string intent = plan.value("intent", "unplanned");
    if (intent.empty() || intent == "unplanned") {
        return elements;
    }
    const std::string domain = preview.value("domain", "native_map");
    const bool would_apply = preview.value("would_apply", false);

    UiElementSnapshot review;
    review.id = "creator_command.review." + intent;
    review.label = "Review native creator " + domain + " plan: " + intent;
    review.hasFocus = true;
    review.focusOrder = 1;
    review.contrastRatio = 0.0f;
    review.sourceContext = "editor/ai/creator_command_panel.h";
    elements.push_back(std::move(review));

    UiElementSnapshot apply;
    apply.id = "creator_command.apply." + intent;
    apply.label = would_apply ? "Apply reviewed native creator plan"
                              : "Creator plan apply unavailable: " + preview.value("code", "unavailable");
    apply.hasFocus = would_apply;
    apply.focusOrder = 2;
    apply.contrastRatio = 0.0f;
    apply.sourceContext = "editor/ai/creator_command_panel.h";
    elements.push_back(std::move(apply));
    return elements;
}

} // namespace urpg::editor
