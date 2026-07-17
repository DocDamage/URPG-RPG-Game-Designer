#include "engine/core/replay/scenario_replay.h"
#include "engine/core/playtest/playtest_scenario_replay_runtime.h"
#include "editor/replay/replay_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

TEST_CASE("Scenario replay captures semantic inputs metadata checkpoints and redaction", "[replay][scenario][pcq504]") {
    using namespace urpg::replay;
    ScenarioReplayCapture capture({77, "project-r42", "runtime-2.4.1", {"player_name"}, 3, 2});
    REQUIRE(capture.recordInput(0, SemanticInputKind::Move, "move_north",
                                {{"tile", "4,8"}, {"token", "secret-value"}}, {{"x", 4}, {"y", 8}}).success);
    REQUIRE(capture.recordInput(2, SemanticInputKind::Interact, "talk",
                                {{"event", "guide"}, {"player_name", "Ada"}}, {{"dialogue", 1}}).success);
    REQUIRE(capture.captureCheckpoint(2, "before_choice",
                                      {{"dialogue", 1}, {"email", "ada@example.test"}}).success);
    REQUIRE_FALSE(capture.recordInput(1, SemanticInputKind::Confirm, "late", {}, {}).success);

    const auto artifact = capture.finish("reported-failure");
    REQUIRE(artifact.seed == 77);
    REQUIRE(artifact.project_revision == "project-r42");
    REQUIRE(artifact.runtime_version == "runtime-2.4.1");
    REQUIRE(artifact.input_log[0].semantic_kind == "move");
    REQUIRE(artifact.input_log[0].payload["token"] == "[REDACTED]");
    REQUIRE(artifact.input_log[1].payload["player_name"] == "[REDACTED]");
    REQUIRE(artifact.checkpoints[0].redacted_state["email"] == "[REDACTED]");
    REQUIRE(artifact.redacted_fields.size() == 3);

    const auto round_trip = ReplayArtifact::fromJson(artifact.toJson());
    REQUIRE(round_trip.input_log == artifact.input_log);
    REQUIRE(round_trip.checkpoints == artifact.checkpoints);
    REQUIRE(round_trip.redacted_fields == artifact.redacted_fields);
}

TEST_CASE("Scenario replay succeeds in headless and interactive modes", "[replay][scenario][pcq504]") {
    using namespace urpg::replay;
    ScenarioReplayCapture capture({9, "rev-a", "runtime-a", {}, 100000, 64});
    REQUIRE(capture.recordInput(0, SemanticInputKind::Confirm, "start", {}, {{"stage", 1}}).success);
    REQUIRE(capture.recordInput(5, SemanticInputKind::Menu, "select_item", {{"item", "potion"}},
                                {{"stage", 2}}).success);
    const auto artifact = capture.finish("scenario");
    const auto executor = [](const ReplayInput& input, const uint64_t seed, const ReplayExecutionMode mode)
        -> std::optional<nlohmann::json> {
        REQUIRE(seed == 9);
        REQUIRE((mode == ReplayExecutionMode::Headless || mode == ReplayExecutionMode::Interactive));
        return input.tick == 0 ? nlohmann::json{{"stage", 1}} : nlohmann::json{{"stage", 2}};
    };

    const auto headless = ScenarioReplayRunner::run({artifact, ReplayExecutionMode::Headless, "rev-a", "runtime-a"}, executor);
    const auto interactive = ScenarioReplayRunner::run({artifact, ReplayExecutionMode::Interactive, "rev-a", "runtime-a"}, executor);
    REQUIRE(headless.success);
    REQUIRE(headless.code == "replay_completed");
    REQUIRE(headless.applied_inputs == 2);
    REQUIRE(interactive.success);
    REQUIRE(interactive.message.find("interactive") != std::string::npos);
}

TEST_CASE("Scenario replay clearly reports environment input and state divergence", "[replay][scenario][pcq504]") {
    using namespace urpg::replay;
    ScenarioReplayCapture capture({11, "rev-b", "runtime-b", {}, 100000, 64});
    REQUIRE(capture.recordInput(3, SemanticInputKind::Interact, "open", {}, {{"door", "open"}}).success);
    const auto artifact = capture.finish("failure");

    const auto environment = ScenarioReplayRunner::run(
        {artifact, ReplayExecutionMode::Headless, "wrong", "runtime-b"}, {});
    REQUIRE_FALSE(environment.success);
    REQUIRE(environment.code == "replay_environment_mismatch");

    const auto unsupported = ScenarioReplayRunner::run(
        {artifact, ReplayExecutionMode::Headless, "rev-b", "runtime-b"},
        [](const auto&, auto, auto) -> std::optional<nlohmann::json> { return std::nullopt; });
    REQUIRE(unsupported.code == "replay_semantic_input_unsupported");
    REQUIRE(unsupported.message.find("tick 3") != std::string::npos);

    const auto diverged = ScenarioReplayRunner::run(
        {artifact, ReplayExecutionMode::Interactive, "rev-b", "runtime-b"},
        [](const auto&, auto, auto) -> std::optional<nlohmann::json> { return nlohmann::json{{"door", "closed"}}; });
    REQUIRE(diverged.code == "replay_diverged");
    REQUIRE(diverged.divergence);
    REQUIRE(diverged.divergence->first_mismatched_tick == 3);
    REQUIRE_FALSE(diverged.divergence->expected_hash.empty());
    REQUIRE_FALSE(diverged.divergence->actual_hash.empty());
}

TEST_CASE("Live playtest scenario workflow captures persists and replays a reported failure",
          "[replay][scenario][pcq504][live]") {
    using namespace urpg;
    const auto root = std::filesystem::temp_directory_path() /
        ("urpg_pcq504_live_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);

    playtest::PlaytestScenarioReplayRuntime runtime(
        root, replay::ScenarioReplayCaptureConfig{42, "project-revision-a", "runtime-a", {}, 100000, 64});
    std::string diagnostic;
    REQUIRE(runtime.start("session-a", {{"player_x", 0}}, &diagnostic));
    runtime.observeInput(input::InputAction::MoveRight, input::ActionState::Pressed);
    REQUIRE(runtime.finishFrame({{"player_x", 1}}, &diagnostic));

    editor::ReplayPanel panel;
    panel.bindLiveSession(root);
    REQUIRE(panel.refreshLive(&diagnostic));
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().live_connected);
    REQUIRE(panel.lastRenderSnapshot().semantic_input_count == 1);
    REQUIRE(panel.lastRenderSnapshot().checkpoint_count == 1);
    REQUIRE(panel.lastRenderSnapshot().project_revision == "project-revision-a");
    REQUIRE(std::filesystem::is_regular_file(root / "replay.json"));

    REQUIRE(panel.requestLiveReplay(replay::ReplayExecutionMode::Headless, &diagnostic));
    REQUIRE(runtime.poll([](const replay::ReplayArtifact& artifact, const replay::ReplayExecutionMode mode) {
        return replay::ScenarioReplayRunner::run(
            {artifact, mode, "project-revision-a", "runtime-a"},
            [](const replay::ReplayInput& semantic, uint64_t,
               replay::ReplayExecutionMode) -> std::optional<nlohmann::json> {
                if (semantic.action != "move_right") return std::nullopt;
                return nlohmann::json{{"player_x", 1}};
            });
    }, &diagnostic));
    REQUIRE(panel.refreshLive(&diagnostic));
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().last_result.success);
    REQUIRE(panel.lastRenderSnapshot().last_result.code == "replay_completed");
    REQUIRE(panel.lastRenderSnapshot().last_result.mode == replay::ReplayExecutionMode::Headless);

    REQUIRE(panel.requestLiveReplay(replay::ReplayExecutionMode::Interactive, &diagnostic));
    REQUIRE(runtime.poll([](const replay::ReplayArtifact& artifact, const replay::ReplayExecutionMode mode) {
        return replay::ScenarioReplayRunner::run(
            {artifact, mode, "project-revision-a", "runtime-a"},
            [](const replay::ReplayInput&, uint64_t,
               replay::ReplayExecutionMode) -> std::optional<nlohmann::json> {
                return nlohmann::json{{"player_x", 9}};
            });
    }, &diagnostic));
    REQUIRE(panel.refreshLive(&diagnostic));
    panel.render();
    REQUIRE_FALSE(panel.lastRenderSnapshot().last_result.success);
    REQUIRE(panel.lastRenderSnapshot().last_result.code == "replay_diverged");
    REQUIRE(panel.lastRenderSnapshot().last_result.divergence.has_value());
    REQUIRE(panel.lastRenderSnapshot().last_result.divergence->first_mismatched_tick == 0);

    std::error_code cleanup_error;
    std::filesystem::remove_all(root, cleanup_error);
}
