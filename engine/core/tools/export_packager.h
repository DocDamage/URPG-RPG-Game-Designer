#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <fstream>
#include <filesystem>
// Note: license audit and plugin manifest integration are stubbed;
// this header is intentionally self-contained to avoid missing dependency headers.

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
        std::string runtimeBinaryPath;
        std::string assetBundleManifestRootOverride;
        std::string normalizedAssetRootOverride;
        bool obfuscateScripts = false;
        bool compressAssets = true;
        bool includeDebugSymbols = false;
    };

    /**
     * @brief Result of pre-export validation.
     */
    struct ExportValidationResult {
        bool passed = false;
        std::vector<std::string> errors;
    };

    /**
     * @brief Automated Export Packager.
     * Handles the bounded in-tree export contract: staged asset bundle output,
     * executable synthesis, and one real Windows launch-smoke path when a
     * runtime binary is explicitly supplied. Broader shipping-grade packaging
     * remains future work.
     * Part of Wave 4 Engine Polish (4.5).
     */
    class ExportPackager {
    public:
        struct ExportResult {
            bool success;
            std::string log;
            std::vector<std::string> generatedFiles;
        };

        ExportResult runExport(const ExportConfig& config);
        ExportValidationResult validateBeforeExport(const ExportConfig& config) const;

    private:
        bool runLicenseAudit(std::string& log);
        std::vector<std::string> bundleAssets(const ExportConfig& config, std::string& log);
        void packScripts(const ExportConfig& config, std::string& log);
        std::vector<std::string> synthesizeExecutable(const ExportConfig& config, std::string& log);
        std::vector<std::string> stageRealWindowsRuntime(const ExportConfig& config, std::string& log);
        std::string targetToString(ExportTarget t);
    };

} // namespace urpg::tools
