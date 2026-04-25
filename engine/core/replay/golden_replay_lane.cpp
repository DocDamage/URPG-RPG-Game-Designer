#include "engine/core/replay/golden_replay_lane.h"

#include "engine/core/replay/replay_player.h"

#include <algorithm>
#include <map>
#include <set>

namespace urpg::replay {

namespace {

std::map<std::string, ReplayArtifact> indexById(const std::vector<ReplayArtifact>& artifacts) {
    std::map<std::string, ReplayArtifact> indexed;
    for (const auto& artifact : artifacts) {
        indexed[artifact.id] = artifact;
    }
    return indexed;
}

GoldenReplayResult passedResult(const std::string& id) {
    return GoldenReplayResult{id, GoldenReplayStatus::Passed, -1, {}, {}, "matched"};
}

} // namespace

GoldenReplayReport GoldenReplayLane::compare(const std::vector<ReplayArtifact>& expected,
                                             const std::vector<ReplayArtifact>& actual,
                                             const std::string& current_project_version) {
    const auto expected_by_id = indexById(expected);
    const auto actual_by_id = indexById(actual);

    std::set<std::string> ids;
    for (const auto& [id, _] : expected_by_id) {
        ids.insert(id);
    }
    for (const auto& [id, _] : actual_by_id) {
        ids.insert(id);
    }

    GoldenReplayReport report;
    for (const auto& id : ids) {
        const auto expected_it = expected_by_id.find(id);
        const auto actual_it = actual_by_id.find(id);
        if (expected_it == expected_by_id.end()) {
            report.passed = false;
            report.results.push_back(GoldenReplayResult{
                id,
                GoldenReplayStatus::MissingExpected,
                -1,
                {},
                {},
                "actual replay has no committed golden baseline",
            });
            continue;
        }
        if (actual_it == actual_by_id.end()) {
            report.passed = false;
            report.results.push_back(GoldenReplayResult{
                id,
                GoldenReplayStatus::MissingActual,
                -1,
                {},
                {},
                "committed golden replay was not produced by the current run",
            });
            continue;
        }
        if (expected_it->second.project_version != current_project_version ||
            actual_it->second.project_version != current_project_version) {
            report.passed = false;
            report.results.push_back(GoldenReplayResult{
                id,
                GoldenReplayStatus::StaleProjectVersion,
                -1,
                {},
                {},
                "replay project version does not match the current validation version",
            });
            continue;
        }

        const auto comparison = ReplayPlayer::compare(expected_it->second, actual_it->second);
        if (!comparison.matches) {
            report.passed = false;
            report.results.push_back(GoldenReplayResult{
                id,
                GoldenReplayStatus::Diverged,
                comparison.first_mismatched_tick,
                comparison.expected_hash,
                comparison.actual_hash,
                "state hash diverged",
            });
            continue;
        }

        report.results.push_back(passedResult(id));
    }

    return report;
}

std::string toString(GoldenReplayStatus status) {
    switch (status) {
    case GoldenReplayStatus::Passed:
        return "passed";
    case GoldenReplayStatus::Diverged:
        return "diverged";
    case GoldenReplayStatus::MissingActual:
        return "missing_actual";
    case GoldenReplayStatus::MissingExpected:
        return "missing_expected";
    case GoldenReplayStatus::StaleProjectVersion:
        return "stale_project_version";
    }
    return "unknown";
}

} // namespace urpg::replay
