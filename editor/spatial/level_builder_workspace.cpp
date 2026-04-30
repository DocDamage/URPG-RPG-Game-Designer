#include "editor/spatial/level_builder_workspace.h"

#include "engine/core/map/grid_part_runtime_compiler.h"
#include "engine/core/map/grid_part_serializer.h"
#include "engine/core/map/grid_part_validator.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

namespace {

size_t countBlockingDiagnostics(const std::vector<urpg::map::GridPartDiagnostic>& diagnostics) {
    return static_cast<size_t>(std::count_if(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.severity == urpg::map::GridPartSeverity::Blocker ||
               diagnostic.severity == urpg::map::GridPartSeverity::Error;
    }));
}

void appendBlockingCodes(std::vector<std::string>& codes,
                         const std::vector<urpg::map::GridPartDiagnostic>& diagnostics) {
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.severity == urpg::map::GridPartSeverity::Blocker ||
            diagnostic.severity == urpg::map::GridPartSeverity::Error) {
            codes.push_back(diagnostic.code);
        }
    }
}

const char* severityName(urpg::map::GridPartSeverity severity) {
    switch (severity) {
    case urpg::map::GridPartSeverity::Info:
        return "info";
    case urpg::map::GridPartSeverity::Warning:
        return "warning";
    case urpg::map::GridPartSeverity::Error:
        return "error";
    case urpg::map::GridPartSeverity::Blocker:
        return "blocker";
    }
    return "error";
}

bool isBlocking(urpg::map::GridPartSeverity severity) {
    return severity == urpg::map::GridPartSeverity::Blocker || severity == urpg::map::GridPartSeverity::Error;
}

void appendDiagnosticSummaries(std::vector<LevelBuilderWorkspace::DiagnosticSummary>& target,
                               const std::string& source,
                               const std::vector<urpg::map::GridPartDiagnostic>& diagnostics) {
    for (const auto& diagnostic : diagnostics) {
        LevelBuilderWorkspace::DiagnosticSummary summary;
        summary.source = source;
        summary.severity = severityName(diagnostic.severity);
        summary.code = diagnostic.code;
        summary.message = diagnostic.message;
        summary.instance_id = diagnostic.instance_id;
        summary.part_id = diagnostic.part_id;
        summary.target = diagnostic.target;
        summary.x = diagnostic.x;
        summary.y = diagnostic.y;
        summary.blocking = isBlocking(diagnostic.severity);
        target.push_back(std::move(summary));
    }
}

} // namespace

const char* LevelBuilderWorkspace::modeName(WorkflowMode mode) {
    switch (mode) {
    case WorkflowMode::Build:
        return "build";
    case WorkflowMode::Validate:
        return "validate";
    case WorkflowMode::Playtest:
        return "playtest";
    case WorkflowMode::Package:
        return "package";
    case WorkflowMode::SupportingSpatial:
        return "supporting_spatial";
    }
    return "build";
}

const char* LevelBuilderWorkspace::readinessName(urpg::map::GridPartReadinessLevel readiness) {
    switch (readiness) {
    case urpg::map::GridPartReadinessLevel::Draft:
        return "draft";
    case urpg::map::GridPartReadinessLevel::Playable:
        return "playable";
    case urpg::map::GridPartReadinessLevel::Validated:
        return "validated";
    case urpg::map::GridPartReadinessLevel::Publishable:
        return "publishable";
    case urpg::map::GridPartReadinessLevel::Exportable:
        return "exportable";
    case urpg::map::GridPartReadinessLevel::Certified:
        return "certified";
    }
    return "draft";
}

void LevelBuilderWorkspace::syncPanelVisibility() {
    const bool show_build = active_mode_ == WorkflowMode::Build;
    palette_panel_.SetVisible(show_build);
    placement_panel_.SetVisible(show_build);
    inspector_panel_.SetVisible(show_build || active_mode_ == WorkflowMode::Validate ||
                                active_mode_ == WorkflowMode::Package);
    playtest_panel_.SetVisible(active_mode_ == WorkflowMode::Playtest);
    supporting_spatial_workspace_.SetVisible(active_mode_ == WorkflowMode::SupportingSpatial);
}

void LevelBuilderWorkspace::Render(const urpg::FrameContext& context) {
    if (!m_visible) {
        return;
    }

    syncPanelVisibility();
    palette_panel_.Render(context);
    placement_panel_.Render(context);
    inspector_panel_.Render(context);
    playtest_panel_.Render(context);
    supporting_spatial_workspace_.Render(context);
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetTargets(urpg::map::GridPartDocument* document,
                                       const urpg::map::GridPartCatalog* catalog,
                                       urpg::presentation::SpatialMapOverlay* overlay,
                                       urpg::scene::MapScene* scene) {
    document_ = document;
    catalog_ = catalog;
    overlay_ = overlay;
    scene_ = scene;

    palette_panel_.SetCatalog(catalog);
    placement_panel_.SetTargets(document, catalog, overlay);
    inspector_panel_.SetTargets(document, catalog);
    playtest_panel_.SetTargets(document, catalog);
    supporting_spatial_workspace_.SetTargets(scene, overlay);
    supporting_spatial_workspace_.SetGridPartTargets(document, catalog);
    syncPanelVisibility();
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetProjectionSettings(const PropPlacementPanel::ScreenProjectionSettings& settings) {
    placement_panel_.SetProjectionSettings(settings);
    supporting_spatial_workspace_.SetProjectionSettings(settings);
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetRulesetProfile(urpg::map::GridRulesetProfile ruleset) {
    ruleset_ = std::move(ruleset);
    playtest_panel_.SetRulesetProfile(ruleset_);
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetObjective(urpg::map::MapObjective objective) {
    objective_ = std::move(objective);
    playtest_panel_.SetObjective(objective_);
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetPackageManifest(urpg::map::GridPartPackageManifest manifest) {
    package_manifest_ = std::move(manifest);
    has_custom_package_manifest_ = true;
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetReadinessEvidence(urpg::map::GridPartReadinessEvidence evidence) {
    readiness_evidence_ = evidence;
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetActiveMode(WorkflowMode mode) {
    if (active_mode_ == mode) {
        return;
    }
    active_mode_ = mode;
    syncPanelVisibility();
    captureRenderSnapshot();
}

bool LevelBuilderWorkspace::ActivateToolbarAction(const std::string& action_id) {
    if (action_id == "build") {
        SetActiveMode(WorkflowMode::Build);
        return true;
    }
    if (action_id == "validate") {
        SetActiveMode(WorkflowMode::Validate);
        return true;
    }
    if (action_id == "playtest") {
        SetActiveMode(WorkflowMode::Playtest);
        return true;
    }
    if (action_id == "package") {
        SetActiveMode(WorkflowMode::Package);
        return true;
    }
    if (action_id == "undo") {
        return UndoLastEdit().success;
    }
    if (action_id == "redo") {
        return RedoLastEdit().success;
    }
    if (action_id == "mark_player_spawn") {
        return MarkSelectedInstanceAsPlayerSpawn().success;
    }
    if (action_id == "set_reach_exit_objective") {
        return SetSelectedInstanceAsReachExitObjective().success;
    }
    if (action_id == "mark_target_export_checks_passed") {
        return MarkTargetExportChecksPassed().success;
    }
    if (action_id == "mark_accessibility_checks_passed") {
        return MarkAccessibilityChecksPassed().success;
    }
    if (action_id == "mark_performance_budget_passed") {
        return MarkPerformanceBudgetPassed().success;
    }
    if (action_id == "mark_human_review_passed") {
        return MarkHumanReviewPassed().success;
    }
    if (action_id == "save_level_draft") {
        return SaveLevelDraft().success;
    }
    if (action_id == "export_current_level") {
        return ExportCurrentLevel().success;
    }
    if (action_id == "supporting_spatial") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return true;
    }
    if (action_id == "supporting_elevation") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return supporting_spatial_workspace_.ActivateToolbarAction("elevation");
    }
    if (action_id == "supporting_props") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return supporting_spatial_workspace_.ActivateToolbarAction("props");
    }
    if (action_id == "supporting_abilities") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return supporting_spatial_workspace_.ActivateToolbarAction("abilities");
    }
    if (action_id == "supporting_composite") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return supporting_spatial_workspace_.ActivateToolbarAction("composite");
    }
    if (action_id == "playtest_start") {
        SetActiveMode(WorkflowMode::Playtest);
        const bool launched = playtest_panel_.PlaytestFromStart();
        const auto& playtest_snapshot = playtest_panel_.lastRenderSnapshot();
        if (launched && playtest_snapshot.latest_result.completed_objective &&
            !playtest_snapshot.latest_result.softlocked) {
            readiness_evidence_.reachability_passed = true;
        }
        captureRenderSnapshot();
        return launched;
    }
    if (action_id == "playtest_return") {
        const bool returned = playtest_panel_.ReturnToEditor();
        captureRenderSnapshot();
        return returned;
    }
    return false;
}

LevelBuilderWorkspace::EditHistoryResult LevelBuilderWorkspace::UndoLastEdit() {
    EditHistoryResult result;
    result.command_id = "undo";
    result.message = "No edit history is available to undo.";

    if (inspector_panel_.lastRenderSnapshot().can_undo) {
        result.success = inspector_panel_.Undo();
        result.source = "inspector";
    } else if (placement_panel_.lastRenderSnapshot().can_undo) {
        result.success = placement_panel_.Undo();
        result.source = "placement";
    }

    if (result.success) {
        result.message = "Undo applied.";
    }
    last_edit_history_result_ = result;
    captureRenderSnapshot();
    return last_edit_history_result_;
}

LevelBuilderWorkspace::EditHistoryResult LevelBuilderWorkspace::RedoLastEdit() {
    EditHistoryResult result;
    result.command_id = "redo";
    result.message = "No edit history is available to redo.";

    if (inspector_panel_.lastRenderSnapshot().can_redo) {
        result.success = inspector_panel_.Redo();
        result.source = "inspector";
    } else if (placement_panel_.lastRenderSnapshot().can_redo) {
        result.success = placement_panel_.Redo();
        result.source = "placement";
    }

    if (result.success) {
        result.message = "Redo applied.";
    }
    last_edit_history_result_ = result;
    captureRenderSnapshot();
    return last_edit_history_result_;
}

LevelBuilderWorkspace::FocusDiagnosticResult LevelBuilderWorkspace::FocusDiagnostic(size_t diagnostic_index) {
    FocusDiagnosticResult result;
    result.diagnostic_index = diagnostic_index;

    captureRenderSnapshot();
    if (diagnostic_index >= last_render_snapshot_.diagnostics.size()) {
        result.message = "Diagnostic index is out of range.";
        last_focus_diagnostic_result_ = result;
        captureRenderSnapshot();
        return last_focus_diagnostic_result_;
    }

    const auto& diagnostic = last_render_snapshot_.diagnostics[diagnostic_index];
    result.source = diagnostic.source;
    result.instance_id = diagnostic.instance_id;
    if (diagnostic.instance_id.empty()) {
        result.message = "Diagnostic has no instance target to focus.";
        last_focus_diagnostic_result_ = result;
        captureRenderSnapshot();
        return last_focus_diagnostic_result_;
    }

    if (!inspector_panel_.SelectInstance(diagnostic.instance_id)) {
        result.message = "Diagnostic target instance could not be selected.";
        last_focus_diagnostic_result_ = result;
        captureRenderSnapshot();
        return last_focus_diagnostic_result_;
    }

    result.success = true;
    result.message = "Diagnostic target selected.";
    SetActiveMode(WorkflowMode::Validate);
    last_focus_diagnostic_result_ = result;
    captureRenderSnapshot();
    return last_focus_diagnostic_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkSelectedInstanceAsPlayerSpawn() {
    AuthoringCommandResult result;
    result.command_id = "mark_player_spawn";

    const auto selected_instance_id = inspector_panel_.lastRenderSnapshot().selected_instance_id;
    result.instance_id = selected_instance_id;
    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before marking a spawn.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }
    if (selected_instance_id.empty() || document_->findPart(selected_instance_id) == nullptr) {
        result.message = "Select a placed grid part before marking a player spawn.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }

    if (!inspector_panel_.SetProperty("role", "player_spawn")) {
        result.message = "Selected part could not be marked as a player spawn.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }

    readiness_evidence_.has_player_spawn = true;
    result.success = true;
    result.message = "Selected part marked as player spawn.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult
LevelBuilderWorkspace::SetSelectedInstanceAsReachExitObjective(const std::string& objective_id) {
    AuthoringCommandResult result;
    result.command_id = "set_reach_exit_objective";

    const auto selected_instance_id = inspector_panel_.lastRenderSnapshot().selected_instance_id;
    result.instance_id = selected_instance_id;
    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before setting an objective.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }
    if (selected_instance_id.empty() || document_->findPart(selected_instance_id) == nullptr) {
        result.message = "Select a placed grid part before setting the objective.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }

    urpg::map::MapObjective objective;
    objective.type = urpg::map::MapObjectiveType::ReachExit;
    objective.objective_id = objective_id.empty() ? "reach_exit" : objective_id;
    objective.target_instance_id = selected_instance_id;
    SetObjective(std::move(objective));
    readiness_evidence_.has_objective = true;
    (void)inspector_panel_.SetProperty("role", "exit");

    result.success = true;
    result.message = "Selected part set as reach-exit objective.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkTargetExportChecksPassed() {
    AuthoringCommandResult result;
    result.command_id = "mark_target_export_checks_passed";
    readiness_evidence_.target_export_checks_passed = true;
    result.success = true;
    result.message = "Target export checks marked as passed.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkAccessibilityChecksPassed() {
    AuthoringCommandResult result;
    result.command_id = "mark_accessibility_checks_passed";
    readiness_evidence_.accessibility_checks_passed = true;
    result.success = true;
    result.message = "Accessibility checks marked as passed.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkPerformanceBudgetPassed() {
    AuthoringCommandResult result;
    result.command_id = "mark_performance_budget_passed";
    readiness_evidence_.performance_budget_passed = true;
    result.success = true;
    result.message = "Performance budget marked as passed.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkHumanReviewPassed() {
    AuthoringCommandResult result;
    result.command_id = "mark_human_review_passed";
    readiness_evidence_.human_review_required = true;
    readiness_evidence_.human_review_passed = true;
    result.success = true;
    result.message = "Human review marked as passed.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::SaveDraftResult LevelBuilderWorkspace::SaveLevelDraft() {
    SaveDraftResult result;

    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before saving.";
        last_save_result_ = result;
        captureRenderSnapshot();
        return last_save_result_;
    }

    result.map_id = document_->mapId();
    result.saved_part_count = document_->parts().size();
    const auto document_validation = urpg::map::ValidateGridPartDocument(*document_, *catalog_);
    result.diagnostic_count = document_validation.diagnostics.size();
    appendBlockingCodes(result.blocker_codes, document_validation.diagnostics);

    if (!result.blocker_codes.empty()) {
        result.message = "Current level draft has blocking document errors.";
        last_save_result_ = std::move(result);
        captureRenderSnapshot();
        return last_save_result_;
    }

    result.success = true;
    result.message = "Current level draft saved.";
    result.serialized_document_json = urpg::map::GridPartDocumentToJson(*document_).dump(2);
    document_->clearDirtyChunks();
    last_save_result_ = std::move(result);
    captureRenderSnapshot();
    return last_save_result_;
}

LevelBuilderWorkspace::LoadDraftResult
LevelBuilderWorkspace::LoadLevelDraft(const std::string& serialized_document_json) {
    LoadDraftResult result;

    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before loading.";
        last_load_result_ = result;
        captureRenderSnapshot();
        return last_load_result_;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(serialized_document_json);
    } catch (const nlohmann::json::exception&) {
        result.message = "Level draft JSON could not be parsed.";
        result.blocker_codes.push_back("draft_json_parse_failed");
        last_load_result_ = std::move(result);
        captureRenderSnapshot();
        return last_load_result_;
    }

    auto parsed = urpg::map::GridPartDocumentFromJson(json);
    if (!parsed.has_value()) {
        result.message = "Level draft JSON is not a valid grid-part document.";
        result.blocker_codes.push_back("draft_document_invalid");
        last_load_result_ = std::move(result);
        captureRenderSnapshot();
        return last_load_result_;
    }

    result.map_id = parsed->mapId();
    if (!document_->mapId().empty() && parsed->mapId() != document_->mapId()) {
        result.message = "Level draft map id does not match the bound document.";
        result.blocker_codes.push_back("draft_map_id_mismatch");
        last_load_result_ = std::move(result);
        captureRenderSnapshot();
        return last_load_result_;
    }

    const auto validation = urpg::map::ValidateGridPartDocument(*parsed, *catalog_);
    result.diagnostic_count = validation.diagnostics.size();
    appendBlockingCodes(result.blocker_codes, validation.diagnostics);
    if (!result.blocker_codes.empty()) {
        result.message = "Level draft has blocking document errors.";
        last_load_result_ = std::move(result);
        captureRenderSnapshot();
        return last_load_result_;
    }

    result.success = true;
    result.message = "Level draft loaded.";
    result.loaded_part_count = parsed->parts().size();
    *document_ = std::move(*parsed);
    placement_panel_.SetTargets(document_, catalog_, overlay_);
    inspector_panel_.SetTargets(document_, catalog_);
    playtest_panel_.SetTargets(document_, catalog_);
    supporting_spatial_workspace_.SetGridPartTargets(document_, catalog_);
    last_load_result_ = std::move(result);
    SetActiveMode(WorkflowMode::Build);
    captureRenderSnapshot();
    return last_load_result_;
}

LevelBuilderWorkspace::ExportResult LevelBuilderWorkspace::ExportCurrentLevel() {
    ExportResult result;

    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before exporting.";
        last_export_result_ = result;
        captureRenderSnapshot();
        return last_export_result_;
    }

    result.map_id = document_->mapId();
    const auto document_validation = urpg::map::ValidateGridPartDocument(*document_, *catalog_);
    const auto ruleset_validation = urpg::map::ValidateGridPartRuleset(*document_, *catalog_, ruleset_);
    const auto objective_validation = urpg::map::ValidateMapObjective(*document_, *catalog_, objective_);
    const auto manifest = has_custom_package_manifest_ ? package_manifest_
                                                       : urpg::map::BuildGridPartPackageManifest(*document_, *catalog_);
    const auto package = urpg::map::ValidateGridPartPackageGovernance(*document_, *catalog_, manifest,
                                                                      readiness_evidence_);

    result.readiness = readinessName(package.readiness);
    result.diagnostic_count = document_validation.diagnostics.size() + ruleset_validation.diagnostics.size() +
                              objective_validation.diagnostics.size() + package.diagnostics.size();
    appendBlockingCodes(result.blocker_codes, document_validation.diagnostics);
    appendBlockingCodes(result.blocker_codes, ruleset_validation.diagnostics);
    appendBlockingCodes(result.blocker_codes, objective_validation.diagnostics);
    appendBlockingCodes(result.blocker_codes, package.diagnostics);

    if (!result.blocker_codes.empty() || !package.canExport()) {
        result.message = "Current level is not exportable.";
        last_export_result_ = std::move(result);
        SetActiveMode(WorkflowMode::Package);
        captureRenderSnapshot();
        return last_export_result_;
    }

    result.success = true;
    result.message = "Current level is exportable.";
    result.serialized_document_json = urpg::map::GridPartDocumentToJson(*document_).dump(2);
    last_export_result_ = std::move(result);
    SetActiveMode(WorkflowMode::Package);
    captureRenderSnapshot();
    return last_export_result_;
}

bool LevelBuilderWorkspace::SelectGridPart(const std::string& part_id) {
    const bool palette_selected = palette_panel_.SelectPart(part_id);
    const bool placement_selected = placement_panel_.SetSelectedPartId(part_id);
    const bool supporting_selected = supporting_spatial_workspace_.SelectGridPart(part_id);
    captureRenderSnapshot();
    return palette_selected && placement_selected && supporting_selected;
}

bool LevelBuilderWorkspace::RouteCanvasHover(float screen_x, float screen_y) {
    if (active_mode_ == WorkflowMode::SupportingSpatial) {
        const bool handled = supporting_spatial_workspace_.RouteCanvasHover(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    if (active_mode_ != WorkflowMode::Build) {
        captureRenderSnapshot();
        return false;
    }
    const bool handled = placement_panel_.HoverSelectedPartFromScreen(screen_x, screen_y);
    captureRenderSnapshot();
    return handled;
}

bool LevelBuilderWorkspace::RouteCanvasPrimaryAction(float screen_x, float screen_y) {
    if (active_mode_ == WorkflowMode::SupportingSpatial) {
        const bool handled = supporting_spatial_workspace_.RouteCanvasPrimaryAction(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    if (active_mode_ != WorkflowMode::Build) {
        captureRenderSnapshot();
        return false;
    }
    const bool placed = placement_panel_.PlaceSelectedPartFromScreen(screen_x, screen_y);
    if (placed && document_ != nullptr) {
        const auto& parts = document_->parts();
        if (!parts.empty()) {
            (void)inspector_panel_.SelectInstance(parts.back().instance_id);
        }
    }
    captureRenderSnapshot();
    return placed;
}

bool LevelBuilderWorkspace::RouteCanvasSecondaryAction(float screen_x, float screen_y) {
    if (active_mode_ == WorkflowMode::SupportingSpatial) {
        const bool handled = supporting_spatial_workspace_.RouteCanvasSecondaryAction(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    if (active_mode_ != WorkflowMode::Build) {
        captureRenderSnapshot();
        return false;
    }
    const bool undone = placement_panel_.Undo();
    captureRenderSnapshot();
    return undone;
}

void LevelBuilderWorkspace::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.active_mode = modeName(active_mode_);
    last_render_snapshot_.has_document = document_ != nullptr;
    last_render_snapshot_.has_catalog = catalog_ != nullptr;
    last_render_snapshot_.has_spatial_overlay = overlay_ != nullptr;
    last_render_snapshot_.has_target_scene = scene_ != nullptr;
    last_render_snapshot_.can_author = document_ != nullptr && catalog_ != nullptr && overlay_ != nullptr;
    last_render_snapshot_.can_playtest = document_ != nullptr && catalog_ != nullptr;
    last_render_snapshot_.has_unsaved_changes = document_ != nullptr && !document_->dirtyChunks().empty();
    last_render_snapshot_.can_undo =
        placement_panel_.lastRenderSnapshot().can_undo || inspector_panel_.lastRenderSnapshot().can_undo;
    last_render_snapshot_.can_redo =
        placement_panel_.lastRenderSnapshot().can_redo || inspector_panel_.lastRenderSnapshot().can_redo;
    last_render_snapshot_.last_authoring_command = last_authoring_command_result_;
    last_render_snapshot_.last_focus_diagnostic = last_focus_diagnostic_result_;
    last_render_snapshot_.last_edit_history = last_edit_history_result_;
    last_render_snapshot_.last_load = last_load_result_;
    last_render_snapshot_.last_save = last_save_result_;
    last_render_snapshot_.last_export = last_export_result_;

    if (document_ == nullptr || catalog_ == nullptr) {
        last_render_snapshot_.status = "disabled";
        last_render_snapshot_.message = "Bind a GridPartDocument and GridPartCatalog to use the native Level Builder.";
    } else if (overlay_ == nullptr) {
        last_render_snapshot_.status = "ready_without_canvas";
        last_render_snapshot_.message =
            "Level Builder is bound; attach a SpatialMapOverlay to enable canvas placement.";
    } else {
        last_render_snapshot_.status = "ready";
        last_render_snapshot_.message = "Grid-part Level Builder is the canonical map authoring surface.";
    }

    last_render_snapshot_.palette = palette_panel_.lastRenderSnapshot();
    last_render_snapshot_.placement = placement_panel_.lastRenderSnapshot();
    last_render_snapshot_.inspector = inspector_panel_.lastRenderSnapshot();
    last_render_snapshot_.playtest = playtest_panel_.lastRenderSnapshot();
    last_render_snapshot_.supporting_spatial = supporting_spatial_workspace_.lastRenderSnapshot();
    last_render_snapshot_.supporting_spatial.visible = active_mode_ == WorkflowMode::SupportingSpatial;
    last_render_snapshot_.readiness_evidence.has_player_spawn = readiness_evidence_.has_player_spawn;
    last_render_snapshot_.readiness_evidence.has_objective = readiness_evidence_.has_objective;
    last_render_snapshot_.readiness_evidence.reachability_passed = readiness_evidence_.reachability_passed;
    last_render_snapshot_.readiness_evidence.target_export_checks_passed =
        readiness_evidence_.target_export_checks_passed;
    last_render_snapshot_.readiness_evidence.accessibility_checks_passed =
        readiness_evidence_.accessibility_checks_passed;
    last_render_snapshot_.readiness_evidence.performance_budget_passed =
        readiness_evidence_.performance_budget_passed;
    last_render_snapshot_.readiness_evidence.human_review_required = readiness_evidence_.human_review_required;
    last_render_snapshot_.readiness_evidence.human_review_passed = readiness_evidence_.human_review_passed;

    if (document_ != nullptr && catalog_ != nullptr) {
        const auto document_validation = urpg::map::ValidateGridPartDocument(*document_, *catalog_);
        const auto ruleset_validation = urpg::map::ValidateGridPartRuleset(*document_, *catalog_, ruleset_);
        const auto objective_validation = urpg::map::ValidateMapObjective(*document_, *catalog_, objective_);

        last_render_snapshot_.validation.document_ok = document_validation.ok;
        last_render_snapshot_.validation.ruleset_ok = ruleset_validation.ok;
        last_render_snapshot_.validation.objective_ok = objective_validation.ok;
        last_render_snapshot_.validation.diagnostic_count = document_validation.diagnostics.size() +
                                                            ruleset_validation.diagnostics.size() +
                                                            objective_validation.diagnostics.size();
        last_render_snapshot_.validation.blocking_count =
            countBlockingDiagnostics(document_validation.diagnostics) +
            countBlockingDiagnostics(ruleset_validation.diagnostics) +
            countBlockingDiagnostics(objective_validation.diagnostics);
        appendDiagnosticSummaries(last_render_snapshot_.diagnostics, "document", document_validation.diagnostics);
        appendDiagnosticSummaries(last_render_snapshot_.diagnostics, "ruleset", ruleset_validation.diagnostics);
        appendDiagnosticSummaries(last_render_snapshot_.diagnostics, "objective", objective_validation.diagnostics);

        const auto manifest = has_custom_package_manifest_
                                  ? package_manifest_
                                  : urpg::map::BuildGridPartPackageManifest(*document_, *catalog_);
        const auto package = urpg::map::ValidateGridPartPackageGovernance(*document_, *catalog_, manifest,
                                                                          readiness_evidence_);
        last_render_snapshot_.package.readiness = readinessName(package.readiness);
        last_render_snapshot_.package.can_publish = package.canPublish();
        last_render_snapshot_.package.can_export = package.canExport();
        last_render_snapshot_.package.dependency_count = package.dependencies.size();
        last_render_snapshot_.package.diagnostic_count = package.diagnostics.size();
        appendDiagnosticSummaries(last_render_snapshot_.diagnostics, "package", package.diagnostics);
        last_render_snapshot_.can_export_current_level =
            last_render_snapshot_.validation.blocking_count == 0 && package.canExport();
    }

    last_render_snapshot_.actions = {
        {"build", "Build", active_mode_ == WorkflowMode::Build, last_render_snapshot_.has_document},
        {"undo", "Undo", false, last_render_snapshot_.can_undo},
        {"redo", "Redo", false, last_render_snapshot_.can_redo},
        {"mark_player_spawn", "Spawn", false, last_render_snapshot_.inspector.has_selection},
        {"set_reach_exit_objective", "Objective", false, last_render_snapshot_.inspector.has_selection},
        {"mark_target_export_checks_passed", "Export Checks", false, last_render_snapshot_.has_document},
        {"mark_accessibility_checks_passed", "Accessibility", false, last_render_snapshot_.has_document},
        {"mark_performance_budget_passed", "Performance", false, last_render_snapshot_.has_document},
        {"mark_human_review_passed", "Review", false, last_render_snapshot_.has_document},
        {"validate", "Validate", active_mode_ == WorkflowMode::Validate, last_render_snapshot_.has_document},
        {"playtest", "Playtest", active_mode_ == WorkflowMode::Playtest, last_render_snapshot_.can_playtest},
        {"playtest_start", "Run", false, last_render_snapshot_.can_playtest},
        {"playtest_return", "Return", false, last_render_snapshot_.playtest.running},
        {"package", "Package", active_mode_ == WorkflowMode::Package, last_render_snapshot_.has_document},
        {"save_level_draft", "Save", false, last_render_snapshot_.has_document},
        {"export_current_level", "Export", false, last_render_snapshot_.can_export_current_level},
        {"supporting_spatial", "Supporting Spatial", active_mode_ == WorkflowMode::SupportingSpatial,
         last_render_snapshot_.has_spatial_overlay || last_render_snapshot_.has_target_scene},
        {"supporting_elevation", "Elevation", active_mode_ == WorkflowMode::SupportingSpatial &&
                                               supporting_spatial_workspace_.activeMode() ==
                                                   SpatialAuthoringWorkspace::ToolMode::Elevation,
         last_render_snapshot_.has_spatial_overlay},
        {"supporting_props", "Props", active_mode_ == WorkflowMode::SupportingSpatial &&
                                      supporting_spatial_workspace_.activeMode() ==
                                          SpatialAuthoringWorkspace::ToolMode::Props,
         last_render_snapshot_.has_spatial_overlay},
        {"supporting_abilities", "Abilities", active_mode_ == WorkflowMode::SupportingSpatial &&
                                              supporting_spatial_workspace_.activeMode() ==
                                                  SpatialAuthoringWorkspace::ToolMode::Abilities,
         last_render_snapshot_.has_spatial_overlay || last_render_snapshot_.has_target_scene},
    };
}

} // namespace urpg::editor
