#pragma once

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace urpg::compat {

enum class MzVisualDiffStatus {
    Passed,
    Failed,
};

struct MzVisualDiffComparison {
    std::string scene_id;
    std::string reference_hash;
    std::string urpg_hash;
    double pixel_delta_percent = 0.0;
};

struct MzVisualDiffRow {
    std::string scene_id;
    std::string reference_hash;
    std::string urpg_hash;
    double pixel_delta_percent = 0.0;
    MzVisualDiffStatus status = MzVisualDiffStatus::Passed;

    nlohmann::json toJson() const;
};

struct MzVisualDiffReport {
    std::vector<MzVisualDiffRow> rows;
    size_t scene_count = 0;
    size_t passed_scene_count = 0;
    size_t failed_scene_count = 0;
    double pass_threshold_percent = 1.0;
    bool release_authoritative = false;

    void addComparison(const MzVisualDiffComparison& comparison);
    nlohmann::json toJson() const;
};

MzVisualDiffReport BuildEmptyMzVisualDiffReport();
const char* ToString(MzVisualDiffStatus status);

} // namespace urpg::compat
