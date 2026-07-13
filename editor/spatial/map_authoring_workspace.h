#pragma once

#include "editor/spatial/map_authoring_context.h"
#include "editor/assets/editor_asset_drag_payload.h"
#include "editor/ui/editor_context_action.h"

#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

class LevelBuilderWorkspace;
class SpatialAuthoringWorkspace;

enum class MapAuthoringMode { Canvas, Tiles, Parts, Props, Events, Abilities, World, Validate, Playtest, Package };

struct MapAuthoringModeState {
    std::string id;
    std::string label;
    bool active = false;
    bool available = true;
};

struct MapAuthoringLayoutState {
    float paletteWidthFraction = 0.22f;
    float inspectorWidthFraction = 0.24f;
    float diagnosticsHeightFraction = 0.24f;
    bool paletteVisible = true;
    bool inspectorVisible = true;
    bool diagnosticsVisible = true;
};

struct MapAuthoringWorkspaceSnapshot {
    std::string activeMode = "canvas";
    std::vector<MapAuthoringModeState> modes;
    MapAuthoringLayoutState layout;
    MapAuthoringContextSnapshot context;
    bool hasLevelBuilder = false;
    bool hasPerspective2D = false;
    std::string activeContextRoute;
    std::string activeContextObjectId;
    std::string activeContextReturnRoute;
    std::string nextAction = "Bind map workspaces to begin authoring.";
};

struct MapAuthoringHistoryResult {
    bool success = false;
    std::string message;
    std::string owner;
};

// One creator-facing map surface that routes to the existing native child
// workspaces. It never owns or converts their underlying documents.
class MapAuthoringWorkspace {
  public:
    void bind(LevelBuilderWorkspace* levelBuilder, SpatialAuthoringWorkspace* perspective2D);
    bool activateMode(MapAuthoringMode mode);
    // Focuses an existing Grid Parts diagnostic through the unified Map
    // surface. The child document remains the diagnostic source of truth.
    bool focusGridDiagnostic(size_t diagnosticIndex);
    MapAuthoringHistoryResult undo();
    MapAuthoringHistoryResult redo();
    void setLayout(MapAuthoringLayoutState layout);
    EditorAssetDropDecision acceptAssetDrop(const EditorAssetDragPayload& payload, std::string_view target_mode);
    void setNextActionHint(std::string hint);
    void clearNextActionHint();
    void setProjectRoot(std::filesystem::path projectRoot);
    void setActiveMapId(std::string mapId);
    EditorContextActionResult openContextAction(EditorContextAction action);
    EditorContextActionResult returnFromContextAction();
    // The app shell uses this to render a contextual tab without gaining
    // ownership of the routing stack or the preserved Map selection.
    const EditorContextAction* activeContextAction() const;
    void refresh();

    MapAuthoringContext& context() { return context_; }
    const MapAuthoringContext& context() const { return context_; }
    const MapAuthoringWorkspaceSnapshot& snapshot() const { return snapshot_; }
    static const char* modeId(MapAuthoringMode mode);

  private:
    void rebuildSnapshot();

    LevelBuilderWorkspace* level_builder_ = nullptr;
    SpatialAuthoringWorkspace* perspective_2d_ = nullptr;
    MapAuthoringContext context_;
    MapAuthoringMode active_mode_ = MapAuthoringMode::Canvas;
    MapAuthoringLayoutState layout_;
    std::string next_action_hint_;
    EditorContextActionStack context_actions_;
    MapAuthoringWorkspaceSnapshot snapshot_;
};

} // namespace urpg::editor
