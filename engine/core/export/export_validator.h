#pragma once

#include <string>
#include <vector>
#include <optional>

#include <nlohmann/json.hpp>

#include "engine/core/tools/export_packager.h"

namespace urpg::exporting {

struct PlatformRequirement {
    std::string filePattern;
    bool required;
    std::string description;
};

class ExportValidator {
public:
    std::vector<std::string> validateExportDirectory(const std::string& path, tools::ExportTarget target) const;
    std::vector<PlatformRequirement> getRequirementsForTarget(tools::ExportTarget target) const;
    nlohmann::json buildReportJson(const std::vector<std::string>& errors, tools::ExportTarget target) const;
    nlohmann::json buildReportJson(const std::string& path, tools::ExportTarget target) const;

private:
    bool checkPatternExists(const std::filesystem::path& dir, const std::string& pattern) const;
    bool checkAnyAppDirectory(const std::filesystem::path& dir) const;
    bool checkAnyExecutableWithoutExtension(const std::filesystem::path& dir) const;
    std::vector<std::string> validateBundleIntegrity(const std::filesystem::path& bundlePath,
                                                     tools::ExportTarget target) const;
    std::optional<nlohmann::json> inspectBundleSummary(const std::filesystem::path& bundlePath,
                                                       tools::ExportTarget target) const;
    std::string targetToString(tools::ExportTarget target) const;
};

} // namespace urpg::exporting
