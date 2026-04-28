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
    return document;
}

ExportPreviewResult RunExportPreview(const ExportPreviewDocument& document,
                                     const std::filesystem::path& workspace_root) {
    ExportPreviewResult result;
    result.diagnostics = document.validate();
    const auto config = document.toConfig(workspace_root);
    result.output_dir = config.outputDir;
    if (!result.diagnostics.empty()) {
        return result;
    }

    tools::ExportPackager packager;
    ExportValidator validator;
    const auto preflight = packager.validateBeforeExport(config);
    result.preflight_passed = preflight.passed;
    for (const auto& error : preflight.errors) {
        result.diagnostics.push_back({"preflight_failed", error, document.id});
    }
    if (!preflight.passed) {
        return result;
    }

    const auto export_result = packager.runExport(config);
    result.export_success = export_result.success;
    result.generated_files = export_result.generatedFiles;
    if (!export_result.success) {
        result.diagnostics.push_back({"export_failed", export_result.log, document.id});
        return result;
    }

    const auto post_errors = validator.validateExportDirectory(config.outputDir, config.target);
    result.post_export_validation_passed = post_errors.empty();
    for (const auto& error : post_errors) {
        result.diagnostics.push_back({"post_export_validation_failed", error, document.id});
    }

    result.emitted_artifacts = listFiles(config.outputDir);
    result.exact_ship_preview = result.export_success && result.post_export_validation_passed &&
                                config.mode == tools::ExportMode::Release;
    if (config.mode != tools::ExportMode::Release) {
        result.diagnostics.push_back({"dev_bootstrap_not_release",
                                      "Dev bootstrap output is not an exact release shipping preview.", document.id});
    }

    result.shipping_manifest = {
        {"schema", "urpg.export_preview_manifest.v1"},
        {"preview_id", document.id},
        {"target", ExportPreviewTargetLabel(config.target)},
        {"mode", ExportPreviewModeLabel(config.mode)},
        {"output_dir", config.outputDir},
        {"exact_ship_preview", result.exact_ship_preview},
        {"generated_files", result.generated_files},
        {"emitted_artifacts", result.emitted_artifacts},
        {"post_export_validation_passed", result.post_export_validation_passed},
    };
    return result;
}

} // namespace urpg::exporting
