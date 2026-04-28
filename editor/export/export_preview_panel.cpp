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
    snapshot_.diagnostic_count = result_.diagnostics.size();
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.shipping_manifest = result_.shipping_manifest;
    snapshot_.status_message =
        snapshot_.exact_ship_preview ? "Export preview is exactly what will ship." : "Export preview has diagnostics.";
}

} // namespace urpg::editor
