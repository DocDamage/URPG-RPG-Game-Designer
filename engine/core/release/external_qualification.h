#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace urpg::release {

enum class ResearchUserType { Novice, Intermediate, Accessibility, Experienced };
enum class FindingSeverity { P0, P1, P2, P3 };
enum class ReleaseDecisionKind { Undecided, Ship, Delay, ReduceScope };

struct ResearchParticipant {
    std::string anonymous_id;
    ResearchUserType type = ResearchUserType::Novice;
    bool consented = false;
    bool privacy_safe = false;
};

struct ResearchPlanAudit {
    bool ready = false;
    std::vector<std::string> diagnostics;
};

ResearchPlanAudit auditResearchPlan(const std::vector<ResearchParticipant>& participants,
                                    bool non_leading_tasks, bool deletion_policy);

struct JourneyMetric {
    std::string route;
    bool completed = false;
    uint32_t errors = 0;
    uint32_t assistance_events = 0;
    uint32_t duration_seconds = 0;
    bool abandoned = false;
    uint8_t confidence = 0;
};

struct JourneySession {
    std::string participant_id;
    std::string build_id;
    std::vector<JourneyMetric> metrics;
    bool recording_consent = false;
};

struct JourneyAudit {
    bool complete = false;
    double completion_rate = 0.0;
    std::vector<std::string> diagnostics;
};

JourneyAudit auditJourneySessions(const std::vector<JourneySession>& sessions);

struct UsabilityFinding {
    std::string id;
    FindingSeverity severity = FindingSeverity::P3;
    bool resolved = false;
    bool retested_with_new_user = false;
    bool automated_regression = false;
};

struct FindingAudit {
    bool qualified = false;
    std::vector<std::string> diagnostics;
};

FindingAudit auditUsabilityFindings(const std::vector<UsabilityFinding>& findings);

struct BetaPlan {
    std::string cohort_id;
    uint32_t participant_count = 0;
    bool diagnostics_opt_in = false;
    bool migration_policy = false;
    bool backup_instructions = false;
    bool known_issues = false;
    std::string support_owner;
    std::string rollback_plan;
    uint32_t crashes = 0;
    uint32_t data_loss_events = 0;
};

struct BetaAudit {
    bool qualified = false;
    std::vector<std::string> diagnostics;
};

BetaAudit auditBetaPlan(const BetaPlan& plan);

struct CandidateChange {
    std::string id;
    bool approved_blocker = false;
    bool focused_gates_passed = false;
    bool full_gates_passed = false;
};

struct CandidateLedger {
    std::string build_id;
    std::string commit;
    bool frozen = false;
    std::vector<CandidateChange> changes;
};

struct CandidateAudit {
    bool qualified = false;
    std::vector<std::string> diagnostics;
};

CandidateAudit auditCandidateLedger(const CandidateLedger& ledger);

enum class FinalGateKind {
    CleanClone, CleanMachine, Package, Upgrade, SaveMigration,
    Accessibility, Performance, Security, License, Truth
};

struct FinalGateEvidence {
    FinalGateKind gate = FinalGateKind::CleanClone;
    bool passed = false;
    std::string build_id;
    std::string report;
    std::string evidence_date;
};

struct FinalQualificationAudit {
    bool qualified = false;
    std::vector<std::string> diagnostics;
};

FinalQualificationAudit auditFinalQualification(const std::vector<FinalGateEvidence>& evidence,
                                                 std::string_view candidate_build);

struct ReleaseDecision {
    ReleaseDecisionKind decision = ReleaseDecisionKind::Undecided;
    std::string build_id;
    std::vector<std::string> risks;
    std::vector<std::string> unsupported_targets;
    std::vector<std::string> known_issues;
    std::vector<std::string> owners;
    bool public_claims_aligned = false;
    bool signed_decision = false;
};

struct ReleaseDecisionAudit {
    bool valid = false;
    std::vector<std::string> diagnostics;
};

ReleaseDecisionAudit auditReleaseDecision(const ReleaseDecision& decision,
                                          const FinalQualificationAudit& qualification);

} // namespace urpg::release
