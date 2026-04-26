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
        snapshot = {
            {"panel", "export_diagnostics"},
            {"status", "disabled"},
            {"disabled_reason", "No ExportConfig is set."},
            {"owner", "editor/export"},
            {"unlock_condition", "Set ExportConfig before rendering export diagnostics."},
            {"readyToExport", false},
        };
        last_render_snapshot_ = std::move(snapshot);
        return;
    }

    const auto& cfg = *config_;

    urpg::exporting::ExportValidator validator;
    urpg::tools::ExportPackager packager;
    snapshot["panel"] = "export_diagnostics";
    snapshot["status"] = "ready";
    snapshot["target"] = validator.buildReportJson(std::vector<std::string>{}, cfg.target)["target"];
    snapshot["outputDir"] = cfg.outputDir;

    const auto validation = packager.validateBeforeExport(cfg);

    snapshot["validationPassed"] = validation.passed;
    snapshot["errors"] = validation.errors;
    snapshot["readyToExport"] = validation.passed;
    snapshot["validationSource"] = "packager_preflight";

    // Post-export validation: if the output directory already contains an emitted tree,
    // validate it and surface the emitted artifacts in the snapshot.
    const auto postExportErrors = validator.validateExportDirectory(cfg.outputDir, cfg.target);
    snapshot["postExportValidationPassed"] = postExportErrors.empty();
    if (postExportErrors.empty()) {
        snapshot["emittedArtifacts"] = nlohmann::json::array();
        std::filesystem::path outDir(cfg.outputDir);
        if (std::filesystem::exists(outDir) && std::filesystem::is_directory(outDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(outDir)) {
                std::string name = entry.path().filename().string();
                if (entry.is_regular_file()) {
                    snapshot["emittedArtifacts"].push_back(name);
                } else if (entry.is_directory() && name.size() > 4 && name.substr(name.size() - 4) == ".app") {
                    snapshot["emittedArtifacts"].push_back(name);
                }
            }
        }
    }

    last_render_snapshot_ = std::move(snapshot);
}

} // namespace urpg::editor
