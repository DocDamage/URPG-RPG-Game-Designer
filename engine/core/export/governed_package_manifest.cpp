#include "engine/core/export/governed_package_manifest.h"

#include "engine/core/security/release_security_audit.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <tuple>
#include <utility>

namespace urpg::exporting {
namespace {

bool validHash(const std::string& value) {
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](const unsigned char character) {
        return std::isxdigit(character) != 0;
    });
}

} // namespace

GovernedPackageManifestResult GovernedPackageManifestBuilder::build(GovernedPackageInput input) const {
    GovernedPackageManifestResult result;
    if (input.package_id.empty()) result.diagnostics.push_back("package_manifest_id_missing");
    if (input.version.empty()) result.diagnostics.push_back("package_manifest_version_missing");
    if (input.platform_capabilities.empty()) result.diagnostics.push_back("package_manifest_capabilities_missing");
    if (input.reproducibility_inputs.empty()) result.diagnostics.push_back("package_manifest_reproducibility_missing");
    std::sort(input.platform_capabilities.begin(), input.platform_capabilities.end());
    if (std::adjacent_find(input.platform_capabilities.begin(), input.platform_capabilities.end()) != input.platform_capabilities.end()) {
        result.diagnostics.push_back("package_manifest_capability_duplicate");
    }
    std::stable_sort(input.files.begin(), input.files.end(), [](const auto& left, const auto& right) {
        return left.path < right.path;
    });
    std::set<std::string> paths;
    nlohmann::json files = nlohmann::json::array();
    for (auto& file : input.files) {
        if (urpg::security::ReleaseSecurityAudit::isUnsafeRelativePath(file.path) || !paths.insert(file.path).second) {
            result.diagnostics.push_back("package_manifest_path_invalid_or_duplicate:" + file.path);
        }
        if (!validHash(file.sha256)) result.diagnostics.push_back("package_manifest_hash_invalid:" + file.path);
        if (file.license_id.empty()) result.diagnostics.push_back("package_manifest_license_missing:" + file.path);
        if (file.notice_id.empty()) result.diagnostics.push_back("package_manifest_notice_missing:" + file.path);
        std::sort(file.references.begin(), file.references.end());
        if (std::adjacent_find(file.references.begin(), file.references.end()) != file.references.end()) {
            result.diagnostics.push_back("package_manifest_reference_duplicate:" + file.path);
        }
        files.push_back({{"path", file.path}, {"sha256", file.sha256}, {"references", file.references},
                         {"license_id", file.license_id}, {"notice_id", file.notice_id}});
    }
    result.valid = result.diagnostics.empty();
    result.manifest = {{"schema", "urpg.governed_package_manifest.v1"}, {"package_id", input.package_id},
                       {"version", input.version}, {"valid", result.valid}, {"files", std::move(files)},
                       {"platform_capabilities", input.platform_capabilities},
                       {"reproducibility_inputs", input.reproducibility_inputs},
                       {"diagnostics", result.diagnostics}};
    result.canonical_json = result.manifest.dump();
    return result;
}

std::vector<std::string> GovernedPackageManifestBuilder::explainDifference(
    const nlohmann::json& left, const nlohmann::json& right) const {
    std::vector<std::string> differences;
    for (const auto* key : {"package_id", "version", "files", "platform_capabilities", "reproducibility_inputs"}) {
        if (left.value(key, nlohmann::json{}) != right.value(key, nlohmann::json{})) differences.push_back(key);
    }
    return differences;
}

} // namespace urpg::exporting
