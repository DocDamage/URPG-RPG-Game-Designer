#include "editor/spatial/map_environment_preview_panel.h"

#include <utility>

namespace urpg::editor {

void MapEnvironmentPreviewPanel::loadDocument(urpg::map::MapEnvironmentPreviewDocument document) {
    document_ = std::move(document);
    loaded_ = true;
    refreshPreview();
}

void MapEnvironmentPreviewPanel::selectTile(int32_t x, int32_t y) {
    snapshot_.selected_x = x;
    snapshot_.selected_y = y;
    if (loaded_) {
        refreshPreview();
    }
}

void MapEnvironmentPreviewPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load a map environment preview before rendering this panel.";
        return;
    }

    refreshPreview();
}

void MapEnvironmentPreviewPanel::refreshPreview() {
    preview_ = urpg::map::PreviewMapEnvironment(document_, snapshot_.selected_x, snapshot_.selected_y);
    snapshot_.disabled = false;
    snapshot_.map_id = document_.map_id;
    snapshot_.region_id = preview_.region ? preview_.region->id : "";
    snapshot_.weather = preview_.region ? preview_.region->weather : document_.base_weather;
    snapshot_.runtime_command_count = preview_.runtime_intent.commands.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Map environment preview is ready." : "Map environment preview has diagnostics.";
}

} // namespace urpg::editor
