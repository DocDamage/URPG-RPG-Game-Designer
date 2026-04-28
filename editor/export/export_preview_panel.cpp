#include "editor/export/export_preview_panel.h"

#include <utility>

namespace urpg::editor {

void ExportPreviewPanel::loadDocument(urpg::exporting::ExportPreviewDocument document,
                                      std::filesystem::path workspace_root) {
    document_ = std::move(document);
    workspace_root_ = std::move(workspace_root);
    loaded_ = true;
    refreshPreview();
}

void ExportPreviewPanel::setMode(urpg::tools::ExportMode mode) {
    document_.mode = mode;
    if (loaded_) {
        refreshPreview();
    }
}

void ExportPreviewPanel::setTarget(urpg::tools::ExportTarget target) {
    document_.target = target;
    if (loaded_) {
        refreshPreview();
    }
}

void ExportPreviewPanel::setRuntimeBinaryPath(std::string runtime_binary_path) {
    document_.runtime_binary_path = std::move(runtime_binary_path);
    if (loaded_) {
        refreshPreview();
    }
}

void ExportPreviewPanel::setOutputDir(std::string output_dir) {
    document_.output_dir = std::move(output_dir);
    if (loaded_) {
        refreshPreview();
    }
}

void ExportPreviewPanel::setExpectedArtifacts(std::vector<std::string> expected_artifacts) {
    document_.expected_artifacts = std::move(expected_artifacts);
    if (loaded_) {
        refreshPreview();
    }
}

void ExportPreviewPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load an export preview before rendering this panel.";
        return;
    }
    refreshPreview();
}

void ExportPreviewPanel::refreshPreview() {
    result_ = urpg::exporting::RunExportPreview(document_, workspace_root_);
    snapshot_.disabled = false;
    snapshot_.preview_id = document_.id;
    snapshot_.target = urpg::exporting::ExportPreviewTargetLabel(document_.target);
    snapshot_.mode = urpg::exporting::ExportPreviewModeLabel(document_.mode);
    snapshot_.output_dir = result_.output_dir;
    snapshot_.preflight_passed = result_.preflight_passed;
    snapshot_.export_success = result_.export_success;
    snapshot_.post_export_validation_passed = result_.post_export_validation_passed;
    snapshot_.exact_ship_preview = result_.exact_ship_preview;
    snapshot_.generated_file_count = result_.generated_files.size();
    snapshot_.emitted_artifact_count = result_.emitted_artifacts.size();
    snapshot_.expected_artifact_count = document_.expected_artifacts.size();
    snapshot_.missing_expected_artifact_count = result_.missing_expected_artifacts.size();
    snapshot_.runtime_trace_count = result_.runtime_trace.size();
    snapshot_.diagnostic_count = result_.diagnostics.size();
    float passed_checks = 0.0f;
    passed_checks += snapshot_.preflight_passed ? 1.0f : 0.0f;
    passed_checks += snapshot_.export_success ? 1.0f : 0.0f;
    passed_checks += snapshot_.post_export_validation_passed ? 1.0f : 0.0f;
    passed_checks += snapshot_.missing_expected_artifact_count == 0 ? 1.0f : 0.0f;
    snapshot_.release_readiness_score = passed_checks / 4.0f;
    if (!snapshot_.preflight_passed) {
        snapshot_.blocker_summary = "Preflight failed.";
        snapshot_.ux_focus_lane = "preflight";
        snapshot_.primary_action = "Fix export preflight inputs before packaging.";
    } else if (!snapshot_.export_success) {
        snapshot_.blocker_summary = "Export execution failed.";
        snapshot_.ux_focus_lane = "export";
        snapshot_.primary_action = "Inspect export trace and generated files.";
    } else if (snapshot_.missing_expected_artifact_count > 0) {
        snapshot_.blocker_summary = "Missing expected shipping artifacts.";
        snapshot_.ux_focus_lane = "shipping_manifest";
        snapshot_.primary_action = "Add or correct expected shipping artifacts.";
    } else if (!snapshot_.post_export_validation_passed) {
        snapshot_.blocker_summary = "Post-export validation failed.";
        snapshot_.ux_focus_lane = "validation";
        snapshot_.primary_action = "Resolve post-export validation diagnostics.";
    } else {
        snapshot_.blocker_summary = "No export blockers for this preview.";
        snapshot_.ux_focus_lane = "exact_ship";
        snapshot_.primary_action = "Use the manifest as the exact shipping preview.";
    }
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.shipping_manifest = result_.shipping_manifest;
    snapshot_.status_message =
        snapshot_.exact_ship_preview ? "Export preview is exactly what will ship." : "Export preview has diagnostics.";
}

} // namespace urpg::editor
