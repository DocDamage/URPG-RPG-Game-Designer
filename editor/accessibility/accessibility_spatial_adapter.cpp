#include "editor/accessibility/accessibility_spatial_adapter.h"

namespace urpg::editor {

using namespace urpg::accessibility;

std::vector<UiElementSnapshot> AccessibilitySpatialAdapter::ingest(
    const ElevationBrushPanel::RenderSnapshot& elevationSnapshot,
    const PropPlacementPanel::RenderSnapshot& propSnapshot,
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

} // namespace urpg::editor
