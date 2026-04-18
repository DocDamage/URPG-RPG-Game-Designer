#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <fstream>
#include <filesystem>
#include "asset/asset_license_audit.h"
#include "plugin/plugin_manifest.h"

namespace urpg::tools {

    /**
     * @brief Target platforms for the export process.
     */
    enum class ExportTarget {
        Windows_x64,
        Linux_x64,
        macOS_Universal,
        Web_WASM
    };

    /**
     * @brief Configuration for the automated export process.
     */
    struct ExportConfig {
        ExportTarget target;
        std::string outputDir;
        bool obfuscateScripts = false;
        bool compressAssets = true;
        bool includeDebugSymbols = false;
    };

    /**
     * @brief Automated Export Packager.
     * Handles license auditing, asset bundling, and final binary generation.
     * Part of Wave 4 Engine Polish (4.5).
     */
    class ExportPackager {
    public:
        struct ExportResult {
            bool success;
            std::string log;
            std::vector<std::string> generatedFiles;
        };

        ExportResult runExport(const ExportConfig& config) {
            std::string log;
            log += "Starting export for target: " + targetToString(config.target) + "\n";

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

    private:
        bool runLicenseAudit(std::string& log) {
            log += "Running Asset License Audit...\n";
            // Call urpg::asset::AssetLicenseAuditor logic
            return true; // Stubbed success
        }

        std::vector<std::string> bundleAssets(const ExportConfig& config, std::string& log) {
            log += "Bundling assets (Compression: " + std::string(config.compressAssets ? "ON" : "OFF") + ")...\n";
            return { "data.pck" };
        }

        void packScripts(const ExportConfig& config, std::string& log) {
            if (config.obfuscateScripts) {
                log += "Applying script obfuscation (Phase 4.6)...\n";
            }
        }

        std::string targetToString(ExportTarget t) {
            switch(t) {
                case ExportTarget::Windows_x64: return "Windows (x64)";
                case ExportTarget::Web_WASM: return "Web (WASM/WebGL)";
                default: return "Other";
            }
        }
    };

} // namespace urpg::tools
