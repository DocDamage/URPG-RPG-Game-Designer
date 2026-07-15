#include "editor/spatial/map_authoring_workspace.h"

#include "editor/spatial/level_builder_workspace.h"
#include "editor/spatial/spatial_authoring_workspace.h"

#include <array>
#include <algorithm>

namespace urpg::editor {
namespace {

struct ModeDefinition { MapAuthoringMode mode; const char* id; const char* label; };

constexpr std::array<ModeDefinition, 10> kModes = {{{MapAuthoringMode::Canvas, "canvas", "Canvas"},
                                                     {MapAuthoringMode::Tiles, "tiles", "Tiles"},
                                                     {MapAuthoringMode::Parts, "parts", "Parts"},
                                                     {MapAuthoringMode::Props, "props", "Props"},
                                                     {MapAuthoringMode::Events, "events", "Events"},
                                                     {MapAuthoringMode::Abilities, "abilities", "Abilities"},
                                                     {MapAuthoringMode::World, "world", "World"},
                                                     {MapAuthoringMode::Validate, "validate", "Validate"},
                                                     {MapAuthoringMode::Playtest, "playtest", "Playtest"},
                                                     {MapAuthoringMode::Package, "package", "Package"}}};

} // namespace

const char* MapAuthoringWorkspace::modeId(MapAuthoringMode mode) {
    for (const auto& definition : kModes) if (definition.mode == mode) return definition.id;
    return "canvas";
}

void MapAuthoringWorkspace::bind(LevelBuilderWorkspace* levelBuilder, SpatialAuthoringWorkspace* perspective2D) {
    level_builder_ = levelBuilder;
    perspective_2d_ = perspective2D;
    rebuildSnapshot();
}

void MapAuthoringWorkspace::setProjectRoot(std::filesystem::path projectRoot) {
    context_.setProjectRoot(std::move(projectRoot));
    rebuildSnapshot();
}

void MapAuthoringWorkspace::setActiveMapId(std::string mapId) {
    context_.setActiveMapId(std::move(mapId));
    rebuildSnapshot();
}

bool MapAuthoringWorkspace::activateMode(MapAuthoringMode mode) {
    if (!level_builder_ && !perspective_2d_) return false;
    active_mode_ = mode;
    if (level_builder_) {
        using WorkflowMode = LevelBuilderWorkspace::WorkflowMode;
        switch (mode) {
        case MapAuthoringMode::Parts: level_builder_->SetActiveMode(WorkflowMode::Build); break;
        case MapAuthoringMode::Validate: level_builder_->SetActiveMode(WorkflowMode::Validate); break;
        case MapAuthoringMode::Playtest: level_builder_->SetActiveMode(WorkflowMode::Playtest); break;
        case MapAuthoringMode::Package: level_builder_->SetActiveMode(WorkflowMode::Package); break;
        default: level_builder_->SetActiveMode(WorkflowMode::Perspective2D); break;
        }
    }
    if (perspective_2d_) {
        switch (mode) {
        case MapAuthoringMode::Tiles: (void)perspective_2d_->ActivateToolbarAction("tiles"); break;
        case MapAuthoringMode::Parts: (void)perspective_2d_->ActivateToolbarAction("parts"); break;
        case MapAuthoringMode::Props: (void)perspective_2d_->ActivateToolbarAction("props"); break;
        case MapAuthoringMode::Events: (void)perspective_2d_->ActivateToolbarAction("events"); break;
        case MapAuthoringMode::Abilities: (void)perspective_2d_->ActivateToolbarAction("abilities"); break;
        case MapAuthoringMode::World: (void)perspective_2d_->ActivateToolbarAction("worldbuilding"); break;
        default: (void)perspective_2d_->ActivateToolbarAction("composite"); break;
        }
    }
    rebuildSnapshot();
    return true;
}

bool MapAuthoringWorkspace::focusGridDiagnostic(size_t diagnosticIndex) {
    if (level_builder_ == nullptr) {
        return false;
    }
    const auto focused = level_builder_->FocusDiagnostic(diagnosticIndex);
    if (!focused.success) {
        rebuildSnapshot();
        return false;
    }
    active_mode_ = MapAuthoringMode::Validate;
    auto selection = context_.snapshot().selection;
    selection.objectId = focused.instance_id;
    selection.viewportFocus = "grid_diagnostic:" + std::to_string(diagnosticIndex);
    selection.activeTool = "validate";
    context_.setSelection(std::move(selection));
    rebuildSnapshot();
    return true;
}

MapAuthoringHistoryResult MapAuthoringWorkspace::undo() {
    if (active_mode_ != MapAuthoringMode::Parts && perspective_2d_ != nullptr && perspective_2d_->canUndoPerspective2D()) {
        const bool success = perspective_2d_->UndoPerspective2D();
        rebuildSnapshot();
        return {success, success ? "Undo applied." : "Perspective 2D undo could not be applied.", "perspective_2d"};
    }
    if (level_builder_ == nullptr) return {false, "Grid Parts history is unavailable.", ""};
    const auto result = level_builder_->UndoLastEdit();
    rebuildSnapshot();
    return {result.success, result.message, result.source.empty() ? "grid_parts" : result.source};
}

MapAuthoringHistoryResult MapAuthoringWorkspace::redo() {
    if (active_mode_ != MapAuthoringMode::Parts && perspective_2d_ != nullptr && perspective_2d_->canRedoPerspective2D()) {
        const bool success = perspective_2d_->RedoPerspective2D();
        rebuildSnapshot();
        return {success, success ? "Redo applied." : "Perspective 2D redo could not be applied.", "perspective_2d"};
    }
    if (level_builder_ == nullptr) return {false, "Grid Parts history is unavailable.", ""};
    const auto result = level_builder_->RedoLastEdit();
    rebuildSnapshot();
    return {result.success, result.message, result.source.empty() ? "grid_parts" : result.source};
}

void MapAuthoringWorkspace::setLayout(MapAuthoringLayoutState layout) {
    // Keep enough central canvas space for the map at the minimum supported
    // editor size while still allowing creators to tune the surrounding panes.
    layout.paletteWidthFraction = std::clamp(layout.paletteWidthFraction, 0.12f, 0.35f);
    layout.inspectorWidthFraction = std::clamp(layout.inspectorWidthFraction, 0.12f, 0.35f);
    layout.diagnosticsHeightFraction = std::clamp(layout.diagnosticsHeightFraction, 0.12f, 0.40f);
    layout_ = layout;
    rebuildSnapshot();
}

EditorAssetDropDecision MapAuthoringWorkspace::acceptAssetDrop(const EditorAssetDragPayload& payload,
                                                                const std::string_view target_mode) {
    auto decision = assessEditorAssetDrop(payload, true);
    if (!decision.accepted) {
        return decision;
    }
    decision = validateEditorAssetAttachmentRevision(payload, context_.snapshot().projectRoot);
    if (!decision.accepted) {
        return decision;
    }
    if (perspective_2d_ == nullptr) {
        return {false, "asset_drop_workspace_unbound", "The Perspective 2D Map workspace is not available.",
                "Open a project map before dropping an asset."};
    }
    bool accepted = false;
    if (target_mode == "tiles") {
        accepted = perspective_2d_->AddAttachedAssetToTilePalette(payload.assetId, payload.projectPath);
    } else if (target_mode == "props") {
        accepted = perspective_2d_->AddAttachedAssetToPropPalette(payload.assetId, payload.projectPath);
    } else {
        return {false, "asset_drop_target_requires_tiles_or_props",
                "This Map mode does not accept an asset drop yet.",
                "Switch to Tiles or Props to add an attached asset to that authoring palette."};
    }
    if (!accepted) {
        return {false, "asset_drop_target_rejected", "The Map palette could not accept this attached asset.",
                "Check that the attached asset has a stable project path."};
    }
    context_.setDocumentDirty(MapAuthoringDocumentOwner::Perspective2D, true);
    rebuildSnapshot();
    return {true, "asset_drop_accepted", "Attached asset was added to the Map palette.", ""};
}

EditorAssetDropDecision MapAuthoringWorkspace::placeAssetDrop(const EditorAssetDragPayload& payload,
                                                               const std::string_view target_mode,
                                                               const float canvas_screen_x,
                                                               const float canvas_screen_y) {
    auto decision = assessEditorAssetDrop(payload, true);
    if (!decision.accepted) {
        return decision;
    }
    decision = validateEditorAssetAttachmentRevision(payload, context_.snapshot().projectRoot);
    if (!decision.accepted) {
        return decision;
    }
    if (perspective_2d_ == nullptr) {
        return {false, "asset_drop_workspace_unbound", "The Perspective 2D Map workspace is not available.",
                "Open a project map before dropping an asset."};
    }
    if (target_mode == "tiles") {
        if (!perspective_2d_->PlaceAttachedAssetTileFromScreen(payload.assetId, payload.projectPath, canvas_screen_x,
                                                                canvas_screen_y)) {
            return {false, "asset_drop_tile_placement_rejected",
                    "The attached asset could not be placed on the current tile layer.",
                    "Select a visible unlocked empty tile cell, or use the tile editor to replace an existing tile."};
        }
    } else if (target_mode == "props") {
        if (!perspective_2d_->PlaceAttachedAssetPropFromScreen(payload.assetId, payload.projectPath, canvas_screen_x,
                                                                canvas_screen_y)) {
            return {false, "asset_drop_prop_placement_rejected",
                    "The attached asset could not be placed on the current map surface.",
                    "Drop inside the map canvas while Props mode is active."};
        }
    } else if (target_mode == "events") {
        if (payload.mediaKind != "image") {
            return {false, "asset_drop_event_requires_image",
                    "Only attached image assets can be placed as authored map events.",
                    "Drop an attached image asset while Events mode is active."};
        }
        if (!perspective_2d_->PlaceAttachedAssetEventFromScreen(payload.assetId, payload.projectPath, canvas_screen_x,
                                                                 canvas_screen_y)) {
            return {false, "asset_drop_event_placement_rejected",
                    "The attached image could not be placed as an authored map event.",
                    "Select a visible unlocked event or object layer and drop inside the map canvas."};
        }
    } else {
        return {false, "asset_drop_target_requires_tiles_props_or_events",
                "This Map mode does not accept an asset drop yet.",
                "Switch to Tiles, Props, or Events and drop inside the map canvas."};
    }
    context_.setDocumentDirty(MapAuthoringDocumentOwner::Perspective2D, true);
    rebuildSnapshot();
    const std::string message = target_mode == "events"
                                    ? "Attached image event metadata was authored as one undoable action; event sprite rendering remains separate."
                                    : "Attached asset was placed on the Map as one undoable action.";
    return {true, "asset_drop_placed", message, ""};
}

void MapAuthoringWorkspace::setNextActionHint(std::string hint) {
    next_action_hint_ = std::move(hint);
    rebuildSnapshot();
}

void MapAuthoringWorkspace::clearNextActionHint() {
    next_action_hint_.clear();
    rebuildSnapshot();
}

void MapAuthoringWorkspace::refresh() { rebuildSnapshot(); }

void MapAuthoringWorkspace::rebuildSnapshot() {
    if (active_mode_ != MapAuthoringMode::Parts && perspective_2d_ != nullptr &&
        (perspective_2d_->canUndoPerspective2D() || perspective_2d_->canRedoPerspective2D())) {
        context_.setChildHistoryAvailability(perspective_2d_->canUndoPerspective2D(), perspective_2d_->canRedoPerspective2D(),
                                             MapAuthoringDocumentOwner::Perspective2D);
    } else if (level_builder_ != nullptr) {
        const auto& child = level_builder_->lastRenderSnapshot();
        context_.setChildHistoryAvailability(child.can_undo, child.can_redo, MapAuthoringDocumentOwner::GridParts);
    }
    snapshot_.activeMode = modeId(active_mode_);
    snapshot_.context = context_.snapshot();
    snapshot_.hasLevelBuilder = level_builder_ != nullptr;
    snapshot_.hasPerspective2D = perspective_2d_ != nullptr;
    snapshot_.modes.clear();
    snapshot_.layout = layout_;
    for (const auto& definition : kModes) {
        snapshot_.modes.push_back({definition.id, definition.label, definition.mode == active_mode_,
                                   level_builder_ != nullptr || perspective_2d_ != nullptr});
    }
    if (!next_action_hint_.empty()) {
        snapshot_.nextAction = next_action_hint_;
    } else if (!level_builder_ && !perspective_2d_) {
        snapshot_.nextAction = "Bind map workspaces to begin authoring.";
    } else if (active_mode_ == MapAuthoringMode::Package && level_builder_ != nullptr) {
        const auto& readiness = level_builder_->lastRenderSnapshot().readiness_evidence;
        const auto& package = level_builder_->lastRenderSnapshot().package;
        if (!readiness.has_player_spawn) {
            snapshot_.nextAction = "Select a map part and set the player spawn before packaging.";
        } else if (!readiness.has_objective) {
            snapshot_.nextAction = "Set a reachable map objective before packaging.";
        } else if (!readiness.reachability_passed) {
            snapshot_.nextAction = "Run a successful map playtest to prove the objective is reachable.";
        } else if (!readiness.target_export_checks_passed) {
            snapshot_.nextAction = "Run target export checks before packaging.";
        } else if (!readiness.accessibility_checks_passed) {
            snapshot_.nextAction = "Record the map accessibility check before packaging.";
        } else if (!readiness.performance_budget_passed) {
            snapshot_.nextAction = "Record the map performance budget before packaging.";
        } else if (readiness.human_review_required && !readiness.human_review_passed) {
            snapshot_.nextAction = "Complete the required human map review before packaging.";
        } else if (!package.can_export) {
            snapshot_.nextAction = "Resolve the remaining governed-asset or package diagnostics before packaging.";
        } else {
            snapshot_.nextAction = "Map package evidence is ready; export the reviewed map when ready.";
        }
    } else if (snapshot_.context.validation.blockingCount > 0) {
        snapshot_.nextAction = snapshot_.context.validation.nextAction;
    } else if (snapshot_.context.gridPartsDirty || snapshot_.context.perspective2DDirty) {
        snapshot_.nextAction = "Save changed map documents before playtest.";
    } else {
        snapshot_.nextAction = "Choose a mode and begin authoring.";
    }
}

} // namespace urpg::editor
