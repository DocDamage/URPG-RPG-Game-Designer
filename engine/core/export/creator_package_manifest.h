#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::exporting {

struct CreatorPackageManifest {
    std::string id;
    std::string type;
    std::string license_evidence;
    std::string compatibility_target;
    std::vector<std::string> dependencies;
    std::string validation_summary;
};

std::vector<std::string> ValidateCreatorPackageManifest(const CreatorPackageManifest& manifest);
nlohmann::json CreatorPackageManifestToJson(const CreatorPackageManifest& manifest);

} // namespace urpg::exporting
