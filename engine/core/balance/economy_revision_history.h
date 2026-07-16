#pragma once

#include "engine/core/balance/economy_simulator.h"
#include "engine/core/project/project_operation_coordinator.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::balance {

struct EconomyCommittedRevision {
    std::string revision_id;
    std::string label;
    uint64_t source_revision = 0;
    std::string content_hash;
    EconomyScenario scenario;
    EconomyScenarioReport simulation;
};

struct EconomyRevisionDifference {
    std::string path;
    nlohmann::json before;
    nlohmann::json after;
    std::optional<double> numeric_delta;
};

struct EconomyRevisionComparison {
    bool success = false;
    std::string code;
    std::string before_revision_id;
    std::string after_revision_id;
    std::vector<EconomyRevisionDifference> differences;
};

class EconomyRevisionHistory {
public:
    bool commit(std::string revision_id, std::string label, uint64_t source_revision,
                const EconomyScenario& scenario, std::string* diagnostic = nullptr);
    const EconomyCommittedRevision* find(std::string_view revision_id) const;
    std::vector<std::string> revisionIds() const;

    EconomyRevisionComparison compare(std::string_view before_revision_id,
                                      std::string_view after_revision_id) const;
    nlohmann::json exportReviewReport(std::string_view before_revision_id,
                                     std::string_view after_revision_id) const;

    std::optional<project::ProjectOperationParticipant> makeRestoreParticipant(
        std::string_view revision_id, EconomyScenario& authoritative_scenario,
        uint64_t& authoritative_revision) const;

private:
    std::vector<EconomyCommittedRevision> revisions_;
};

} // namespace urpg::balance
