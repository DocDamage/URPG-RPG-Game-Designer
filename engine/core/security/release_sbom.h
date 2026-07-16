#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::security {

enum class SbomComponentClass : uint8_t { Binary, Tool, PackagedAsset, OptionalProvider, ResearchOutput };

struct SbomComponent {
    std::string id;
    std::string name;
    std::string version;
    SbomComponentClass component_class = SbomComponentClass::Binary;
    std::string sha256;
    std::string license_id;
    std::string source;
    std::string notice;
    bool included_in_release = false;
    bool redistribution_approved = false;
    bool optional = false;
};

struct ReleaseSbomResult {
    bool complete = false;
    bool release_allowed = false;
    std::vector<std::string> diagnostics;
    nlohmann::json sbom;
    std::string notices;
};

class ReleaseSbomBuilder {
public:
    ReleaseSbomResult build(std::string release_version, std::vector<SbomComponent> components) const;
};

const char* sbomComponentClassName(SbomComponentClass component_class);

} // namespace urpg::security
