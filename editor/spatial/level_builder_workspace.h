#pragma once

#include "editor/spatial/grid_part_inspector_panel.h"
#include "editor/spatial/grid_part_palette_panel.h"
#include "editor/spatial/grid_part_placement_panel.h"
#include "editor/spatial/grid_part_playtest_panel.h"
#include "editor/spatial/prop_placement_panel.h"
#include "editor/spatial/spatial_authoring_workspace.h"
#include "editor/ui/editor_panel.h"
#include "engine/core/map/grid_part_objective.h"
#include "engine/core/map/grid_part_package_governance.h"
#include "engine/core/map/grid_part_ruleset.h"

#include <string>
#include <vector>

namespace urpg::scene {
class MapScene;
}

namespace urpg::presentation {
struct SpatialMapOverlay;
}

namespace urpg::editor {

class LevelBuilderWorkspace : public EditorPanel {
  public:
    enum class WorkflowMode {
        Build = 0,
        Validate = 1,
        Playtest = 2,
        Package = 3,
        SupportingSpatial = 4,
    };

    struct ToolbarAction {
        std::string id;
        std::string label;
        bool active = false;
        bool enabled = true;
    };

    struct ValidationSummary {
        bool document_ok = false;
        bool ruleset_ok = false;
        bool objective_ok = false;
        size_t diagnostic_count = 0;
        size_t blocking_count = 0;
    };

    struct DiagnosticSummary {
        std::string source;
        std::string severity;
        std::string code;
        std::string message;
        std::string instance_id;
        std::string part_id;
        std::string target;
        int32_t x = -1;
        int32_t y = -1;
        bool blocking = false;
    };

    struct PackageSummary {
        std::string readiness = "draft";
        bool can_publish = false;
        bool can_export = false;
        size_t dependency_count = 0;
        size_t diagnostic_count = 0;
    };

    struct ReadinessEvidenceSummary {
        bool has_player_spawn = false;
        bool has_objective = false;
        bool reachability_passed = false;
        bool target_export_checks_passed = false;
        bool accessibility_checks_passed = false;
        bool performance_budget_passed = false;
        bool human_review_required = false;
        bool human_review_passed = false;
    };

    struct ExportResult {
        bool success = false;
        std::string command_id = "export_current_level";
        std::string message = "Export has not run.";
        std::string map_id;
        std::string readiness = "draft";
        std::string serialized_document_json;
        size_t diagnostic_count = 0;
        std::vector<std::string> blocker_codes;
    };

    struct SaveDraftResult {
        bool success = false;
        std::string command_id = "save_level_draft";
        std::string message = "Save has not run.";
        std::string map_id;
        std::string serialized_document_json;
        size_t diagnostic_count = 0;
        size_t saved_part_count = 0;
        std::vector<std::string> blocker_codes;
    };

    struct LoadDraftResult {
        bool success = false;
        std::string command_id = "load_level_draft";
        std::string message = "Load has not run.";
        std::string map_id;
        size_t loaded_part_count = 0;
        size_t diagnostic_count = 0;
        std::vector<std::string> blocker_codes;
    };

    struct EditHistoryResult {
        bool success = false;
        std::string command_id;
        std::string message;
        std::string source;
    };

    struct FocusDiagnosticResult {
        bool success = false;
        std::string command_id = "focus_diagnostic";
        std::string message = "Diagnostic focus has not run.";
        size_t diagnostic_index = 0;
        std::string instance_id;
        std::string source;
    };

    struct AuthoringCommandResult {
        bool success = false;
        std::string command_id;
        std::string message;
        std::string instance_id;
    };

    struct RenderSnapshot {
        std::string status = "disabled";
        std::string message = "No level builder document is bound.";
        std::string active_mode = "build";
        bool visible = true;
        bool native_level_editor = true;
        bool grid_part_document_is_source_of_truth = true;
        bool legacy_spatial_tools_are_supporting = true;
        bool has_document = false;
        bool has_catalog = false;
        bool has_spatial_overlay = false;
        bool has_target_scene = false;
        bool can_author = false;
        bool can_playtest = false;
        bool can_export_current_level = false;
        bool has_unsaved_changes = false;
        bool can_undo = false;
        bool can_redo = false;
        ExportResult last_export;
        SaveDraftResult last_save;
        LoadDraftResult last_load;
        EditHistoryResult last_edit_history;
        FocusDiagnosticResult last_focus_diagnostic;
        AuthoringCommandResult last_authoring_command;
        std::vector<ToolbarAction> actions;
        GridPartPalettePanel::RenderSnapshot palette;
        GridPartPlacementPanel::RenderSnapshot placement;
        GridPartInspectorPanel::RenderSnapshot inspector;
        GridPartPlaytestPanel::RenderSnapshot playtest;
        SpatialAuthoringWorkspace::RenderSnapshot supporting_spatial;
        ValidationSummary validation;
        PackageSummary package;
        ReadinessEvidenceSummary readiness_evidence;
        std::vector<DiagnosticSummary> diagnostics;
    };

    LevelBuilderWorkspace() : EditorPanel("Level Builder") {}

    void Render(const urpg::FrameContext& context) override;

    void SetTargets(urpg::map::GridPartDocument* document, const urpg::map::GridPartCatalog* catalog,
                    urpg::presentation::SpatialMapOverlay* overlay = nullptr,
                    urpg::scene::MapScene* scene = nullptr);
    void SetProjectionSettings(const PropPlacementPanel::ScreenProjectionSettings& settings);
    void SetRulesetProfile(urpg::map::GridRulesetProfile ruleset);
    void SetObjective(urpg::map::MapObjective objective);
    void SetPackageManifest(urpg::map::GridPartPackageManifest manifest);
    void SetReadinessEvidence(urpg::map::GridPartReadinessEvidence evidence);

    void SetActiveMode(WorkflowMode mode);
    bool ActivateToolbarAction(const std::string& action_id);
    bool SelectGridPart(const std::string& part_id);
    bool RouteCanvasHover(float screen_x, float screen_y);
    bool RouteCanvasPrimaryAction(float screen_x, float screen_y);
    bool RouteCanvasSecondaryAction(float screen_x, float screen_y);
    EditHistoryResult UndoLastEdit();
    EditHistoryResult RedoLastEdit();
    FocusDiagnosticResult FocusDiagnostic(size_t diagnostic_index);
    AuthoringCommandResult MarkSelectedInstanceAsPlayerSpawn();
    AuthoringCommandResult SetSelectedInstanceAsReachExitObjective(const std::string& objective_id = "reach_exit");
    AuthoringCommandResult MarkTargetExportChecksPassed();
    AuthoringCommandResult MarkAccessibilityChecksPassed();
    AuthoringCommandResult MarkPerformanceBudgetPassed();
    AuthoringCommandResult MarkHumanReviewPassed();
    SaveDraftResult SaveLevelDraft();
    LoadDraftResult LoadLevelDraft(const std::string& serialized_document_json);
    ExportResult ExportCurrentLevel();

    WorkflowMode activeMode() const { return active_mode_; }
    GridPartPalettePanel& palettePanel() { return palette_panel_; }
    GridPartPlacementPanel& placementPanel() { return placement_panel_; }
    GridPartInspectorPanel& inspectorPanel() { return inspector_panel_; }
    GridPartPlaytestPanel& playtestPanel() { return playtest_panel_; }
    SpatialAuthoringWorkspace& supportingSpatialWorkspace() { return supporting_spatial_workspace_; }
    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    const AuthoringCommandResult& lastAuthoringCommandResult() const { return last_authoring_command_result_; }
    const FocusDiagnosticResult& lastFocusDiagnosticResult() const { return last_focus_diagnostic_result_; }
    const EditHistoryResult& lastEditHistoryResult() const { return last_edit_history_result_; }
    const LoadDraftResult& lastLoadDraftResult() const { return last_load_result_; }
    const SaveDraftResult& lastSaveDraftResult() const { return last_save_result_; }
    const ExportResult& lastExportResult() const { return last_export_result_; }

  private:
    void captureRenderSnapshot();
    void syncPanelVisibility();
    static const char* modeName(WorkflowMode mode);
    static const char* readinessName(urpg::map::GridPartReadinessLevel readiness);

    urpg::map::GridPartDocument* document_ = nullptr;
    const urpg::map::GridPartCatalog* catalog_ = nullptr;
    urpg::presentation::SpatialMapOverlay* overlay_ = nullptr;
    urpg::scene::MapScene* scene_ = nullptr;
    WorkflowMode active_mode_ = WorkflowMode::Build;

    urpg::map::GridRulesetProfile ruleset_ =
        urpg::map::MakeDefaultGridRulesetProfile(urpg::map::GridPartRuleset::TopDownJRPG);
    urpg::map::MapObjective objective_;
    urpg::map::GridPartPackageManifest package_manifest_;
    urpg::map::GridPartReadinessEvidence readiness_evidence_;
    bool has_custom_package_manifest_ = false;

    GridPartPalettePanel palette_panel_;
    GridPartPlacementPanel placement_panel_;
    GridPartInspectorPanel inspector_panel_;
    GridPartPlaytestPanel playtest_panel_;
    SpatialAuthoringWorkspace supporting_spatial_workspace_;
    AuthoringCommandResult last_authoring_command_result_;
    FocusDiagnosticResult last_focus_diagnostic_result_;
    EditHistoryResult last_edit_history_result_;
    LoadDraftResult last_load_result_;
    SaveDraftResult last_save_result_;
    ExportResult last_export_result_;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
