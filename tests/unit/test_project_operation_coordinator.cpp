#include "engine/core/project/project_operation_coordinator.h"

#include <catch2/catch_test_macros.hpp>

using urpg::project::ProjectOperationCoordinator;
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
            }};
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
