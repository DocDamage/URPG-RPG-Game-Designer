#include "engine/core/replay/replay_gallery.h"
#include "engine/core/replay/golden_replay_lane.h"
#include "engine/core/replay/replay_player.h"
#include "engine/core/replay/replay_recorder.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Replay artifact records seed inputs labels version and stable state hashes", "[replay][ffs11]") {
    urpg::replay::ReplayRecorder recorder(42, "1.2.3");
    recorder.addLabel("smoke");
    recorder.recordInput(0, "press_accept", {{"button", "ok"}}, {{"party_gold", 10}});
    recorder.recordInput(1, "move_down", {}, {{"party_gold", 10}, {"map", "town"}});

    const auto first = recorder.finish("intro");
    const auto second = urpg::replay::ReplayArtifact::fromJson(first.toJson());

    REQUIRE(first.seed == 42);
    REQUIRE(first.project_version == "1.2.3");
    REQUIRE(first.labels.count("smoke") == 1);
    REQUIRE(first.input_log.size() == 2);
    REQUIRE(first.state_hashes == second.state_hashes);
}

TEST_CASE("Replay player reports first mismatched tick and restores bounded checkpoints", "[replay][ffs11]") {
    urpg::replay::ReplayRecorder expected_recorder(7, "1.0.0");
    expected_recorder.recordInput(0, "start", {}, {{"hp", 10}});
    expected_recorder.recordInput(4, "attack", {}, {{"hp", 8}});
    const auto expected = expected_recorder.finish("battle");

    urpg::replay::ReplayRecorder actual_recorder(7, "1.0.0");
    actual_recorder.recordInput(0, "start", {}, {{"hp", 10}});
    actual_recorder.recordInput(4, "attack", {}, {{"hp", 9}});
    const auto actual = actual_recorder.finish("battle");

    const auto comparison = urpg::replay::ReplayPlayer::compare(expected, actual);
    REQUIRE_FALSE(comparison.matches);
    REQUIRE(comparison.first_mismatched_tick == 4);

    urpg::replay::ReplayPlayer player(2);
    player.captureCheckpoint(0, {{"hp", 10}});
    player.captureCheckpoint(4, {{"hp", 8}});
    player.captureCheckpoint(8, {{"hp", 3}});

    const auto restored = player.restoreCheckpoint(4);
    REQUIRE(restored.has_value());
    REQUIRE((*restored)["hp"] == 8);
    REQUIRE_FALSE(player.restoreCheckpoint(0).has_value());
}

TEST_CASE("Replay gallery sorts artifacts deterministically by label and id", "[replay][ffs11]") {
    urpg::replay::ReplayGallery gallery;
    urpg::replay::ReplayArtifact zeta;
    zeta.id = "zeta";
    zeta.seed = 1;
    zeta.project_version = "1.0.0";
    zeta.labels = {"nightly"};
    urpg::replay::ReplayArtifact alpha;
    alpha.id = "alpha";
    alpha.seed = 1;
    alpha.project_version = "1.0.0";
    alpha.labels = {"golden"};
    urpg::replay::ReplayArtifact beta;
    beta.id = "beta";
    beta.seed = 1;
    beta.project_version = "1.0.0";
    beta.labels = {"golden"};
    gallery.add(zeta);
    gallery.add(alpha);
    gallery.add(beta);

    const auto golden = gallery.findByLabel("golden");

    REQUIRE(golden.size() == 2);
    REQUIRE(golden[0].id == "alpha");
    REQUIRE(golden[1].id == "beta");
}

TEST_CASE("Golden replay lane fails on first divergent state hash", "[replay][golden][ffs11]") {
    urpg::replay::ReplayRecorder expected_recorder(7, "1.0.0");
    expected_recorder.addLabel("golden");
    expected_recorder.recordInput(0, "start", {}, {{"hp", 10}});
    expected_recorder.recordInput(3, "hit", {}, {{"hp", 8}});
    const auto expected = expected_recorder.finish("boss_intro");

    urpg::replay::ReplayRecorder actual_recorder(7, "1.0.0");
    actual_recorder.addLabel("golden");
    actual_recorder.recordInput(0, "start", {}, {{"hp", 10}});
    actual_recorder.recordInput(3, "hit", {}, {{"hp", 9}});
    const auto actual = actual_recorder.finish("boss_intro");

    const auto report = urpg::replay::GoldenReplayLane::compare({expected}, {actual}, "1.0.0");

    REQUIRE_FALSE(report.passed);
    REQUIRE(report.results.size() == 1);
    REQUIRE(report.results[0].id == "boss_intro");
    REQUIRE(report.results[0].status == urpg::replay::GoldenReplayStatus::Diverged);
    REQUIRE(report.results[0].first_mismatched_tick == 3);
}

TEST_CASE("Golden replay lane reports missing and stale replay artifacts", "[replay][golden][ffs11]") {
    urpg::replay::ReplayArtifact missing;
    missing.id = "missing";
    missing.seed = 1;
    missing.project_version = "1.0.0";
    missing.labels = {"golden"};

    urpg::replay::ReplayArtifact stale;
    stale.id = "stale";
    stale.seed = 1;
    stale.project_version = "0.9.0";
    stale.labels = {"golden"};

    const auto report = urpg::replay::GoldenReplayLane::compare({missing, stale}, {stale}, "1.0.0");

    REQUIRE_FALSE(report.passed);
    REQUIRE(report.results.size() == 2);
    REQUIRE(report.results[0].id == "missing");
    REQUIRE(report.results[0].status == urpg::replay::GoldenReplayStatus::MissingActual);
    REQUIRE(report.results[1].id == "stale");
    REQUIRE(report.results[1].status == urpg::replay::GoldenReplayStatus::StaleProjectVersion);
}
