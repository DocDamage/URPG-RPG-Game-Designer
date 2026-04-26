#include "engine/core/export/export_validator.h"
#include "engine/core/tools/export_packager.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

namespace {

using urpg::exporting::ExportValidator;
using urpg::tools::ExportConfig;
using urpg::tools::ExportPackager;
using urpg::tools::ExportTarget;
using urpg::tools::ExportValidationResult;

struct CliOptions {
    bool help = false;
    bool json = false;
    bool preflightOnly = false;
    bool validateOnly = false;
    bool explainHardening = false;
    bool enableAutoAssetDiscovery = true;
    bool obfuscateScripts = false;
    bool compressAssets = true;
    bool includeDebugSymbols = false;
    std::optional<ExportTarget> target;
    std::filesystem::path outputDir;
    std::filesystem::path runtimeBinaryPath;
    std::filesystem::path assetBundleManifestRootOverride;
    std::filesystem::path normalizedAssetRootOverride;
    std::vector<std::filesystem::path> assetDiscoveryRoots;
};

struct ParseResult {
    bool ok = true;
    std::string error;
    CliOptions options;
};

std::string targetToString(ExportTarget target) {
    switch (target) {
    case ExportTarget::Windows_x64:
        return "Windows_x64";
    case ExportTarget::Linux_x64:
        return "Linux_x64";
    case ExportTarget::macOS_Universal:
        return "macOS_Universal";
    case ExportTarget::Web_WASM:
        return "Web_WASM";
    }
    return "Unknown";
}

std::optional<ExportTarget> parseTarget(std::string_view value) {
    if (value == "Windows_x64") {
        return ExportTarget::Windows_x64;
    }
    if (value == "Linux_x64") {
        return ExportTarget::Linux_x64;
    }
    if (value == "macOS_Universal") {
        return ExportTarget::macOS_Universal;
    }
    if (value == "Web_WASM") {
        return ExportTarget::Web_WASM;
    }
    return std::nullopt;
}

void printHelp() {
    std::cout << "urpg_pack_cli --target <target> --output <dir> [options]\n\n"
              << "Targets:\n"
              << "  Windows_x64\n"
              << "  Linux_x64\n"
              << "  macOS_Universal\n"
              << "  Web_WASM\n\n"
              << "Options:\n"
              << "  --runtime-binary <path>       Runtime binary to stage into native exports\n"
              << "  --manifest-root <path>        Manifest root for bundled assets\n"
              << "  --normalized-root <path>      Normalized asset root for bundled assets\n"
              << "  --asset-root <path>           Asset discovery root; may be repeated\n"
              << "  --no-auto-discovery           Disable default asset discovery roots\n"
              << "  --no-compress                 Disable asset bundle compression\n"
              << "  --obfuscate-scripts           Enable script obfuscation flag\n"
              << "  --include-debug-symbols       Include debug symbol flag in native exports\n"
              << "  --preflight-only              Run export preflight and stop\n"
              << "  --validate-only               Run post-export validation for an existing output\n"
              << "  --explain-hardening           Describe signed bundle and patch/package checks\n"
              << "  --json                        Emit a machine-readable JSON report\n"
              << "  --help                        Show this help text\n";
}

bool readValue(int argc, char** argv, int& index, std::string& value, std::string& error) {
    if (index + 1 >= argc) {
        error = std::string(argv[index]) + " requires a value";
        return false;
    }
    ++index;
    value = argv[index];
    return true;
}

ParseResult parseArgs(int argc, char** argv) {
    ParseResult result;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        std::string value;

        if (arg == "--help" || arg == "-h") {
            result.options.help = true;
        } else if (arg == "--json") {
            result.options.json = true;
        } else if (arg == "--preflight-only") {
            result.options.preflightOnly = true;
        } else if (arg == "--validate-only") {
            result.options.validateOnly = true;
        } else if (arg == "--explain-hardening") {
            result.options.explainHardening = true;
        } else if (arg == "--no-auto-discovery") {
            result.options.enableAutoAssetDiscovery = false;
        } else if (arg == "--no-compress") {
            result.options.compressAssets = false;
        } else if (arg == "--obfuscate-scripts") {
            result.options.obfuscateScripts = true;
        } else if (arg == "--include-debug-symbols") {
            result.options.includeDebugSymbols = true;
        } else if (arg == "--target") {
            if (!readValue(argc, argv, i, value, result.error)) {
                result.ok = false;
                return result;
            }
            result.options.target = parseTarget(value);
            if (!result.options.target.has_value()) {
                result.ok = false;
                result.error = "unknown export target: " + value;
                return result;
            }
        } else if (arg == "--output") {
            if (!readValue(argc, argv, i, value, result.error)) {
                result.ok = false;
                return result;
            }
            result.options.outputDir = value;
        } else if (arg == "--runtime-binary") {
            if (!readValue(argc, argv, i, value, result.error)) {
                result.ok = false;
                return result;
            }
            result.options.runtimeBinaryPath = value;
        } else if (arg == "--manifest-root") {
            if (!readValue(argc, argv, i, value, result.error)) {
                result.ok = false;
                return result;
            }
            result.options.assetBundleManifestRootOverride = value;
        } else if (arg == "--normalized-root") {
            if (!readValue(argc, argv, i, value, result.error)) {
                result.ok = false;
                return result;
            }
            result.options.normalizedAssetRootOverride = value;
        } else if (arg == "--asset-root") {
            if (!readValue(argc, argv, i, value, result.error)) {
                result.ok = false;
                return result;
            }
            result.options.assetDiscoveryRoots.emplace_back(value);
        } else {
            result.ok = false;
            result.error = "unknown argument: " + arg;
            return result;
        }
    }

    if (result.options.preflightOnly && result.options.validateOnly) {
        result.ok = false;
        result.error = "--preflight-only and --validate-only are mutually exclusive";
        return result;
    }

    if (!result.options.help && !result.options.explainHardening && !result.options.target.has_value()) {
        result.ok = false;
        result.error = "--target is required";
        return result;
    }

    if (!result.options.help && !result.options.explainHardening && result.options.outputDir.empty()) {
        result.ok = false;
        result.error = "--output is required";
        return result;
    }

    return result;
}

ExportConfig toExportConfig(const CliOptions& options) {
    ExportConfig config;
    config.target = *options.target;
    config.outputDir = options.outputDir.string();
    config.runtimeBinaryPath = options.runtimeBinaryPath.string();
    config.assetBundleManifestRootOverride = options.assetBundleManifestRootOverride.string();
    config.normalizedAssetRootOverride = options.normalizedAssetRootOverride.string();
    for (const auto& root : options.assetDiscoveryRoots) {
        config.assetDiscoveryRoots.push_back(root.string());
    }
    config.enableAutoAssetDiscovery = options.enableAutoAssetDiscovery;
    config.obfuscateScripts = options.obfuscateScripts;
    config.compressAssets = options.compressAssets;
    config.includeDebugSymbols = options.includeDebugSymbols;
    return config;
}

nlohmann::json preflightToJson(const ExportValidationResult& result) {
    nlohmann::json errors = nlohmann::json::array();
    for (const auto& error : result.errors) {
        errors.push_back(error);
    }

    return {
        {"passed", result.passed},
        {"errors", errors},
    };
}

nlohmann::json exportResultToJson(const ExportPackager::ExportResult& result) {
    nlohmann::json files = nlohmann::json::array();
    for (const auto& file : result.generatedFiles) {
        files.push_back(file);
    }

    return {
        {"success", result.success},
        {"log", result.log},
        {"generatedFiles", files},
    };
}

nlohmann::json hardeningReportToJson() {
    return {
        {"tool", "urpg_pack_cli"},
        {"phase", "export_hardening"},
        {"runtimeBundle",
         {
             {"signatureMode", "sha256_keyed_bundle_v1"},
             {"tamperPolicy", "reject_before_load"},
             {"publication", "temp_file_then_rename"},
         }},
        {"artifactCompare",
         {
             "changed_assets",
             "changed_schemas",
             "missing_files",
             "signature_changed",
             "manifest_changed",
         }},
        {"patchManifest",
         {
             {"tracks", {"changed_data", "changed_assets", "dependencies"}},
             {"compatibilityChecks", {"base_version", "target_version", "dependency_presence"}},
         }},
        {"creatorPackageManifest",
         {
             "id",
             "type",
             "license_evidence",
             "compatibility_target",
             "dependencies",
             "validation_summary",
         }},
    };
}

nlohmann::json baseReport(const CliOptions& options, std::string phase) {
    return {
        {"tool", "urpg_pack_cli"},
        {"phase", std::move(phase)},
        {"target", targetToString(*options.target)},
        {"outputDir", options.outputDir.generic_string()},
    };
}

void emitJson(const nlohmann::json& report) {
    std::cout << report.dump(2) << '\n';
}

void emitTextErrors(const std::vector<std::string>& errors) {
    for (const auto& error : errors) {
        std::cerr << error << '\n';
    }
}

int usageFailure(const std::string& error) {
    std::cerr << error << '\n';
    std::cerr << "Run urpg_pack_cli --help for usage.\n";
    return 1;
}

} // namespace

int main(int argc, char** argv) {
    const ParseResult parsed = parseArgs(argc, argv);
    if (!parsed.ok) {
        return usageFailure(parsed.error);
    }

    const CliOptions& options = parsed.options;
    if (options.help) {
        printHelp();
        return 0;
    }
    if (options.explainHardening) {
        const auto report = hardeningReportToJson();
        if (options.json) {
            emitJson(report);
        } else {
            std::cout << report.dump(2) << '\n';
        }
        return 0;
    }

    ExportPackager packager;
    ExportValidator validator;
    const ExportConfig config = toExportConfig(options);

    if (options.validateOnly) {
        nlohmann::json report = baseReport(options, "post_export_validation");
        nlohmann::json validation = validator.buildReportJson(options.outputDir.string(), *options.target);
        const bool passed = validation.value("passed", false);
        report["success"] = passed;
        report["postExportValidation"] = std::move(validation);

        if (options.json) {
            emitJson(report);
        } else if (!passed) {
            std::cerr << "Post-export validation failed.\n";
            for (const auto& error : report["postExportValidation"].value("errors", std::vector<std::string>{})) {
                std::cerr << error << '\n';
            }
        }

        return passed ? 0 : 3;
    }

    const ExportValidationResult preflight = packager.validateBeforeExport(config);
    if (options.preflightOnly) {
        nlohmann::json report = baseReport(options, "preflight");
        report["success"] = preflight.passed;
        report["preflight"] = preflightToJson(preflight);

        if (options.json) {
            emitJson(report);
        } else if (!preflight.passed) {
            emitTextErrors(preflight.errors);
        }

        return preflight.passed ? 0 : 2;
    }

    nlohmann::json report = baseReport(options, "export");
    report["preflight"] = preflightToJson(preflight);
    if (!preflight.passed) {
        report["success"] = false;
        report["export"] = {
            {"success", false},
            {"log", "preflight failed"},
            {"generatedFiles", nlohmann::json::array()},
        };

        if (options.json) {
            emitJson(report);
        } else {
            emitTextErrors(preflight.errors);
        }

        return 2;
    }

    const ExportPackager::ExportResult exportResult = packager.runExport(config);
    report["export"] = exportResultToJson(exportResult);
    if (!exportResult.success) {
        report["success"] = false;

        if (options.json) {
            emitJson(report);
        } else {
            std::cerr << exportResult.log << '\n';
        }

        return 2;
    }

    nlohmann::json validation = validator.buildReportJson(options.outputDir.string(), *options.target);
    const bool postExportPassed = validation.value("passed", false);
    report["postExportValidation"] = std::move(validation);
    report["success"] = postExportPassed;

    if (options.json) {
        emitJson(report);
    } else {
        std::cout << exportResult.log << '\n';
        if (!postExportPassed) {
            std::cerr << "Post-export validation failed.\n";
            for (const auto& error : report["postExportValidation"].value("errors", std::vector<std::string>{})) {
                std::cerr << error << '\n';
            }
        }
    }

    return postExportPassed ? 0 : 3;
}
