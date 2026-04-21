#include "editor/export/export_diagnostics_panel.h"

#include "engine/core/export/export_validator.h"
#include "engine/core/tools/export_packager.h"

namespace urpg::editor {

void ExportDiagnosticsPanel::setExportConfig(const urpg::tools::ExportConfig& config) {
    config_ = config;
}

void ExportDiagnosticsPanel::render() {
    nlohmann::json snapshot;

    if (!config_.has_value()) {
        snapshot = nlohmann::json::object();
        last_render_snapshot_ = std::move(snapshot);
        return;
    }

    const auto& cfg = *config_;

    urpg::exporting::ExportValidator validator;
    urpg::tools::ExportPackager packager;
    snapshot["target"] = validator.buildReportJson({}, cfg.target)["target"];
    snapshot["outputDir"] = cfg.outputDir;

    const auto validation = packager.validateBeforeExport(cfg);

    snapshot["validationPassed"] = validation.passed;
    snapshot["errors"] = validation.errors;
    snapshot["readyToExport"] = validation.passed;
    snapshot["validationSource"] = "packager_preflight";

    last_render_snapshot_ = std::move(snapshot);
}

} // namespace urpg::editor
