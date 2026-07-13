#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace urpg::compat {

struct MzCompatibilityLaneInput {
    int32_t score = 0;
    int32_t covered_count = 0;
    int32_t unsupported_count = 0;
    std::string id;
};

struct MzCompatibilityLaneReport {
    std::string id;
    int32_t score = 0;
    int32_t covered_count = 0;
    int32_t unsupported_count = 0;
    bool present = false;

    nlohmann::json toJson() const;
};

struct MzProjectCompatibilityInput {
    MzCompatibilityLaneInput maps;
    MzCompatibilityLaneInput events;
    MzCompatibilityLaneInput plugins;
    MzCompatibilityLaneInput saves;
    MzCompatibilityLaneInput assets;
    MzCompatibilityLaneInput visual_parity;
    MzCompatibilityLaneInput runtime_parity;
    int32_t unsupported_event_command_count = 0;
};

struct MzProjectCompatibilityReport {
    std::map<std::string, MzCompatibilityLaneReport> lanes;
    std::vector<std::string> blockers;
    int32_t project_score = 100;
    bool release_authoritative = false;

    nlohmann::json toJson() const;
};

MzProjectCompatibilityReport BuildMzProjectCompatibilityReport(const MzProjectCompatibilityInput& input);

} // namespace urpg::compat
