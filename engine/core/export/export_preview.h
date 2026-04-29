#pragma once

#include "engine/core/export/export_validator.h"
#include "engine/core/tools/export_packager.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::exporting {

struct ExportPreviewDiagnostic {
    std::string code;
    std::string message;
    std::string target;
};

struct ExportPreviewDocument {
    std::string id;
    tools::ExportTarget target = tools::ExportTarget::Windows_x64;
    tools::ExportMode mode = tools::ExportMode::DevBootstrap;
    std::string output_dir;
    std::string runtime_binary_path;
    bool compress_assets = true;
    bool obfuscate_scripts = false;
    bool include_debug_symbols = false;
    std::vector<std::string> asset_discovery_roots;
    std::vector<std::string> expected_artifacts;

    std::vector<ExportPreviewDiagnostic> validate() const;
    tools::ExportConfig toConfig(const std::filesystem::path& workspace_root) const;
    nlohmann::json toJson() const;

    static ExportPreviewDocument fromJson(const nlohmann::json& json);
};

struct ExportPreviewResult {
    bool preflight_passed = false;
    bool export_success = false;
    bool post_export_validation_passed = false;
    bool exact_ship_preview = false;
    std::string output_dir;
    std::vector<std::string> generated_files;
    std::vector<std::string> emitted_artifacts;
    std::vector<std::string> missing_expected_artifacts;
    std::vector<std::string> runtime_trace;
    nlohmann::json shipping_manifest = nlohmann::json::object();
    nlohmann::json platform_checklist = nlohmann::json::array();
    nlohmann::json missing_asset_report = nlohmann::json::object();
    nlohmann::json packaging_diagnostics = nlohmann::json::array();
    nlohmann::json signing_status = nlohmann::json::object();
    nlohmann::json smoke_test_evidence = nlohmann::json::object();
    std::vector<ExportPreviewDiagnostic> diagnostics;
};

ExportPreviewResult RunExportPreview(const ExportPreviewDocument& document,
                                     const std::filesystem::path& workspace_root);

std::string ExportPreviewTargetLabel(tools::ExportTarget target);
std::string ExportPreviewModeLabel(tools::ExportMode mode);

} // namespace urpg::exporting
