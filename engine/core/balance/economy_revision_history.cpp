#include "engine/core/balance/economy_revision_history.h"

#include <algorithm>
#include <iomanip>
#include <memory>
#include <set>
#include <sstream>

namespace urpg::balance {

namespace {

nlohmann::json scenarioToJson(const EconomyScenario& scenario) {
    nlohmann::json steps = nlohmann::json::array();
    for (const auto& step : scenario.route.steps) {
        steps.push_back({{"id", step.id}, {"gold_delta", step.gold_delta}, {"xp_delta", step.xp_delta},
                         {"item_id", step.item_id}, {"required", step.required}, {"cost", step.cost}});
    }
    nlohmann::json strategies = nlohmann::json::array();
    for (const auto& strategy : scenario.strategies) {
        strategies.push_back({{"id", strategy.id}, {"optional_purchase_chance", strategy.optional_purchase_chance},
                              {"gold_reward_percent", strategy.gold_reward_percent},
                              {"xp_reward_percent", strategy.xp_reward_percent}});
    }
    return {{"id", scenario.id}, {"version", scenario.version},
            {"route", {{"starting_gold", scenario.route.starting_gold}, {"steps", std::move(steps)}}},
            {"strategies", std::move(strategies)}, {"level_xp_curve", scenario.level_xp_curve},
            {"seed", scenario.seed}, {"iterations", scenario.iterations},
            {"sensitivity_starting_gold", scenario.sensitivity_starting_gold},
            {"max_reward_gap", scenario.max_reward_gap}};
}

nlohmann::json simulationToJson(const EconomyScenarioReport& report) {
    nlohmann::json strategies = nlohmann::json::array();
    for (const auto& strategy : report.strategies) {
        strategies.push_back({{"strategy_id", strategy.strategy_id},
                              {"average_final_gold", strategy.average_final_gold},
                              {"average_total_xp", strategy.average_total_xp},
                              {"average_level", strategy.average_level},
                              {"required_deadlocks", strategy.required_deadlocks}});
    }
    nlohmann::json sensitivities = nlohmann::json::array();
    for (const auto& sensitivity : report.sensitivities) {
        sensitivities.push_back({{"parameter", sensitivity.parameter}, {"baseline", sensitivity.baseline},
                                 {"tested", sensitivity.tested},
                                 {"final_gold_delta", sensitivity.final_gold_delta}});
    }
    nlohmann::json findings = nlohmann::json::array();
    for (const auto& finding : report.findings) {
        findings.push_back({{"code", finding.code}, {"severity", finding.severity},
                            {"strategy_id", finding.strategy_id}, {"step_id", finding.step_id},
                            {"parameter", finding.parameter}, {"source_route", finding.source_route},
                            {"message", finding.message}});
    }
    return {{"scenario_id", report.scenario_id}, {"scenario_version", report.scenario_version},
            {"seed", report.seed}, {"iterations", report.iterations}, {"assumptions", report.assumptions},
            {"strategies", std::move(strategies)}, {"sensitivities", std::move(sensitivities)},
            {"findings", std::move(findings)}};
}

std::string contentHash(const nlohmann::json& value) {
    uint64_t hash = 1469598103934665603ULL;
    for (const auto character : value.dump()) {
        hash ^= static_cast<unsigned char>(character);
        hash *= 1099511628211ULL;
    }
    std::ostringstream stream;
    stream << std::hex << std::setw(16) << std::setfill('0') << hash;
    return stream.str();
}

void collectDifferences(const nlohmann::json& before, const nlohmann::json& after, const std::string& path,
                        std::vector<EconomyRevisionDifference>& differences) {
    if (before.type() != after.type()) {
        differences.push_back({path, before, after, std::nullopt});
        return;
    }
    if (before.is_object()) {
        std::set<std::string> keys;
        for (const auto& [key, _] : before.items()) keys.insert(key);
        for (const auto& [key, _] : after.items()) keys.insert(key);
        for (const auto& key : keys) {
            const auto childPath = path + "/" + key;
            if (!before.contains(key)) differences.push_back({childPath, nullptr, after.at(key), std::nullopt});
            else if (!after.contains(key)) differences.push_back({childPath, before.at(key), nullptr, std::nullopt});
            else collectDifferences(before.at(key), after.at(key), childPath, differences);
        }
        return;
    }
    if (before.is_array()) {
        const auto count = std::max(before.size(), after.size());
        for (std::size_t index = 0; index < count; ++index) {
            const auto childPath = path + "/" + std::to_string(index);
            if (index >= before.size()) differences.push_back({childPath, nullptr, after[index], std::nullopt});
            else if (index >= after.size()) differences.push_back({childPath, before[index], nullptr, std::nullopt});
            else collectDifferences(before[index], after[index], childPath, differences);
        }
        return;
    }
    if (before == after) return;
    std::optional<double> delta;
    if (before.is_number() && after.is_number()) delta = after.get<double>() - before.get<double>();
    differences.push_back({path, before, after, delta});
}

nlohmann::json revisionToJson(const EconomyCommittedRevision& revision) {
    return {{"revision_id", revision.revision_id}, {"label", revision.label},
            {"source_revision", revision.source_revision}, {"content_hash", revision.content_hash},
            {"scenario", scenarioToJson(revision.scenario)}, {"simulation", simulationToJson(revision.simulation)}};
}

} // namespace

bool EconomyRevisionHistory::commit(std::string revision_id, std::string label, uint64_t source_revision,
                                    const EconomyScenario& scenario, std::string* diagnostic) {
    if (revision_id.empty() || label.empty() || source_revision == 0 || find(revision_id) != nullptr) {
        if (diagnostic) *diagnostic = find(revision_id) != nullptr ? "economy_revision_duplicate" :
                                                                    "economy_revision_invalid";
        return false;
    }
    const auto simulation = EconomySimulator::runScenario(scenario);
    if (std::any_of(simulation.findings.begin(), simulation.findings.end(), [](const auto& finding) {
            return finding.code == "scenario_invalid";
        })) {
        if (diagnostic) *diagnostic = "economy_revision_scenario_invalid";
        return false;
    }
    const auto scenarioJson = scenarioToJson(scenario);
    revisions_.push_back({std::move(revision_id), std::move(label), source_revision, contentHash(scenarioJson),
                          scenario, simulation});
    return true;
}

const EconomyCommittedRevision* EconomyRevisionHistory::find(std::string_view revision_id) const {
    const auto found = std::find_if(revisions_.begin(), revisions_.end(), [revision_id](const auto& revision) {
        return revision.revision_id == revision_id;
    });
    return found == revisions_.end() ? nullptr : &*found;
}

std::vector<std::string> EconomyRevisionHistory::revisionIds() const {
    std::vector<std::string> ids;
    ids.reserve(revisions_.size());
    for (const auto& revision : revisions_) ids.push_back(revision.revision_id);
    return ids;
}

EconomyRevisionComparison EconomyRevisionHistory::compare(std::string_view before_revision_id,
                                                          std::string_view after_revision_id) const {
    EconomyRevisionComparison result;
    result.before_revision_id = std::string(before_revision_id);
    result.after_revision_id = std::string(after_revision_id);
    const auto* before = find(before_revision_id);
    const auto* after = find(after_revision_id);
    if (before == nullptr || after == nullptr || before_revision_id == after_revision_id) {
        result.code = before_revision_id == after_revision_id ? "economy_revision_compare_same" :
                                                               "economy_revision_compare_missing";
        return result;
    }
    collectDifferences(scenarioToJson(before->scenario), scenarioToJson(after->scenario), "scenario",
                       result.differences);
    collectDifferences(simulationToJson(before->simulation), simulationToJson(after->simulation), "simulation",
                       result.differences);
    result.success = true;
    result.code = result.differences.empty() ? "economy_revision_compare_identical" :
                                              "economy_revision_compare_ready";
    return result;
}

nlohmann::json EconomyRevisionHistory::exportReviewReport(std::string_view before_revision_id,
                                                          std::string_view after_revision_id) const {
    const auto comparison = compare(before_revision_id, after_revision_id);
    if (!comparison.success) return {{"schema", "urpg.economy_revision_review.v1"},
                                     {"success", false}, {"code", comparison.code}};
    nlohmann::json differences = nlohmann::json::array();
    for (const auto& difference : comparison.differences) {
        auto value = nlohmann::json{{"path", difference.path}, {"before", difference.before},
                                    {"after", difference.after}};
        if (difference.numeric_delta) value["numeric_delta"] = *difference.numeric_delta;
        differences.push_back(std::move(value));
    }
    return {{"schema", "urpg.economy_revision_review.v1"}, {"success", true}, {"code", comparison.code},
            {"before", revisionToJson(*find(before_revision_id))},
            {"after", revisionToJson(*find(after_revision_id))}, {"differences", std::move(differences)}};
}

std::optional<project::ProjectOperationParticipant> EconomyRevisionHistory::makeRestoreParticipant(
    std::string_view revision_id, EconomyScenario& authoritative_scenario, uint64_t& authoritative_revision) const {
    const auto* revision = find(revision_id);
    if (revision == nullptr) return std::nullopt;
    struct RestoreState {
        EconomyScenario target;
        EconomyScenario before;
        EconomyScenario* authority = nullptr;
        uint64_t* revision = nullptr;
        uint64_t before_revision = 0;
    };
    auto state = std::make_shared<RestoreState>(RestoreState{revision->scenario, {}, &authoritative_scenario,
                                                             &authoritative_revision, authoritative_revision});
    return project::ProjectOperationParticipant{
        "balance.economy", authoritative_revision,
        [state] { return *state->revision; },
        [state](std::string&) {
            state->before = *state->authority;
            state->before_revision = *state->revision;
            return true;
        },
        [state](std::string&) {
            *state->authority = state->target;
            *state->revision = state->before_revision + 1;
            return true;
        },
        [state] {
            *state->authority = state->before;
            *state->revision = state->before_revision;
        },
        [state](std::string&) {
            *state->authority = state->before;
            ++*state->revision;
            return true;
        }};
}

} // namespace urpg::balance
