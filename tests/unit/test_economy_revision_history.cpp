#include "engine/core/balance/economy_revision_history.h"
#include "engine/core/project/project_operation_journal.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

urpg::balance::EconomyScenario scenario(std::string version, int startingGold, int questReward) {
    urpg::balance::EconomyScenario value;
    value.id = "willow.economy";
    value.version = std::move(version);
    value.route.starting_gold = startingGold;
    value.route.steps = {{"quest.reward", questReward, 50, {}, false, 0},
                         {"gate.key", 0, 0, "item.key", true, 80}};
    value.strategies = {{"completionist", 100, 100, 100}};
    value.level_xp_curve = {40, 100};
    value.seed = 99;
    value.iterations = 5;
    value.sensitivity_starting_gold = 20;
    return value;
}

} // namespace

TEST_CASE("Economy history compares committed revisions and exports deterministic review evidence",
          "[balance][economy][history][pcq454]") {
    urpg::balance::EconomyRevisionHistory history;
    REQUIRE(history.commit("balance.r1", "Initial economy", 1, scenario("v1", 20, 40)));
    REQUIRE(history.commit("balance.r2", "Affordable key", 2, scenario("v2", 50, 60)));
    REQUIRE(history.revisionIds() == std::vector<std::string>{"balance.r1", "balance.r2"});
    REQUIRE(history.find("balance.r1")->content_hash.size() == 16);
    REQUIRE(history.find("balance.r1")->content_hash != history.find("balance.r2")->content_hash);

    const auto comparison = history.compare("balance.r1", "balance.r2");
    REQUIRE(comparison.success);
    REQUIRE(comparison.code == "economy_revision_compare_ready");
    REQUIRE(std::any_of(comparison.differences.begin(), comparison.differences.end(), [](const auto& difference) {
        return difference.path == "scenario/route/starting_gold" && difference.numeric_delta == 30.0;
    }));
    REQUIRE(std::any_of(comparison.differences.begin(), comparison.differences.end(), [](const auto& difference) {
        return difference.path == "scenario/route/steps/0/gold_delta" && difference.numeric_delta == 20.0;
    }));

    const auto report = history.exportReviewReport("balance.r1", "balance.r2");
    REQUIRE(report["schema"] == "urpg.economy_revision_review.v1");
    REQUIRE(report["success"] == true);
    REQUIRE(report["before"]["content_hash"] == history.find("balance.r1")->content_hash);
    REQUIRE(report["after"]["simulation"]["seed"] == 99);
    REQUIRE(report["differences"].size() == comparison.differences.size());
    REQUIRE(report.dump() == history.exportReviewReport("balance.r1", "balance.r2").dump());

    const auto path = std::filesystem::temp_directory_path() /
                      ("urpg-economy-review-" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".json");
    std::string diagnostic;
    REQUIRE(history.writeReviewReport(path, "balance.r1", "balance.r2", &diagnostic));
    REQUIRE(diagnostic.empty());
    std::ifstream input(path, std::ios::binary);
    REQUIRE(nlohmann::json::parse(input) == report);
    input.close();
    std::filesystem::remove(path);
}

TEST_CASE("Economy revision restore uses composite project command undo and redo",
          "[balance][economy][history][pcq454]") {
    urpg::balance::EconomyRevisionHistory history;
    const auto first = scenario("v1", 20, 40);
    auto current = scenario("v2", 50, 60);
    REQUIRE(history.commit("balance.r1", "Initial economy", 1, first));
    uint64_t currentRevision = 2;
    auto participant = history.makeRestoreParticipant("balance.r1", current, currentRevision);
    REQUIRE(participant.has_value());

    urpg::project::ProjectOperationCoordinator coordinator;
    const auto restored = coordinator.execute({"balance.restore.r1", "Restore Initial economy", {*participant}});
    REQUIRE(restored.success);
    REQUIRE(restored.committed_owners == std::vector<std::string>{"balance.economy"});
    REQUIRE(current.version == "v1");
    REQUIRE(current.route.starting_gold == 20);
    REQUIRE(currentRevision == 3);
    REQUIRE(coordinator.undoLabel() == "Restore Initial economy");

    REQUIRE(coordinator.undoLast().success);
    REQUIRE(current.version == "v2");
    REQUIRE(current.route.starting_gold == 50);
    REQUIRE(currentRevision == 4);
    REQUIRE(coordinator.redoLast().success);
    REQUIRE(current.version == "v1");
    REQUIRE(currentRevision == 5);
}

TEST_CASE("Economy revision restore supplies durable recovery snapshots",
          "[balance][economy][history][project][operation_journal]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_economy_restore_journal_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    urpg::project::ProjectOperationJournal journal(root / "journal.json");
    urpg::project::ProjectOperationCoordinator coordinator(&journal);
    urpg::balance::EconomyRevisionHistory history;
    const auto first = scenario("v1", 20, 40);
    auto current = scenario("v2", 50, 60);
    REQUIRE(history.commit("balance.r1", "Initial economy", 1, first));
    uint64_t currentRevision = 2;
    auto participant = history.makeRestoreParticipant("balance.r1", current, currentRevision);
    REQUIRE(participant.has_value());
    REQUIRE(participant->recovery_snapshot);

    REQUIRE(coordinator.execute({"balance.restore.journaled", "Restore Initial economy", {*participant}}).success);
    const auto recovered = journal.recover();
    REQUIRE(recovered.success);
    REQUIRE(recovered.has_acknowledged_operation);
    REQUIRE(recovered.last_acknowledged.owners.size() == 1);
    REQUIRE(recovered.last_acknowledged.owners[0].snapshot.find("starting_gold") != std::string::npos);
    REQUIRE(recovered.last_acknowledged.owners[0].snapshot.find("20") != std::string::npos);
    std::filesystem::remove_all(root, error);
}

TEST_CASE("Economy history rejects invalid duplicate missing and stale restoration requests",
          "[balance][economy][history][pcq454]") {
    urpg::balance::EconomyRevisionHistory history;
    std::string diagnostic;
    REQUIRE_FALSE(history.commit("", "Missing", 1, scenario("v1", 20, 40), &diagnostic));
    REQUIRE(diagnostic == "economy_revision_invalid");
    REQUIRE(history.commit("balance.r1", "Initial", 1, scenario("v1", 20, 40)));
    REQUIRE_FALSE(history.commit("balance.r1", "Duplicate", 2, scenario("v2", 50, 60), &diagnostic));
    REQUIRE(diagnostic == "economy_revision_duplicate");
    REQUIRE_FALSE(history.compare("balance.r1", "missing").success);
    urpg::balance::EconomyScenario missingTarget;
    uint64_t missingRevision = 1;
    REQUIRE_FALSE(history.makeRestoreParticipant("missing", missingTarget, missingRevision).has_value());

    auto current = scenario("v2", 50, 60);
    uint64_t currentRevision = 2;
    auto participant = history.makeRestoreParticipant("balance.r1", current, currentRevision);
    REQUIRE(participant.has_value());
    ++currentRevision;
    urpg::project::ProjectOperationCoordinator coordinator;
    const auto stale = coordinator.execute({"balance.restore.stale", "Stale restore", {*participant}});
    REQUIRE_FALSE(stale.success);
    REQUIRE(stale.code == "project_operation_source_revision_mismatch");
    REQUIRE(current.version == "v2");
}
