#pragma once

#include "engine/core/export/export_preview.h"

#include <filesystem>
#include <string>

namespace urpg::editor {

struct ExportPreviewPanelSnapshot {
    bool visible = true;
    bool rendered = false;
    bool disabled = true;
    std::string preview_id;
    std::string target;
    std::string mode;
    std::string output_dir;
    bool preflight_passed = false;
    bool export_success = false;
    bool post_export_validation_passed = false;
    bool exact_ship_preview = false;
    size_t generated_file_count = 0;
    size_t emitted_artifact_count = 0;
    size_t diagnostic_count = 0;
    std::string saved_project_json;
    nlohmann::json shipping_manifest = nlohmann::json::object();
    std::string status_message = "Load an export preview before rendering this panel.";
};

class ExportPreviewPanel {
public:
    void loadDocument(urpg::exporting::ExportPreviewDocument document, std::filesystem::path workspace_root);
    void render();

    const ExportPreviewPanelSnapshot& snapshot() const { return snapshot_; }
    const urpg::exporting::ExportPreviewResult& result() const { return result_; }
    bool hasRenderedFrame() const { return snapshot_.rendered; }

private:
    void refreshPreview();

    urpg::exporting::ExportPreviewDocument document_;
    std::filesystem::path workspace_root_;
    urpg::exporting::ExportPreviewResult result_;
    ExportPreviewPanelSnapshot snapshot_{};
    bool loaded_ = false;
};

} // namespace urpg::editor
