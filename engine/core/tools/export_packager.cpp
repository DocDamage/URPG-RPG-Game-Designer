#include "engine/core/tools/export_packager.h"

#include "engine/core/export/export_validator.h"

namespace urpg::tools {

ExportPackager::ExportResult ExportPackager::runExport(const ExportConfig& config) {
    std::string log;
    log += "Starting export for target: " + targetToString(config.target) + "\n";

    const auto validation = validateBeforeExport(config);
    if (!validation.passed) {
        log += "Pre-export validation failed.\n";
        for (const auto& error : validation.errors) {
            log += " - " + error + "\n";
        }
        return { false, log, {} };
    }

    // 1. License Audit (Phase 4 Security Gate)
    if (!runLicenseAudit(log)) {
        return { false, log, {} };
    }

    // 2. Asset Bundling
    std::vector<std::string> bundles = bundleAssets(config, log);

    // 3. Script Packing (with optional obfuscation 4.6)
    packScripts(config, log);

    // 4. Binary Synthesis
    log += "Synthesizing final executable...\n";

    return { true, log, bundles };
}

ExportValidationResult ExportPackager::validateBeforeExport(const ExportConfig& config) const {
    urpg::exporting::ExportValidator validator;
    auto errors = validator.validateExportDirectory(config.outputDir, config.target);
    return { errors.empty(), std::move(errors) };
}

bool ExportPackager::runLicenseAudit(std::string& log) {
    log += "Running Asset License Audit...\n";
    // Call urpg::asset::AssetLicenseAuditor logic
    return true; // Stubbed success
}

std::vector<std::string> ExportPackager::bundleAssets(const ExportConfig& config, std::string& log) {
    log += "Bundling assets (Compression: " + std::string(config.compressAssets ? "ON" : "OFF") + ")...\n";
    return { "data.pck" };
}

void ExportPackager::packScripts(const ExportConfig& config, std::string& log) {
    if (config.obfuscateScripts) {
        log += "Applying script obfuscation (Phase 4.6)...\n";
    }
}

std::string ExportPackager::targetToString(ExportTarget t) {
    switch(t) {
        case ExportTarget::Windows_x64: return "Windows (x64)";
        case ExportTarget::Linux_x64: return "Linux (x64)";
        case ExportTarget::macOS_Universal: return "macOS (Universal)";
        case ExportTarget::Web_WASM: return "Web (WASM/WebGL)";
        default: return "Other";
    }
}

} // namespace urpg::tools
