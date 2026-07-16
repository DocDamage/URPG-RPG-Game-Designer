#pragma once

#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <vector>

namespace urpg::exporting {

struct GovernedPackageFile {
    std::string path;
    std::string sha256;
    std::vector<std::string> references;
    std::string license_id;
    std::string notice_id;
};

struct GovernedPackageInput {
    std::string package_id;
    std::string version;
    std::vector<GovernedPackageFile> files;
    std::vector<std::string> platform_capabilities;
    std::map<std::string, std::string> reproducibility_inputs;
};

struct GovernedPackageManifestResult {
    bool valid = false;
    std::vector<std::string> diagnostics;
    nlohmann::json manifest;
    std::string canonical_json;
};

class GovernedPackageManifestBuilder {
public:
    GovernedPackageManifestResult build(GovernedPackageInput input) const;
    std::vector<std::string> explainDifference(const nlohmann::json& left, const nlohmann::json& right) const;
};

} // namespace urpg::exporting
