#include "engine/core/compat/mz_visual_diff_report.h"

#include <algorithm>

namespace urpg::compat {

const char* ToString(MzVisualDiffStatus status) {
    switch (status) {
    case MzVisualDiffStatus::Passed:
        return "passed";
    case MzVisualDiffStatus::Failed:
        return "failed";
    }
    return "unknown";
}

nlohmann::json MzVisualDiffRow::toJson() const {
    return {
        {"scene_id", scene_id},
        {"reference_hash", reference_hash},
        {"urpg_hash", urpg_hash},
        {"pixel_delta_percent", pixel_delta_percent},
        {"status", ToString(status)},
    };
}

void MzVisualDiffReport::addComparison(const MzVisualDiffComparison& comparison) {
    MzVisualDiffRow row;
    row.scene_id = comparison.scene_id;
    row.reference_hash = comparison.reference_hash;
    row.urpg_hash = comparison.urpg_hash;
    row.pixel_delta_percent = comparison.pixel_delta_percent;
    row.status = comparison.pixel_delta_percent <= pass_threshold_percent ? MzVisualDiffStatus::Passed
                                                                           : MzVisualDiffStatus::Failed;
    rows.push_back(std::move(row));

    std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
        return a.scene_id < b.scene_id;
    });

    scene_count = rows.size();
    passed_scene_count = 0;
    failed_scene_count = 0;
    for (const auto& existing : rows) {
        if (existing.status == MzVisualDiffStatus::Passed) {
            ++passed_scene_count;
        } else {
            ++failed_scene_count;
        }
    }
}

nlohmann::json MzVisualDiffReport::toJson() const {
    nlohmann::json row_json = nlohmann::json::array();
    for (const auto& row : rows) {
        row_json.push_back(row.toJson());
    }

    return {
        {"scene_count", scene_count},
        {"passed_scene_count", passed_scene_count},
        {"failed_scene_count", failed_scene_count},
        {"pass_threshold_percent", pass_threshold_percent},
        {"release_authoritative", release_authoritative},
        {"rows", std::move(row_json)},
    };
}

MzVisualDiffReport BuildEmptyMzVisualDiffReport() {
    MzVisualDiffReport report;
    report.release_authoritative = false;
    return report;
}

} // namespace urpg::compat
