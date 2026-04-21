#include "editor/export/export_diagnostics_panel.h"

#include "engine/core/export/export_validator.h"

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
    snapshot["target"] = validator.buildReportJson({}, cfg.target)["target"];
    snapshot["outputDir"] = cfg.outputDir;

    auto errors = validator.validateExportDirectory(cfg.outputDir, cfg.target);

    snapshot["validationPassed"] = errors.empty();
    snapshot["errors"] = errors;
    snapshot["readyToExport"] = errors.empty();

    last_render_snapshot_ = std::move(snapshot);
}

} // namespace urpg::editor
