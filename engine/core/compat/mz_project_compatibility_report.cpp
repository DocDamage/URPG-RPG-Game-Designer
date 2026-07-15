#include "engine/core/compat/mz_project_compatibility_report.h"

#include <algorithm>

namespace urpg::compat {

namespace {

int32_t clampScore(int32_t score) {
    return std::clamp(score, 0, 100);
}

bool lanePresent(const MzCompatibilityLaneInput& lane) {
    return lane.covered_count > 0 || lane.unsupported_count > 0 || lane.score > 0;
}

void addLane(MzProjectCompatibilityReport& report, const MzCompatibilityLaneInput& input, const std::string& fallback_id) {
    if (!lanePresent(input)) {
        return;
    }

    MzCompatibilityLaneReport lane;
    lane.id = input.id.empty() ? fallback_id : input.id;
    lane.score = clampScore(input.score);
    lane.covered_count = std::max(0, input.covered_count);
    lane.unsupported_count = std::max(0, input.unsupported_count);
    lane.present = true;
    report.lanes[lane.id] = lane;

    if (lane.score == 0) {
        report.blockers.push_back(lane.id + "_unready");
    }
}

} // namespace

nlohmann::json MzCompatibilityLaneReport::toJson() const {
    return {
        {"id", id},
        {"score", score},
        {"covered_count", covered_count},
        {"unsupported_count", unsupported_count},
        {"present", present},
    };
}

nlohmann::json MzProjectCompatibilityReport::toJson() const {
    nlohmann::json lane_json = nlohmann::json::object();
    for (const auto& [id, lane] : lanes) {
        lane_json[id] = lane.toJson();
    }

    return {
        {"project_score", project_score},
        {"release_authoritative", release_authoritative},
        {"blockers", blockers},
        {"lanes", std::move(lane_json)},
    };
}

MzProjectCompatibilityReport BuildMzProjectCompatibilityReport(const MzProjectCompatibilityInput& input) {
    MzProjectCompatibilityReport report;
    addLane(report, input.maps, "maps");
    addLane(report, input.events, "events");
    addLane(report, input.plugins, "plugins");
    addLane(report, input.saves, "saves");
    addLane(report, input.assets, "assets");
    addLane(report, input.visual_parity, "visual_parity");
    addLane(report, input.runtime_parity, "runtime_parity");

    if (input.unsupported_event_command_count > 0) {
        report.blockers.push_back("unsupported_event_commands");
    }

    std::sort(report.blockers.begin(), report.blockers.end());
    report.blockers.erase(std::unique(report.blockers.begin(), report.blockers.end()), report.blockers.end());

    if (!report.lanes.empty()) {
        int32_t total = 0;
        for (const auto& [_, lane] : report.lanes) {
            total += lane.score;
        }
        report.project_score = total / static_cast<int32_t>(report.lanes.size());
    }
    report.release_authoritative = false;
    return report;
}

} // namespace urpg::compat
