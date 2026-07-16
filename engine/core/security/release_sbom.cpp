#include "engine/core/security/release_sbom.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <set>
#include <sstream>
#include <tuple>
#include <utility>

namespace urpg::security {
namespace {

constexpr std::array<SbomComponentClass, 5> kRequiredClasses = {
    SbomComponentClass::Binary, SbomComponentClass::Tool, SbomComponentClass::PackagedAsset,
    SbomComponentClass::OptionalProvider, SbomComponentClass::ResearchOutput};

bool validSha256(const std::string& value) {
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](const unsigned char character) {
        return std::isxdigit(character) != 0;
    });
}

} // namespace

const char* sbomComponentClassName(const SbomComponentClass component_class) {
    switch (component_class) {
    case SbomComponentClass::Binary: return "binary";
    case SbomComponentClass::Tool: return "tool";
    case SbomComponentClass::PackagedAsset: return "packaged_asset";
    case SbomComponentClass::OptionalProvider: return "optional_provider";
    case SbomComponentClass::ResearchOutput: return "research_output";
    }
    return "unknown";
}

ReleaseSbomResult ReleaseSbomBuilder::build(std::string release_version, std::vector<SbomComponent> components) const {
    ReleaseSbomResult result;
    if (release_version.empty()) result.diagnostics.push_back("sbom_release_version_missing");
    std::stable_sort(components.begin(), components.end(), [](const auto& left, const auto& right) {
        return std::tie(left.component_class, left.id) < std::tie(right.component_class, right.id);
    });
    std::set<std::string> ids;
    std::set<SbomComponentClass> covered;
    nlohmann::json packages = nlohmann::json::array();
    std::ostringstream notices;
    notices << "URPG Release Dependency and Asset Notices\n";
    for (const auto& component : components) {
        const auto componentClass = std::string(sbomComponentClassName(component.component_class));
        covered.insert(component.component_class);
        if (component.id.empty() || component.name.empty() || component.version.empty() || !ids.insert(component.id).second) {
            result.diagnostics.push_back("sbom_component_invalid_or_duplicate:" + component.id);
        }
        if (component.source.empty()) result.diagnostics.push_back("sbom_source_missing:" + component.id);
        if (component.license_id.empty()) result.diagnostics.push_back("sbom_license_missing:" + component.id);
        if (component.included_in_release && !validSha256(component.sha256)) {
            result.diagnostics.push_back("sbom_release_hash_invalid:" + component.id);
        }
        if (component.included_in_release && !component.redistribution_approved) {
            result.diagnostics.push_back("sbom_redistribution_unapproved:" + component.id);
        }
        packages.push_back({{"SPDXID", "SPDXRef-" + component.id}, {"name", component.name},
                            {"versionInfo", component.version}, {"componentClass", componentClass},
                            {"downloadLocation", component.source}, {"licenseConcluded", component.license_id},
                            {"sha256", component.sha256}, {"includedInRelease", component.included_in_release},
                            {"redistributionApproved", component.redistribution_approved},
                            {"optional", component.optional}});
        notices << "\n" << component.name << " " << component.version << " [" << componentClass << "]\n"
                << "License: " << component.license_id << "\nSource: " << component.source << "\n";
        if (!component.notice.empty()) notices << component.notice << "\n";
    }
    for (const auto componentClass : kRequiredClasses) {
        if (!covered.contains(componentClass)) {
            result.diagnostics.push_back(std::string("sbom_component_class_missing:") + sbomComponentClassName(componentClass));
        }
    }
    result.complete = covered.size() == kRequiredClasses.size();
    result.release_allowed = result.complete && result.diagnostics.empty();
    result.sbom = {{"spdxVersion", "SPDX-2.3"}, {"dataLicense", "CC0-1.0"},
                   {"SPDXID", "SPDXRef-DOCUMENT"}, {"name", "URPG-" + release_version},
                   {"documentNamespace", "https://urpg.invalid/sbom/" + release_version},
                   {"releaseAllowed", result.release_allowed}, {"packages", std::move(packages)},
                   {"diagnostics", result.diagnostics}};
    result.notices = notices.str();
    return result;
}

} // namespace urpg::security
