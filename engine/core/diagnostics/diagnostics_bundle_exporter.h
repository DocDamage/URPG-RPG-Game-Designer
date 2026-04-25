#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::diagnostics {

struct DiagnosticsBundleInput {
    std::filesystem::path projectRoot;
    std::vector<std::filesystem::path> logs;
    std::vector<std::filesystem::path> projectAudits;
    std::vector<std::filesystem::path> assetReports;
    std::vector<std::filesystem::path> configs;
    std::vector<std::filesystem::path> saveMetadata;
    std::vector<std::filesystem::path> diagnosticsSnapshots;
    nlohmann::json systemInfo = nlohmann::json::object();
};

struct DiagnosticsBundleResult {
    bool success = false;
    std::filesystem::path manifestPath;
    nlohmann::json manifest = nlohmann::json::object();
    std::vector<std::filesystem::path> excludedPaths;
    std::vector<std::string> warnings;
};

/**
 * @brief Exports a local diagnostics manifest without copying secrets into the bundle.
 */
class DiagnosticsBundleExporter {
public:
    DiagnosticsBundleResult exportBundle(const DiagnosticsBundleInput& input,
                                         const std::filesystem::path& outputDirectory) const;

    bool isExcludedPath(const std::filesystem::path& path) const;

private:
    void addPathGroup(nlohmann::json& manifest,
                      DiagnosticsBundleResult& result,
                      const std::string& key,
                      const std::vector<std::filesystem::path>& paths,
                      const std::filesystem::path& projectRoot) const;
};

} // namespace urpg::diagnostics
