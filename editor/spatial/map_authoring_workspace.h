#pragma once

#include "editor/spatial/map_authoring_context.h"
#include "editor/assets/editor_asset_drag_payload.h"

#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

class LevelBuilderWorkspace;
class SpatialAuthoringWorkspace;

enum class MapAuthoringMode { Canvas, Tiles, Parts, Props, Events, Abilities, World, Validate, Playtest, Package };
enum class MapAuthoringEntrySource { Project, Object, Asset, Diagnostic, Playtest };

struct MapAuthoringEntryRequest {
    MapAuthoringEntrySource source = MapAuthoringEntrySource::Project;
    std::string map_id;
    std::string object_id;
    std::string event_id;
    std::string part_id;
    std::string focus;
    MapAuthoringMode preferred_mode = MapAuthoringMode::Canvas;
};

struct MapAuthoringEntryResult {
    bool success = false;
    std::string code;
    std::string canonical_route = "map";
    std::string active_mode;
};

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

struct MapAuthoringRegionState {
    std::string id;
    std::string label;
    bool visible = true;
    bool resizable = true;
};

struct MapAuthoringWorkspaceSnapshot {
    std::string activeMode = "canvas";
    std::vector<MapAuthoringModeState> modes;
    MapAuthoringLayoutState layout;
    std::vector<MapAuthoringRegionState> regions;
    bool layoutRecovered = false;
    std::string layoutRecoveryMessage;
    MapAuthoringContextSnapshot context;
    bool hasLevelBuilder = false;
    bool hasPerspective2D = false;
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
    MapAuthoringEntryResult enterCanonicalRoute(const MapAuthoringEntryRequest& request);
    // Focuses an existing Grid Parts diagnostic through the unified Map
    // surface. The child document remains the diagnostic source of truth.
    bool focusGridDiagnostic(size_t diagnosticIndex);
    MapAuthoringHistoryResult undo();
    MapAuthoringHistoryResult redo();
    void setLayout(MapAuthoringLayoutState layout);
    void resetLayout();
    EditorAssetDropDecision acceptAssetDrop(const EditorAssetDragPayload& payload, std::string_view target_mode);
    EditorAssetDropDecision placeAssetDrop(const EditorAssetDragPayload& payload,
                                           std::string_view target_mode,
                                           float canvas_screen_x,
                                           float canvas_screen_y);
    // Replaces supported attached visual references in the active Perspective
    // 2D Map only. The source remains attached; deletion and project-wide
    // replacement require their own domain-owner operations.
    EditorAssetDropDecision replaceActiveMapAttachedAssetReferences(
        std::string_view source_asset_id, const EditorAssetDragPayload& replacement);
    void setNextActionHint(std::string hint);
    void clearNextActionHint();
    void setProjectRoot(std::filesystem::path projectRoot);
    void setActiveMapId(std::string mapId);
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
    bool layout_recovered_ = false;
    std::string layout_recovery_message_;
    std::string next_action_hint_;
    MapAuthoringWorkspaceSnapshot snapshot_;
};

} // namespace urpg::editor
