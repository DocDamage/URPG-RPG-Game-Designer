#include "engine/core/release/external_qualification.h"

#include <algorithm>
#include <map>

namespace urpg::release {

namespace {

bool validId(std::string_view value) {
    return !value.empty() && value.size() <= 128 && std::all_of(value.begin(), value.end(), [](unsigned char ch) {
        return std::isalnum(ch) != 0 || ch == '_' || ch == '-' || ch == '.';
    });
}

const std::vector<std::string>& requiredRoutes() {
    static const std::vector<std::string> routes = {"create_open_import", "asset_use", "map_authoring",
        "event_authoring", "quest_authoring", "ui_authoring", "playtest_debug", "recovery", "package", "reopen"};
    return routes;
}

std::string gateName(FinalGateKind gate) {
    switch (gate) {
    case FinalGateKind::CleanClone: return "clean_clone";
    case FinalGateKind::CleanMachine: return "clean_machine";
    case FinalGateKind::Package: return "package";
    case FinalGateKind::Upgrade: return "upgrade";
    case FinalGateKind::SaveMigration: return "save_migration";
    case FinalGateKind::Accessibility: return "accessibility";
    case FinalGateKind::Performance: return "performance";
    case FinalGateKind::Security: return "security";
    case FinalGateKind::License: return "license";
    case FinalGateKind::Truth: return "truth";
    }
    return "unknown";
}

} // namespace

ResearchPlanAudit auditResearchPlan(const std::vector<ResearchParticipant>& participants,
                                    bool non_leading_tasks, bool deletion_policy) {
    ResearchPlanAudit result;
    std::set<ResearchUserType> types;
    std::set<std::string> ids;
    for (const auto& participant : participants) {
        if (!validId(participant.anonymous_id) || !ids.insert(participant.anonymous_id).second)
            result.diagnostics.push_back("Participant IDs must be anonymous, stable, and unique.");
        if (!participant.consented) result.diagnostics.push_back(participant.anonymous_id + ": consent is missing.");
        if (!participant.privacy_safe) result.diagnostics.push_back(participant.anonymous_id + ": privacy review is missing.");
        types.insert(participant.type);
    }
    if (types.size() != 4) result.diagnostics.push_back("Novice, intermediate, accessibility, and experienced users are all required.");
    if (!non_leading_tasks) result.diagnostics.push_back("Research tasks must be non-leading.");
    if (!deletion_policy) result.diagnostics.push_back("Research data deletion policy is missing.");
    result.ready = result.diagnostics.empty();
    return result;
}

JourneyAudit auditJourneySessions(const std::vector<JourneySession>& sessions) {
    JourneyAudit result;
    if (sessions.empty()) {
        result.diagnostics.push_back("No external journey sessions were recorded.");
        return result;
    }
    uint64_t required_count = 0;
    uint64_t completed_count = 0;
    for (const auto& session : sessions) {
        if (!validId(session.participant_id) || !validId(session.build_id))
            result.diagnostics.push_back("Session participant and build IDs are required.");
        std::set<std::string> routes;
        for (const auto& metric : session.metrics) {
            routes.insert(metric.route);
            ++required_count;
            completed_count += metric.completed && !metric.abandoned ? 1U : 0U;
            if (metric.duration_seconds == 0 || metric.confidence > 5)
                result.diagnostics.push_back(session.participant_id + ": route metrics are incomplete or invalid.");
        }
        for (const auto& route : requiredRoutes()) if (!routes.contains(route))
            result.diagnostics.push_back(session.participant_id + ": missing route " + route + ".");
    }
    result.completion_rate = required_count == 0 ? 0.0 : static_cast<double>(completed_count) / required_count;
    result.complete = result.diagnostics.empty();
    return result;
}

FindingAudit auditUsabilityFindings(const std::vector<UsabilityFinding>& findings) {
    FindingAudit result;
    std::set<std::string> ids;
    for (const auto& finding : findings) {
        if (!validId(finding.id) || !ids.insert(finding.id).second)
            result.diagnostics.push_back("Finding IDs must be stable and unique.");
        if (finding.severity == FindingSeverity::P0 || finding.severity == FindingSeverity::P1) {
            if (!finding.resolved) result.diagnostics.push_back(finding.id + ": P0/P1 finding is unresolved.");
            if (!finding.retested_with_new_user) result.diagnostics.push_back(finding.id + ": new-user retest is missing.");
            if (!finding.automated_regression) result.diagnostics.push_back(finding.id + ": automated regression is missing.");
        }
    }
    result.qualified = result.diagnostics.empty();
    return result;
}

BetaAudit auditBetaPlan(const BetaPlan& plan) {
    BetaAudit result;
    if (!validId(plan.cohort_id) || plan.participant_count < 5)
        result.diagnostics.push_back("A privacy-safe beta cohort of at least five participants is required.");
    if (!plan.diagnostics_opt_in) result.diagnostics.push_back("Beta diagnostics must be opt-in.");
    if (!plan.migration_policy) result.diagnostics.push_back("Beta migration policy is missing.");
    if (!plan.backup_instructions) result.diagnostics.push_back("Beta backup instructions are missing.");
    if (!plan.known_issues) result.diagnostics.push_back("Beta known-issues disclosure is missing.");
    if (plan.support_owner.empty()) result.diagnostics.push_back("Beta support response owner is missing.");
    if (plan.rollback_plan.empty()) result.diagnostics.push_back("Beta rollback plan is missing.");
    if (plan.data_loss_events != 0) result.diagnostics.push_back("Beta has unresolved data-loss events.");
    result.qualified = result.diagnostics.empty();
    return result;
}

CandidateAudit auditCandidateLedger(const CandidateLedger& ledger) {
    CandidateAudit result;
    if (!ledger.frozen || !validId(ledger.build_id) || ledger.commit.size() < 8)
        result.diagnostics.push_back("Candidate must be frozen with exact build and commit identity.");
    std::set<std::string> ids;
    for (const auto& change : ledger.changes) {
        if (!validId(change.id) || !ids.insert(change.id).second)
            result.diagnostics.push_back("Candidate change IDs must be stable and unique.");
        if (!change.approved_blocker) result.diagnostics.push_back(change.id + ": non-blocker change entered the freeze.");
        if (!change.focused_gates_passed || !change.full_gates_passed)
            result.diagnostics.push_back(change.id + ": focused and full retest chain is incomplete.");
    }
    result.qualified = result.diagnostics.empty();
    return result;
}

FinalQualificationAudit auditFinalQualification(const std::vector<FinalGateEvidence>& evidence,
                                                 std::string_view candidate_build) {
    FinalQualificationAudit result;
    std::map<FinalGateKind, size_t> counts;
    for (const auto& item : evidence) {
        ++counts[item.gate];
        if (!item.passed) result.diagnostics.push_back(gateName(item.gate) + ": gate did not pass.");
        if (item.build_id != candidate_build) result.diagnostics.push_back(gateName(item.gate) + ": evidence is for a stale build.");
        if (item.report.empty() || item.evidence_date.empty())
            result.diagnostics.push_back(gateName(item.gate) + ": report or date is missing.");
    }
    for (int gate = static_cast<int>(FinalGateKind::CleanClone); gate <= static_cast<int>(FinalGateKind::Truth); ++gate) {
        const auto kind = static_cast<FinalGateKind>(gate);
        if (counts[kind] != 1) result.diagnostics.push_back(gateName(kind) + ": exactly one current evidence row is required.");
    }
    result.qualified = result.diagnostics.empty();
    return result;
}

ReleaseDecisionAudit auditReleaseDecision(const ReleaseDecision& decision,
                                          const FinalQualificationAudit& qualification) {
    ReleaseDecisionAudit result;
    if (decision.decision == ReleaseDecisionKind::Undecided || !validId(decision.build_id))
        result.diagnostics.push_back("Release decision and exact build are required.");
    if (!decision.public_claims_aligned) result.diagnostics.push_back("Public claims are not aligned to the decision.");
    if (!decision.signed_decision) result.diagnostics.push_back("Release decision is unsigned.");
    if (decision.owners.empty()) result.diagnostics.push_back("Risk and follow-up owners are missing.");
    if (decision.decision == ReleaseDecisionKind::Ship && !qualification.qualified)
        result.diagnostics.push_back("Ship decision requires complete final qualification.");
    if ((decision.decision == ReleaseDecisionKind::Delay || decision.decision == ReleaseDecisionKind::ReduceScope) &&
        decision.risks.empty() && decision.known_issues.empty())
        result.diagnostics.push_back("Delay or reduced-scope decision must name the blocking risk or issue.");
    result.valid = result.diagnostics.empty();
    return result;
}

} // namespace urpg::release
