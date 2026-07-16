#include "engine/core/release/external_qualification.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("External research plan requires representative consented privacy-safe users and non-leading tasks",
          "[release][research][pcq900]") {
    using namespace urpg::release;
    const auto passing = auditResearchPlan({
        {"p-novice", ResearchUserType::Novice, true, true},
        {"p-intermediate", ResearchUserType::Intermediate, true, true},
        {"p-access", ResearchUserType::Accessibility, true, true},
        {"p-experienced", ResearchUserType::Experienced, true, true}}, true, true);
    REQUIRE(passing.ready);
    REQUIRE(passing.diagnostics.empty());
    const auto failing = auditResearchPlan({{"p", ResearchUserType::Novice, false, false}}, false, false);
    REQUIRE_FALSE(failing.ready);
    REQUIRE(failing.diagnostics.size() == 5);
}

TEST_CASE("Creator journey sessions require route-level completion error assistance time abandonment and confidence metrics",
          "[release][research][pcq901]") {
    using namespace urpg::release;
    JourneySession session{"p-novice", "candidate-1", {}, true};
    for (const auto& route : {"create_open_import", "asset_use", "map_authoring", "event_authoring",
                              "quest_authoring", "ui_authoring", "playtest_debug", "recovery", "package", "reopen"})
        session.metrics.push_back({route, true, 0, 0, 60, false, 4});
    const auto passing = auditJourneySessions({session});
    REQUIRE(passing.complete);
    REQUIRE(passing.completion_rate == 1.0);
    session.metrics.pop_back();
    const auto failing = auditJourneySessions({session});
    REQUIRE_FALSE(failing.complete);
    REQUIRE(failing.diagnostics == std::vector<std::string>{"p-novice: missing route reopen."});
}

TEST_CASE("P0 and P1 usability findings require resolution new-user retest and automated regression",
          "[release][research][pcq902]") {
    using namespace urpg::release;
    REQUIRE(auditUsabilityFindings({{"finding-1", FindingSeverity::P1, true, true, true}}).qualified);
    const auto failing = auditUsabilityFindings({{"finding-1", FindingSeverity::P0, false, false, false}});
    REQUIRE_FALSE(failing.qualified);
    REQUIRE(failing.diagnostics.size() == 3);
}

TEST_CASE("Bounded beta requires opt-in diagnostics migration backup known issues support rollback and no data loss",
          "[release][beta][pcq903]") {
    using namespace urpg::release;
    BetaPlan plan{"beta-1", 8, true, true, true, true, "support-owner", "restore candidate backup", 2, 0};
    REQUIRE(auditBetaPlan(plan).qualified);
    plan.diagnostics_opt_in = false;
    plan.data_loss_events = 1;
    const auto failing = auditBetaPlan(plan);
    REQUIRE_FALSE(failing.qualified);
    REQUIRE(failing.diagnostics.size() == 2);
}

TEST_CASE("Candidate freeze admits only approved blocker changes with focused and full retest chains",
          "[release][candidate][pcq904]") {
    using namespace urpg::release;
    CandidateLedger ledger{"candidate-1", "abcdef123456", true,
                           {{"blocker-1", true, true, true}}};
    REQUIRE(auditCandidateLedger(ledger).qualified);
    ledger.changes.push_back({"feature-2", false, true, false});
    const auto failing = auditCandidateLedger(ledger);
    REQUIRE_FALSE(failing.qualified);
    REQUIRE(failing.diagnostics.size() == 2);
}

TEST_CASE("Final qualification requires one current passing row for all ten release authorities",
          "[release][qualification][pcq905]") {
    using namespace urpg::release;
    std::vector<FinalGateEvidence> evidence;
    for (int value = static_cast<int>(FinalGateKind::CleanClone);
         value <= static_cast<int>(FinalGateKind::Truth); ++value)
        evidence.push_back({static_cast<FinalGateKind>(value), true, "candidate-1", "report.json", "2026-07-16"});
    const auto passing = auditFinalQualification(evidence, "candidate-1");
    REQUIRE(passing.qualified);
    evidence.back().build_id = "stale";
    const auto failing = auditFinalQualification(evidence, "candidate-1");
    REQUIRE_FALSE(failing.qualified);
    REQUIRE(failing.diagnostics.size() == 1);
}

TEST_CASE("Release decision records ship delay or reduced scope with signatures owners risks and aligned claims",
          "[release][decision][pcq906]") {
    using namespace urpg::release;
    FinalQualificationAudit qualified{true, {}};
    ReleaseDecision ship{ReleaseDecisionKind::Ship, "candidate-1", {}, {}, {}, {"release-owner"}, true, true};
    REQUIRE(auditReleaseDecision(ship, qualified).valid);
    qualified.qualified = false;
    REQUIRE_FALSE(auditReleaseDecision(ship, qualified).valid);
    ReleaseDecision delay{ReleaseDecisionKind::Delay, "candidate-1", {"clean-machine evidence missing"},
                          {}, {}, {"release-owner"}, true, true};
    REQUIRE(auditReleaseDecision(delay, qualified).valid);
}
