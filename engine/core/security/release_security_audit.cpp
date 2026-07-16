#include "engine/core/security/release_security_audit.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <set>
#include <tuple>
#include <utility>

namespace urpg::security {
namespace {

constexpr std::array<SecuritySurface, 8> kRequiredSurfaces = {
    SecuritySurface::ExternalProcess, SecuritySurface::ArchiveExtraction, SecuritySurface::PathContainment,
    SecuritySurface::PluginModTrust, SecuritySurface::Secrets, SecuritySurface::NetworkDefaults,
    SecuritySurface::Logs, SecuritySurface::SupportBundles};

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

} // namespace

const char* securitySurfaceName(const SecuritySurface surface) {
    switch (surface) {
    case SecuritySurface::ExternalProcess: return "external_process";
    case SecuritySurface::ArchiveExtraction: return "archive_extraction";
    case SecuritySurface::PathContainment: return "path_containment";
    case SecuritySurface::PluginModTrust: return "plugin_mod_trust";
    case SecuritySurface::Secrets: return "secrets";
    case SecuritySurface::NetworkDefaults: return "network_defaults";
    case SecuritySurface::Logs: return "logs";
    case SecuritySurface::SupportBundles: return "support_bundles";
    }
    return "unknown";
}

bool ReleaseSecurityAudit::isUnsafeRelativePath(const std::string& path) {
    if (path.empty()) return true;
    const std::filesystem::path candidate(path);
    if (candidate.is_absolute() || candidate.has_root_name() || path.starts_with("\\\\") || path.starts_with("//")) return true;
    for (const auto& component : candidate) {
        if (component == ".." || component == ".") return true;
    }
    return false;
}

bool ReleaseSecurityAudit::containsLikelySecret(const std::string& text) {
    const auto value = lower(text);
    for (const auto* marker : {"authorization:", "bearer ", "api_key=", "apikey=", "password=", "passwd=",
                               "client_secret=", "private_key", "-----begin private key-----"}) {
        if (value.find(marker) != std::string::npos) return true;
    }
    return false;
}

ReleaseSecurityResult ReleaseSecurityAudit::evaluate(std::vector<SecurityControlEvidence> evidence) const {
    ReleaseSecurityResult result;
    std::stable_sort(evidence.begin(), evidence.end(), [](const auto& left, const auto& right) {
        return std::tie(left.surface, left.owner) < std::tie(right.surface, right.owner);
    });
    std::set<SecuritySurface> covered;
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& row : evidence) {
        const auto surface = std::string(securitySurfaceName(row.surface));
        if (!covered.insert(row.surface).second || row.owner.empty() || row.control.empty() || row.adversarial_fixture.empty()) {
            result.diagnostics.push_back("security_evidence_invalid_or_duplicate:" + surface);
        }
        if (!row.default_deny) result.diagnostics.push_back("security_default_deny_missing:" + surface);
        if (!row.bounded) result.diagnostics.push_back("security_control_unbounded:" + surface);
        if (!row.secret_free_output) result.diagnostics.push_back("security_output_secret_risk:" + surface);
        rows.push_back({{"surface", surface}, {"owner", row.owner}, {"control", row.control},
                        {"adversarial_fixture", row.adversarial_fixture}, {"default_deny", row.default_deny},
                        {"bounded", row.bounded}, {"secret_free_output", row.secret_free_output}});
    }
    for (const auto surface : kRequiredSurfaces) {
        if (!covered.contains(surface)) result.diagnostics.push_back(std::string("security_surface_missing:") + securitySurfaceName(surface));
    }
    result.complete = covered.size() == kRequiredSurfaces.size();
    result.safe = result.complete && result.diagnostics.empty();
    result.threat_model = {{"schema", "urpg.release_security_threat_model.v1"}, {"complete", result.complete},
                           {"safe", result.safe}, {"controls", std::move(rows)},
                           {"diagnostics", result.diagnostics}};
    return result;
}

} // namespace urpg::security
