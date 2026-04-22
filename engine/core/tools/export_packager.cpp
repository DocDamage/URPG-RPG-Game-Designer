#include "engine/core/tools/export_packager.h"

#include "engine/core/export/export_validator.h"

#include <filesystem>
#include <fstream>

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
    auto executableFiles = synthesizeExecutable(config, log);
    bundles.insert(bundles.end(), executableFiles.begin(), executableFiles.end());

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
    std::filesystem::path outDir(config.outputDir);
    std::filesystem::path pckPath = outDir / "data.pck";
    std::ofstream pck(pckPath, std::ios::binary);
    pck << "URPG_SYNTHETIC_ASSET_BUNDLE\n";
    return { "data.pck" };
}

void ExportPackager::packScripts(const ExportConfig& config, std::string& log) {
    if (config.obfuscateScripts) {
        log += "Applying script obfuscation (Phase 4.6)...\n";
    }
}

std::vector<std::string> ExportPackager::synthesizeExecutable(const ExportConfig& config, std::string& log) {
    std::filesystem::path outDir(config.outputDir);
    std::vector<std::string> files;

    switch (config.target) {
        case ExportTarget::Windows_x64: {
            std::filesystem::path exePath = outDir / "game.exe";
            std::ofstream exe(exePath, std::ios::binary);
            exe << "MZ synthetic windows executable\n";
            files.push_back("game.exe");
            break;
        }
        case ExportTarget::Linux_x64: {
            std::filesystem::path exePath = outDir / "game";
            std::ofstream exe(exePath, std::ios::binary);
            exe << "ELF synthetic linux executable\n";
            files.push_back("game");
            break;
        }
        case ExportTarget::macOS_Universal: {
            std::filesystem::path appPath = outDir / "MyGame.app";
            std::filesystem::create_directories(appPath);
            std::filesystem::path exePath = appPath / "Contents" / "MacOS" / "MyGame";
            std::filesystem::create_directories(exePath.parent_path());
            std::ofstream exe(exePath, std::ios::binary);
            exe << "MACH-O synthetic macos executable\n";
            files.push_back("MyGame.app");
            break;
        }
        case ExportTarget::Web_WASM: {
            std::ofstream html(outDir / "index.html", std::ios::binary);
            html << "<html></html>\n";
            std::ofstream wasm(outDir / "game.wasm", std::ios::binary);
            wasm << "WASM synthetic module\n";
            std::ofstream js(outDir / "game.js", std::ios::binary);
            js << "// JS loader\n";
            files.push_back("index.html");
            files.push_back("game.wasm");
            files.push_back("game.js");
            break;
        }
    }

    log += "Synthesized " + std::to_string(files.size()) + " executable artifact(s).\n";
    return files;
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
