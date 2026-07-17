#include "engine/core/project/project_operation_coordinator.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

using urpg::project::ProjectOperationCoordinator;
using urpg::project::ProjectOperationJournal;
using urpg::project::ProjectOperationParticipant;
using urpg::project::ProjectOperationRequest;

namespace {

struct OwnerState {
    int value = 0;
    int before = 0;
    uint64_t revision = 1;
    bool fail_commit = false;
};

ProjectOperationParticipant participant(std::string id, OwnerState& state, const int nextValue) {
    const auto documentPath = std::filesystem::path("content") / (id + ".json");
    return {std::move(id),
            state.revision,
            [&] { return state.revision; },
            [&](std::string&) {
                state.before = state.value;
                return true;
            },
            [&, nextValue](std::string& diagnostic) {
                state.value = nextValue;
                ++state.revision;
                if (state.fail_commit) {
                    diagnostic = "injected owner commit failure";
                    return false;
                }
                return true;
            },
            [&] {
                state.value = state.before;
                --state.revision;
            },
            [&](std::string&) {
                state.value = state.before;
                ++state.revision;
                return true;
            },
            [&] { return std::to_string(state.value); },
            documentPath};
}

} // namespace

TEST_CASE("ProjectOperationCoordinator fully commits and replays one multi-owner intent",
          "[project][operation_coordinator]") {
    OwnerState map{1};
    OwnerState assets{2};
    ProjectOperationCoordinator coordinator;
    ProjectOperationRequest request{"op.replace-prop", "Replace prop asset",
                                    {participant("map", map, 10), participant("assets", assets, 20)}};
    const auto committed = coordinator.execute(request);
    REQUIRE(committed.success);
    REQUIRE(map.value == 10);
    REQUIRE(assets.value == 20);
    REQUIRE(coordinator.historySize() == 1);
    const auto replayed = coordinator.execute(request);
    REQUIRE(replayed.success);
    REQUIRE(replayed.replayed);
    REQUIRE(map.value == 10);
    REQUIRE(assets.value == 20);
}

TEST_CASE("ProjectOperationCoordinator rolls back every owner after commit failure",
          "[project][operation_coordinator]") {
    OwnerState map{1};
    OwnerState assets{2};
    assets.fail_commit = true;
    ProjectOperationCoordinator coordinator;
    const auto result = coordinator.execute({"op.fail", "Injected failure",
                                             {participant("map", map, 10), participant("assets", assets, 20)}});
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "project_operation_commit_failed");
    REQUIRE(map.value == 1);
    REQUIRE(assets.value == 2);
    REQUIRE(map.revision == 1);
    REQUIRE(assets.revision == 1);
    REQUIRE(coordinator.historySize() == 0);
}

TEST_CASE("ProjectOperationCoordinator rejects stale source and stale inverse revisions",
          "[project][operation_coordinator]") {
    OwnerState map{1};
    ProjectOperationCoordinator coordinator;
    auto planned = participant("map", map, 10);
    ++map.revision;
    REQUIRE(coordinator.execute({"op.stale", "Stale", {planned}}).code ==
            "project_operation_source_revision_mismatch");
    --map.revision;
    REQUIRE(coordinator.execute({"op.commit", "Commit", {participant("map", map, 10)}}).success);
    ++map.revision;
    const auto undo = coordinator.undoLast();
    REQUIRE_FALSE(undo.success);
    REQUIRE(undo.code == "project_operation_inverse_revision_mismatch");
    REQUIRE(map.value == 10);
}

TEST_CASE("ProjectOperationCoordinator exposes one labeled composite undo and redo boundary",
          "[project][operation_coordinator]") {
    OwnerState map{1};
    OwnerState dialogue{2};
    ProjectOperationCoordinator coordinator;
    REQUIRE(coordinator.execute({"op.rename-intro", "Rename intro dialogue",
                                 {participant("map", map, 10), participant("dialogue", dialogue, 20)}})
                .success);
    REQUIRE(coordinator.undoLabel() == "Rename intro dialogue");
    REQUIRE(coordinator.redoLabel().empty());
    REQUIRE(coordinator.undoLast().success);
    REQUIRE(map.value == 1);
    REQUIRE(dialogue.value == 2);
    REQUIRE(coordinator.historySize() == 0);
    REQUIRE(coordinator.redoSize() == 1);
    REQUIRE(coordinator.redoLabel() == "Rename intro dialogue");
    REQUIRE(coordinator.redoLast().success);
    REQUIRE(map.value == 10);
    REQUIRE(dialogue.value == 20);
    REQUIRE(coordinator.historySize() == 1);
    REQUIRE(coordinator.redoSize() == 0);
}

TEST_CASE("ProjectOperationCoordinator refuses redo after an owner changes", "[project][operation_coordinator]") {
    OwnerState map{1};
    ProjectOperationCoordinator coordinator;
    REQUIRE(coordinator.execute({"op.move-map", "Move map", {participant("map", map, 10)}}).success);
    REQUIRE(coordinator.undoLast().success);
    ++map.revision;
    const auto redo = coordinator.redoLast();
    REQUIRE_FALSE(redo.success);
    REQUIRE(redo.code == "project_operation_redo_revision_mismatch");
    REQUIRE(map.value == 1);
    REQUIRE(coordinator.redoSize() == 1);
}

TEST_CASE("ProjectOperationCoordinator journals real prepare and acknowledged commit boundaries",
          "[project][operation_coordinator][operation_journal]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_coordinator_journal_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    ProjectOperationJournal journal(root / "journal.json");
    ProjectOperationCoordinator coordinator(&journal);
    OwnerState map{1};
    OwnerState dialogue{2};

    const auto result = coordinator.execute({"op.journaled", "Journalled operation",
        {participant("map", map, 10), participant("dialogue", dialogue, 20)}});
    REQUIRE(result.success);
    const auto recovered = journal.recover();
    REQUIRE(recovered.success);
    REQUIRE(recovered.code == "project_operation_recovery_clean");
    REQUIRE(recovered.has_acknowledged_operation);
    REQUIRE(recovered.last_acknowledged.operation_id == "op.journaled");
    REQUIRE(recovered.last_acknowledged.owners.size() == 2);
    REQUIRE(recovered.last_acknowledged.owners[0].snapshot == "10");
    REQUIRE(recovered.last_acknowledged.owners[1].snapshot == "20");
    REQUIRE_FALSE(recovered.last_acknowledged.owners[0].document_path.empty());
    std::filesystem::remove_all(root, error);
}

TEST_CASE("ProjectOperationCoordinator refuses durable mutation without owner recovery snapshots",
          "[project][operation_coordinator][operation_journal]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_coordinator_snapshot_required_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    ProjectOperationJournal journal(root / "journal.json");
    ProjectOperationCoordinator coordinator(&journal);
    OwnerState map{1};
    auto owner = participant("map", map, 10);
    owner.recovery_snapshot = {};
    const auto result = coordinator.execute({"op.no-snapshot", "Unsafe operation", {owner}});
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "project_operation_participant_invalid");
    REQUIRE(map.value == 1);
    REQUIRE_FALSE(std::filesystem::exists(root / "journal.json"));
    std::filesystem::remove_all(root, error);
}

TEST_CASE("ProjectOperationCoordinator aborts durable prepare after owner commit failure and can retry",
          "[project][operation_coordinator][operation_journal]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_coordinator_journal_abort_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    ProjectOperationJournal journal(root / "journal.json");
    ProjectOperationCoordinator coordinator(&journal);
    OwnerState map{1};
    OwnerState dialogue{2};
    dialogue.fail_commit = true;
    auto request = ProjectOperationRequest{"op.retry", "Retry operation",
        {participant("map", map, 10), participant("dialogue", dialogue, 20)}};
    REQUIRE_FALSE(coordinator.execute(request).success);
    REQUIRE(journal.recover().code == "project_operation_recovery_clean");
    REQUIRE(map.value == 1);
    REQUIRE(dialogue.value == 2);

    dialogue.fail_commit = false;
    request.participants = {participant("map", map, 10), participant("dialogue", dialogue, 20)};
    REQUIRE(coordinator.execute(request).success);
    REQUIRE(journal.recover().has_acknowledged_operation);
    std::filesystem::remove_all(root, error);
}

TEST_CASE("ProjectOperationCoordinator rolls back and closes prepare when journal acknowledgement fails",
          "[project][operation_coordinator][operation_journal]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_coordinator_ack_failure_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    int replacements = 0;
    ProjectOperationJournal journal(root / "journal.json", [&replacements] {
        ++replacements;
        return replacements != 2;
    });
    ProjectOperationCoordinator coordinator(&journal);
    OwnerState map{1};

    const auto failed = coordinator.execute({"op.ack-failure", "Ack failure", {participant("map", map, 10)}});
    REQUIRE_FALSE(failed.success);
    REQUIRE(failed.code == "project_operation_journal_commit_failed");
    REQUIRE(map.value == 1);
    REQUIRE(coordinator.historySize() == 0);
    const auto recovered = journal.recover();
    REQUIRE(recovered.success);
    REQUIRE(recovered.code == "project_operation_recovery_clean");
    REQUIRE_FALSE(recovered.has_acknowledged_operation);
    std::filesystem::remove_all(root, error);
}
