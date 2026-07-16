#include "engine/core/release/release_provenance.h"

#include "engine/core/security/release_security_audit.h"

#include <algorithm>
#include <cctype>

namespace urpg::release {
namespace {

bool validHash(const std::string& value) {
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](const unsigned char character) {
        return std::isxdigit(character) != 0;
    });
}

bool containsWorkstationPath(const std::string& value) {
    return value.find("C:/Users/") != std::string::npos || value.find("C:\\Users\\") != std::string::npos ||
           value.find("/home/") != std::string::npos || value.find("/Users/") != std::string::npos;
}

} // namespace

const char* releaseChannelName(const ReleaseChannel channel) {
    switch (channel) {
    case ReleaseChannel::Nightly: return "nightly";
    case ReleaseChannel::Beta: return "beta";
    case ReleaseChannel::Stable: return "stable";
    }
    return "unknown";
}

ReleaseProvenanceResult ReleaseProvenanceBuilder::build(const ReleaseProvenance& provenance) const {
    ReleaseProvenanceResult result;
    const std::pair<const char*, const std::string*> fields[] = {
        {"version", &provenance.version}, {"build_id", &provenance.build_id},
        {"source_commit", &provenance.source_commit}, {"compiler", &provenance.compiler}, {"target", &provenance.target}};
    for (const auto& [name, value] : fields) {
        if (value->empty()) result.diagnostics.push_back(std::string("release_provenance_missing:") + name);
        if (containsWorkstationPath(*value)) result.diagnostics.push_back(std::string("release_provenance_workstation_path:") + name);
        if (urpg::security::ReleaseSecurityAudit::containsLikelySecret(*value)) {
            result.diagnostics.push_back(std::string("release_provenance_secret:") + name);
        }
    }
    if (!validHash(provenance.runtime_sha256)) result.diagnostics.push_back("release_provenance_runtime_hash_invalid");
    if (!validHash(provenance.package_sha256)) result.diagnostics.push_back("release_provenance_package_hash_invalid");
    if (provenance.channel == ReleaseChannel::Stable && provenance.dirty) result.diagnostics.push_back("release_provenance_stable_dirty");
    result.valid = result.diagnostics.empty();
    result.support_version = provenance.version + "+" + provenance.build_id + "." + provenance.source_commit;
    result.metadata = {{"schema", "urpg.release_provenance.v1"}, {"channel", releaseChannelName(provenance.channel)},
                       {"version", provenance.version}, {"build_id", provenance.build_id},
                       {"source_commit", provenance.source_commit}, {"compiler", provenance.compiler},
                       {"target", provenance.target}, {"runtime_sha256", provenance.runtime_sha256},
                       {"package_sha256", provenance.package_sha256}, {"dirty", provenance.dirty},
                       {"support_version", result.support_version}, {"valid", result.valid},
                       {"diagnostics", result.diagnostics}};
    return result;
}

} // namespace urpg::release
