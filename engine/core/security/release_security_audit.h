#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::security {

enum class SecuritySurface : uint8_t {
    ExternalProcess,
    ArchiveExtraction,
    PathContainment,
    PluginModTrust,
    Secrets,
    NetworkDefaults,
    Logs,
    SupportBundles
};

struct SecurityControlEvidence {
    SecuritySurface surface = SecuritySurface::ExternalProcess;
    std::string owner;
    std::string control;
    std::string adversarial_fixture;
    bool default_deny = false;
    bool bounded = false;
    bool secret_free_output = false;
};

struct ReleaseSecurityResult {
    bool complete = false;
    bool safe = false;
    std::vector<std::string> diagnostics;
    nlohmann::json threat_model;
};

class ReleaseSecurityAudit {
public:
    ReleaseSecurityResult evaluate(std::vector<SecurityControlEvidence> evidence) const;

    static bool isUnsafeRelativePath(const std::string& path);
    static bool containsLikelySecret(const std::string& text);
};

const char* securitySurfaceName(SecuritySurface surface);

} // namespace urpg::security
