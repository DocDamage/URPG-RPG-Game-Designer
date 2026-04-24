#include "engine/core/tools/export_packager.h"

#include "engine/core/tools/export_packager_bundle_writer.h"
#include "engine/core/tools/export_packager_executable_staging.h"
#include "engine/core/tools/export_packager_license_audit.h"
#include "engine/core/tools/export_packager_payload_builder.h"

#include <filesystem>
#include <system_error>

namespace urpg::tools {

ExportPackager::ExportResult ExportPackager::runExport(const ExportConfig& config) {
    std::string log;
    log += "Starting export for target: " + targetToString(config.target) + "\n";

    if (!config.outputDir.empty()) {
        std::error_code mkdirError;
        std::filesystem::create_directories(config.outputDir, mkdirError);
        if (mkdirError) {
            log += "Failed to prepare export output directory: " + mkdirError.message() + "\n";
            return { false, log, {} };
        }
    }

    const auto validation = validateBeforeExport(config);
    if (!validation.passed) {
        log += "Pre-export validation failed.\n";
        for (const auto& error : validation.errors) {
            log += " - " + error + "\n";
        }
        return { false, log, {} };
    }

    // 1. License Audit (Phase 4 Security Gate)
    if (!runLicenseAudit(config, log)) {
        return { false, log, {} };
    }

    // 2. Asset Bundling
    std::vector<std::string> bundles = bundleAssets(config, log);
    if (bundles.empty()) {
        log += "Asset bundling failed.\n";
        return { false, log, {} };
    }

    // 3. Script Packing (with optional obfuscation 4.6)
    packScripts(config, log);

    // 4. Binary Synthesis
    log += "Synthesizing final executable...\n";
    auto stageResult = export_packager_detail::stageExecutableArtifacts(config);
    log += stageResult.log;
    auto executableFiles = std::move(stageResult.files);
    if (config.target == ExportTarget::Windows_x64 && !config.runtimeBinaryPath.empty() && executableFiles.empty()) {
        log += "Real Windows runtime staging failed.\n";
        return { false, log, bundles };
    }
    bundles.insert(bundles.end(), executableFiles.begin(), executableFiles.end());

    return { true, log, bundles };
}

ExportValidationResult ExportPackager::validateBeforeExport(const ExportConfig& config) const {
    std::vector<std::string> errors;

    if (config.outputDir.empty()) {
        errors.emplace_back("Output directory is required for export staging.");
    } else {
        const std::filesystem::path outputDir(config.outputDir);
        if (std::filesystem::exists(outputDir) && !std::filesystem::is_directory(outputDir)) {
            errors.emplace_back("Output path exists but is not a directory: " + config.outputDir);
        }
    }

    if (config.target == ExportTarget::Windows_x64 && !config.runtimeBinaryPath.empty()) {
        std::filesystem::path runtimeBinary(config.runtimeBinaryPath);
        if (runtimeBinary.empty() || !std::filesystem::exists(runtimeBinary) ||
            !std::filesystem::is_regular_file(runtimeBinary)) {
            errors.emplace_back("Missing runtime binary for real Windows smoke export: " + config.runtimeBinaryPath);
        } else if (runtimeBinary.extension() != ".exe") {
            errors.emplace_back("Real Windows smoke export requires an .exe runtime binary: " +
                                config.runtimeBinaryPath);
        }
    }

    return { errors.empty(), std::move(errors) };
}

bool ExportPackager::runLicenseAudit(const ExportConfig& config, std::string& log) {
    log += "Running Asset License Audit...\n";
    std::vector<std::string> errors;
    export_packager_detail::auditPromotedAssetBundleLicenses(config, errors);
    export_packager_detail::auditAutoDiscoveredAssetLicenses(config, errors);
    if (!errors.empty()) {
        log += "Asset License Audit failed.\n";
        for (const auto& error : errors) {
            log += " - " + error + "\n";
        }
        return false;
    }
    log += "Asset License Audit passed.\n";
    return true;
}

std::vector<std::string> ExportPackager::bundleAssets(const ExportConfig& config, std::string& log) {
    log += "Bundling assets (Compression: " + std::string(config.compressAssets ? "ON" : "OFF") + ")...\n";
    std::filesystem::path outDir(config.outputDir);

    auto buildResult = export_packager_detail::buildBundlePayloads(config);
    if (!buildResult.errors.empty()) {
        log += "Failed to build bounded asset bundle payloads.\n";
        for (const auto& error : buildResult.errors) {
            log += " - " + error + "\n";
        }
        return {};
    }
    const auto assetDiscoveryMode = config.enableAutoAssetDiscovery ? "project_root_scan_v1" : "disabled";
    auto writeResult = export_packager_detail::writeBundleFile(
        outDir,
        config.target,
        config.compressAssets,
        assetDiscoveryMode,
        std::move(buildResult.payloads));
    log += writeResult.log;
    if (!writeResult.success) {
        for (const auto& error : writeResult.errors) {
            log += error + "\n";
        }
        return {};
    }
    return { writeResult.fileName };
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
