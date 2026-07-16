#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::release {

enum class ReleaseChannel { Nightly, Beta, Stable };

struct ReleaseProvenance {
    ReleaseChannel channel = ReleaseChannel::Nightly;
    std::string version;
    std::string build_id;
    std::string source_commit;
    std::string compiler;
    std::string target;
    std::string runtime_sha256;
    std::string package_sha256;
    bool dirty = true;
};

struct ReleaseProvenanceResult {
    bool valid = false;
    std::vector<std::string> diagnostics;
    nlohmann::json metadata;
    std::string support_version;
};

class ReleaseProvenanceBuilder {
public:
    ReleaseProvenanceResult build(const ReleaseProvenance& provenance) const;
};

const char* releaseChannelName(ReleaseChannel channel);

} // namespace urpg::release
