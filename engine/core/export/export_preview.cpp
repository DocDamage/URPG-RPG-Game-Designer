#include "engine/core/export/export_preview.h"

#include <algorithm>

namespace urpg::exporting {
namespace {

tools::ExportTarget targetFromString(const std::string& value) {
    if (value == "Linux_x64") {
        return tools::ExportTarget::Linux_x64;
    }
    if (value == "macOS_Universal") {
        return tools::ExportTarget::macOS_Universal;
    }
    if (value == "Web_WASM") {
        return tools::ExportTarget::Web_WASM;
    }
    return tools::ExportTarget::Windows_x64;
}

tools::ExportMode modeFromString(const std::string& value) {
    return value == "release" ? tools::ExportMode::Release : tools::ExportMode::DevBootstrap;
}

std::vector<std::string> stringArray(const nlohmann::json& json) {
    std::vector<std::string> values;
    if (!json.is_array()) {
        return values;
    }
    for (const auto& value : json) {
        if (value.is_string()) {
            values.push_back(value.get<std::string>());
        }
    }
    return values;
}

std::vector<std::string> listFiles(const std::filesystem::path& root) {
    std::vector<std::string> files;
    std::error_code ec;
    if (!std::filesystem::exists(root, ec) || ec) {
        return files;
    }
    for (std::filesystem::recursive_directory_iterator it(root, ec), end; !ec && it != end; it.increment(ec)) {
        if (!it->is_regular_file()) {
            continue;
        }
        auto relative = std::filesystem::relative(it->path(), root, ec);
        if (ec) {
            ec.clear();
            relative = it->path().filename();
        }
        files.push_back(relative.generic_string());
    }
    std::sort(files.begin(), files.end());
    return files;
}

nlohmann::json checklistItem(const std::string& id,
                             const std::string& label,
                             bool passed,
                             const std::string& detail) {
    return {{"id", id}, {"label", label}, {"passed", passed}, {"detail", detail}};
}

nlohmann::json diagnosticsToJson(const std::vector<ExportPreviewDiagnostic>& diagnostics) {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& item : diagnostics) {
        out.push_back({{"code", item.code}, {"message", item.message}, {"target", item.target}});
    }
    return out;
}

nlohmann::json buildMissingAssetReport(const ExportPreviewDocument& document,
                                       const ExportPreviewResult& result) {
    nlohmann::json missingRoots = nlohmann::json::array();
    for (const auto& root : document.asset_discovery_roots) {
        std::error_code ec;
        if (!std::filesystem::exists(root, ec) || ec) {
            missingRoots.push_back(root);
        }
    }
    return {
        {"schema", "urpg.export_missing_asset_report.v1"},
        {"asset_discovery_root_count", document.asset_discovery_roots.size()},
        {"missing_asset_discovery_roots", missingRoots},
        {"missing_expected_artifacts", result.missing_expected_artifacts},
        {"has_missing_assets_or_artifacts", !missingRoots.empty() || !result.missing_expected_artifacts.empty()},
    };
}

nlohmann::json buildSigningStatus(const ExportPreviewDocument& document) {
    return {
        {"schema", "urpg.export_signing_status.v1"},
        {"target", ExportPreviewTargetLabel(document.target)},
        {"signing_required_for_public_release", document.mode == tools::ExportMode::Release},
        {"native_signing_configured", false},
        {"notarization_configured", false},
        {"status", document.mode == tools::ExportMode::Release ? "not_configured" : "not_required_for_dev_bootstrap"},
        {"detail", "Runtime bundle integrity is validated separately; platform signing/notarization remains explicit release backlog."},
    };
}

nlohmann::json buildSmokeEvidence(const ExportPreviewDocument& document,
                                  const ExportPreviewResult& result) {
    const bool executablePresent = std::find(result.emitted_artifacts.begin(), result.emitted_artifacts.end(),
                                            document.target == tools::ExportTarget::Windows_x64 ? "game.exe" : "game") !=
                                   result.emitted_artifacts.end();
    return {
        {"schema", "urpg.export_smoke_evidence.v1"},
        {"target", ExportPreviewTargetLabel(document.target)},
        {"mode", ExportPreviewModeLabel(document.mode)},
        {"staged_runtime_present", executablePresent},
        {"post_export_validation_passed", result.post_export_validation_passed},
        {"playable_smoke_status", executablePresent && result.post_export_validation_passed ? "staged_not_launched" : "not_ready"},
        {"detail", executablePresent ? "Runtime artifact is staged for smoke launch."
                                      : "No runnable runtime artifact was staged for this preview."},
    };
}

nlohmann::json buildPlatformChecklist(const ExportPreviewDocument& document,
                                      const ExportPreviewResult& result) {
    const bool nativeTarget = document.target != tools::ExportTarget::Web_WASM;
    const bool runtimeConfigured = !document.runtime_binary_path.empty();
    const bool noMissingArtifacts = result.missing_expected_artifacts.empty();
    nlohmann::json checklist = nlohmann::json::array();
    checklist.push_back(checklistItem("output_dir", "Output directory",
                                      !result.output_dir.empty(), result.output_dir));
    checklist.push_back(checklistItem("runtime_binary", "Runtime binary",
                                      !nativeTarget || runtimeConfigured,
                                      nativeTarget ? (runtimeConfigured ? document.runtime_binary_path
                                                                        : "Missing runtime binary path.")
                                                   : "Web runtime artifact is not part of this native checklist."));
    checklist.push_back(checklistItem("preflight", "Packager preflight", result.preflight_passed,
                                      result.preflight_passed ? "Preflight passed." : "Preflight failed."));
    checklist.push_back(checklistItem("export", "Export execution", result.export_success,
                                      result.export_success ? "Export completed." : "Export did not complete."));
    checklist.push_back(checklistItem("post_validation", "Post-export validation",
                                      result.post_export_validation_passed,
                                      result.post_export_validation_passed ? "Export directory validated."
                                                                           : "Export directory validation failed."));
    checklist.push_back(checklistItem("expected_artifacts", "Expected artifacts",
                                      noMissingArtifacts, noMissingArtifacts ? "All expected artifacts were emitted."
                                                                            : "Expected artifacts are missing."));
    checklist.push_back(checklistItem("platform_signing", "Platform signing/notarization",
                                      document.mode != tools::ExportMode::Release,
                                      document.mode == tools::ExportMode::Release
                                          ? "Not configured; required before public release."
                                          : "Not required for dev bootstrap preview."));
    return checklist;
}

} // namespace

std::string ExportPreviewTargetLabel(tools::ExportTarget target) {
    switch (target) {
    case tools::ExportTarget::Windows_x64:
        return "Windows_x64";
    case tools::ExportTarget::Linux_x64:
        return "Linux_x64";
    case tools::ExportTarget::macOS_Universal:
        return "macOS_Universal";
    case tools::ExportTarget::Web_WASM:
        return "Web_WASM";
    }
    return "Windows_x64";
}

std::string ExportPreviewModeLabel(tools::ExportMode mode) {
    return mode == tools::ExportMode::Release ? "release" : "dev_bootstrap";
}

std::vector<ExportPreviewDiagnostic> ExportPreviewDocument::validate() const {
    std::vector<ExportPreviewDiagnostic> diagnostics;
    if (id.empty()) {
        diagnostics.push_back({"missing_preview_id", "Export preview requires an id.", ""});
    }
    if (mode == tools::ExportMode::Release) {
        if (target == tools::ExportTarget::Web_WASM) {
            diagnostics.push_back({"release_web_not_supported", "Release Web export requires a real Web runtime artifact.",
                                   id});
        }
        if (target != tools::ExportTarget::Web_WASM && runtime_binary_path.empty()) {
            diagnostics.push_back({"missing_runtime_binary", "Release native export requires runtime_binary_path.", id});
        }
    }
    return diagnostics;
}

tools::ExportConfig ExportPreviewDocument::toConfig(const std::filesystem::path& workspace_root) const {
    tools::ExportConfig config;
    config.target = target;
    config.mode = mode;
    config.outputDir = output_dir.empty() ? (workspace_root / id).string() : output_dir;
    config.runtimeBinaryPath = runtime_binary_path;
    config.compressAssets = compress_assets;
    config.obfuscateScripts = obfuscate_scripts;
    config.includeDebugSymbols = include_debug_symbols;
    config.assetDiscoveryRoots = asset_discovery_roots;
    return config;
}

nlohmann::json ExportPreviewDocument::toJson() const {
    return {
        {"schema", "urpg.export_preview.v1"},
        {"id", id},
        {"target", ExportPreviewTargetLabel(target)},
        {"mode", ExportPreviewModeLabel(mode)},
        {"output_dir", output_dir},
        {"runtime_binary_path", runtime_binary_path},
        {"compress_assets", compress_assets},
        {"obfuscate_scripts", obfuscate_scripts},
        {"include_debug_symbols", include_debug_symbols},
        {"asset_discovery_roots", asset_discovery_roots},
        {"expected_artifacts", expected_artifacts},
    };
}

ExportPreviewDocument ExportPreviewDocument::fromJson(const nlohmann::json& json) {
    ExportPreviewDocument document;
    if (!json.is_object()) {
        return document;
    }
    document.id = json.value("id", "");
    document.target = targetFromString(json.value("target", "Windows_x64"));
    document.mode = modeFromString(json.value("mode", "dev_bootstrap"));
    document.output_dir = json.value("output_dir", "");
    document.runtime_binary_path = json.value("runtime_binary_path", "");
    document.compress_assets = json.value("compress_assets", true);
    document.obfuscate_scripts = json.value("obfuscate_scripts", false);
    document.include_debug_symbols = json.value("include_debug_symbols", false);
    document.asset_discovery_roots = stringArray(json.value("asset_discovery_roots", nlohmann::json::array()));
    document.expected_artifacts = stringArray(json.value("expected_artifacts", nlohmann::json::array()));
    return document;
}

ExportPreviewResult RunExportPreview(const ExportPreviewDocument& document,
                                     const std::filesystem::path& workspace_root) {
    ExportPreviewResult result;
    result.diagnostics = document.validate();
    const auto config = document.toConfig(workspace_root);
    result.output_dir = config.outputDir;
    result.runtime_trace.push_back("target:" + ExportPreviewTargetLabel(config.target));
    result.runtime_trace.push_back("mode:" + ExportPreviewModeLabel(config.mode));
    if (!result.diagnostics.empty()) {
        result.platform_checklist = buildPlatformChecklist(document, result);
        result.missing_asset_report = buildMissingAssetReport(document, result);
        result.signing_status = buildSigningStatus(document);
        result.smoke_test_evidence = buildSmokeEvidence(document, result);
        result.packaging_diagnostics = diagnosticsToJson(result.diagnostics);
        return result;
    }

    tools::ExportPackager packager;
    ExportValidator validator;
    const auto preflight = packager.validateBeforeExport(config);
    result.preflight_passed = preflight.passed;
    result.runtime_trace.push_back(std::string("preflight:") + (preflight.passed ? "passed" : "failed"));
    for (const auto& error : preflight.errors) {
        result.diagnostics.push_back({"preflight_failed", error, document.id});
    }
    if (!preflight.passed) {
        result.platform_checklist = buildPlatformChecklist(document, result);
        result.missing_asset_report = buildMissingAssetReport(document, result);
        result.signing_status = buildSigningStatus(document);
        result.smoke_test_evidence = buildSmokeEvidence(document, result);
        result.packaging_diagnostics = diagnosticsToJson(result.diagnostics);
        return result;
    }

    const auto export_result = packager.runExport(config);
    result.export_success = export_result.success;
    result.runtime_trace.push_back(std::string("export:") + (export_result.success ? "success" : "failed"));
    result.generated_files = export_result.generatedFiles;
    if (!export_result.success) {
        result.diagnostics.push_back({"export_failed", export_result.log, document.id});
        result.platform_checklist = buildPlatformChecklist(document, result);
        result.missing_asset_report = buildMissingAssetReport(document, result);
        result.signing_status = buildSigningStatus(document);
        result.smoke_test_evidence = buildSmokeEvidence(document, result);
        result.packaging_diagnostics = diagnosticsToJson(result.diagnostics);
        return result;
    }

    const auto post_errors = validator.validateExportDirectory(config.outputDir, config.target);
    result.post_export_validation_passed = post_errors.empty();
    result.runtime_trace.push_back(std::string("post_validate:") + (post_errors.empty() ? "passed" : "failed"));
    for (const auto& error : post_errors) {
        result.diagnostics.push_back({"post_export_validation_failed", error, document.id});
    }

    result.emitted_artifacts = listFiles(config.outputDir);
    for (const auto& expected : document.expected_artifacts) {
        if (std::find(result.emitted_artifacts.begin(), result.emitted_artifacts.end(), expected) ==
            result.emitted_artifacts.end()) {
            result.missing_expected_artifacts.push_back(expected);
            result.diagnostics.push_back({"missing_expected_artifact", "Expected shipping artifact was not emitted: " + expected,
                                          document.id});
        }
    }
    result.exact_ship_preview = result.export_success && result.post_export_validation_passed &&
                                result.missing_expected_artifacts.empty() && config.mode == tools::ExportMode::Release;
    if (config.mode != tools::ExportMode::Release) {
        result.diagnostics.push_back({"dev_bootstrap_not_release",
                                      "Dev bootstrap output is not an exact release shipping preview.", document.id});
    }
    result.platform_checklist = buildPlatformChecklist(document, result);
    result.missing_asset_report = buildMissingAssetReport(document, result);
    result.signing_status = buildSigningStatus(document);
    result.smoke_test_evidence = buildSmokeEvidence(document, result);
    result.packaging_diagnostics = diagnosticsToJson(result.diagnostics);

    result.shipping_manifest = {
        {"schema", "urpg.export_preview_manifest.v1"},
        {"preview_id", document.id},
        {"target", ExportPreviewTargetLabel(config.target)},
        {"mode", ExportPreviewModeLabel(config.mode)},
        {"output_dir", config.outputDir},
        {"exact_ship_preview", result.exact_ship_preview},
        {"generated_files", result.generated_files},
        {"emitted_artifacts", result.emitted_artifacts},
        {"expected_artifacts", document.expected_artifacts},
        {"missing_expected_artifacts", result.missing_expected_artifacts},
        {"runtime_trace", result.runtime_trace},
        {"post_export_validation_passed", result.post_export_validation_passed},
        {"platform_checklist", result.platform_checklist},
        {"missing_asset_report", result.missing_asset_report},
        {"packaging_diagnostics", result.packaging_diagnostics},
        {"signing_status", result.signing_status},
        {"smoke_test_evidence", result.smoke_test_evidence},
    };
    return result;
}

} // namespace urpg::exporting
