#include "engine/core/diagnostics/diagnostics_bundle_exporter.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>

namespace {

std::string normalizeGeneric(const std::filesystem::path& path) {
    auto text = path.lexically_normal().generic_string();
    std::replace(text.begin(), text.end(), '\\', '/');
    return text;
}

std::string lowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

nlohmann::json pathRecord(const std::filesystem::path& path, const std::filesystem::path& root) {
    std::error_code ec;
    const auto absolute = std::filesystem::absolute(path, ec);
    const auto relative = root.empty() ? path : std::filesystem::relative(absolute, root, ec);

    nlohmann::json record;
    record["path"] = normalizeGeneric(ec ? path : relative);
    record["exists"] = std::filesystem::exists(path);
    if (record["exists"].get<bool>() && std::filesystem::is_regular_file(path)) {
        record["bytes"] = static_cast<std::uintmax_t>(std::filesystem::file_size(path));
    }
    return record;
}

std::filesystem::path relativePathForBundle(const std::filesystem::path& path, const std::filesystem::path& root) {
    std::error_code ec;
    const auto absolute = std::filesystem::absolute(path, ec);
    const auto relative = std::filesystem::relative(absolute, root, ec);
    const auto generic = relative.generic_string();
    if (ec || relative.empty() || generic == ".." || generic.rfind("../", 0) == 0) {
        return path.filename();
    }
    return relative;
}

} // namespace

namespace urpg::diagnostics {

DiagnosticsBundleResult DiagnosticsBundleExporter::exportBundle(
    const DiagnosticsBundleInput& input,
    const std::filesystem::path& outputDirectory) const {
    DiagnosticsBundleResult result;
    std::error_code ec;
    std::filesystem::create_directories(outputDirectory, ec);
    if (ec) {
        result.warnings.push_back("Unable to create diagnostics bundle directory: " + ec.message());
        return result;
    }

    nlohmann::json manifest;
    manifest["schema"] = "urpg.diagnostics.bundle.v1";
    manifest["contains_secret_files"] = false;
    manifest["system_info"] = input.systemInfo;
    manifest["logs"] = nlohmann::json::array();
    manifest["project_audits"] = nlohmann::json::array();
    manifest["asset_reports"] = nlohmann::json::array();
    manifest["configs"] = nlohmann::json::array();
    manifest["save_metadata"] = nlohmann::json::array();
    manifest["diagnostics_snapshots"] = nlohmann::json::array();

    const auto root = input.projectRoot.empty() ? std::filesystem::current_path() : input.projectRoot;
    addPathGroup(manifest, result, "logs", input.logs, root, outputDirectory);
    addPathGroup(manifest, result, "project_audits", input.projectAudits, root, outputDirectory);
    addPathGroup(manifest, result, "asset_reports", input.assetReports, root, outputDirectory);
    addPathGroup(manifest, result, "configs", input.configs, root, outputDirectory);
    addPathGroup(manifest, result, "save_metadata", input.saveMetadata, root, outputDirectory);
    addPathGroup(manifest, result, "diagnostics_snapshots", input.diagnosticsSnapshots, root, outputDirectory);

    manifest["excluded_paths"] = nlohmann::json::array();
    for (const auto& path : result.excludedPaths) {
        manifest["excluded_paths"].push_back(normalizeGeneric(path));
    }

    result.manifestPath = outputDirectory / "diagnostics_bundle_manifest.json";
    std::ofstream out(result.manifestPath);
    if (!out.is_open()) {
        result.warnings.push_back("Unable to write diagnostics bundle manifest.");
        return result;
    }
    out << manifest.dump(2) << '\n';

    result.manifest = manifest;
    result.success = true;
    return result;
}

bool DiagnosticsBundleExporter::isExcludedPath(const std::filesystem::path& path) const {
    const auto lowered = lowerCopy(normalizeGeneric(path.filename()));
    const auto full = lowerCopy(normalizeGeneric(path));
    return lowered == ".env" || lowered.rfind(".env.", 0) == 0 || lowered.find("secret") != std::string::npos ||
           lowered.find("token") != std::string::npos || full.find("/.git/") != std::string::npos;
}

void DiagnosticsBundleExporter::addPathGroup(nlohmann::json& manifest,
                                             DiagnosticsBundleResult& result,
                                             const std::string& key,
                                             const std::vector<std::filesystem::path>& paths,
                                             const std::filesystem::path& projectRoot,
                                             const std::filesystem::path& outputDirectory) const {
    for (const auto& path : paths) {
        if (isExcludedPath(path)) {
            result.excludedPaths.push_back(path);
            continue;
        }
        auto record = pathRecord(path, projectRoot);
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            const auto relative = relativePathForBundle(path, projectRoot);
            const auto destination = outputDirectory / relative;

            std::error_code ec;
            std::filesystem::create_directories(destination.parent_path(), ec);
            if (!ec) {
                std::filesystem::copy_file(path, destination, std::filesystem::copy_options::overwrite_existing, ec);
            }

            if (ec) {
                result.warnings.push_back("Unable to copy diagnostics bundle file '" + path.string() + "': " + ec.message());
            } else {
                record["bundle_path"] = normalizeGeneric(relative);
            }
        }
        manifest[key].push_back(std::move(record));
    }
}

} // namespace urpg::diagnostics
