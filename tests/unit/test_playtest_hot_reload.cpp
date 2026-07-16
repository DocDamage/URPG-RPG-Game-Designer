#include "editor/playtest/playtest_hot_reload.h"

#include <catch2/catch_test_macros.hpp>

namespace {

nlohmann::json runtimeCapabilities() {
    auto rows = nlohmann::json::array();
    for (const auto& capability : urpg::editor::PlaytestHotReloadCoordinator::productCapabilityMatrix()) {
        rows.push_back({{"resource_class", urpg::editor::playtestResourceClassName(capability.resource_class)},
                        {"behavior", urpg::editor::playtestReloadBehaviorName(capability.behavior)},
                        {"supports_state_preservation", capability.supports_state_preservation}});
    }
    return {{"schema", "urpg.playtest_reload_capabilities.v1"}, {"capabilities", rows}};
}

} // namespace

TEST_CASE("Playtest reload negotiation exposes bounded product behavior matrix",
          "[playtest][hot_reload][pcq501]") {
    urpg::editor::PlaytestHotReloadCoordinator coordinator([](const auto&) {
        return urpg::editor::PlaytestReloadRuntimeAck{true, false, "ok", "accepted"};
    });
    REQUIRE(coordinator.negotiate(runtimeCapabilities()));
    const auto& matrix = coordinator.negotiatedCapabilities();
    REQUIRE(matrix.size() == 9);
    REQUIRE(matrix[0].behavior == urpg::editor::PlaytestReloadBehavior::HotReloadable);
    REQUIRE_FALSE(matrix[0].supports_state_preservation);
    REQUIRE(matrix[2].behavior == urpg::editor::PlaytestReloadBehavior::RestartRequired);
    REQUIRE(matrix[4].supports_state_preservation);
    REQUIRE(matrix[7].behavior == urpg::editor::PlaytestReloadBehavior::Rejected);

    auto reduced = runtimeCapabilities();
    reduced["capabilities"].erase(reduced["capabilities"].begin());
    REQUIRE(coordinator.negotiate(reduced));
    REQUIRE(coordinator.negotiatedCapabilities()[0].behavior == urpg::editor::PlaytestReloadBehavior::Rejected);
    REQUIRE_FALSE(coordinator.negotiate({{"schema", "bad"}}));
}

TEST_CASE("Playtest hot reload validates state policy revision and bounded payload",
          "[playtest][hot_reload][pcq501]") {
    urpg::editor::PlaytestHotReloadCoordinator coordinator([](const auto&) {
        return urpg::editor::PlaytestReloadRuntimeAck{true, false, "ok", "accepted"};
    });
    REQUIRE(coordinator.negotiate(runtimeCapabilities()));
    urpg::editor::PlaytestHotReloadRequest map{"reload.map", urpg::editor::PlaytestResourceClass::Map,
        "map.willow", "{}", 0, urpg::editor::PlaytestReloadStatePolicy::Preserve, true};
    REQUIRE(coordinator.preview(map).code == "playtest_reload_preserve_unsupported");
    map.state_policy = urpg::editor::PlaytestReloadStatePolicy::ResetAffected;
    REQUIRE(coordinator.preview(map).valid);
    REQUIRE(coordinator.execute(map).status == urpg::editor::PlaytestReloadStatus::Applied);
    REQUIRE(coordinator.revision(map.resource_class, map.resource_id) == 1);
    REQUIRE(coordinator.preview(map).code == "playtest_reload_revision_mismatch");

    auto invalid = map;
    invalid.request_id = "invalid";
    invalid.resource_id = "../escape";
    invalid.expected_revision = 1;
    REQUIRE(invalid.resource_id.size() > 0);
    REQUIRE(coordinator.preview(invalid).code == "playtest_reload_request_invalid");
    invalid.resource_id = "map.willow";
    invalid.content = "not-json";
    REQUIRE(coordinator.preview(invalid).code == "playtest_reload_payload_invalid");
    invalid.content.assign(urpg::editor::PlaytestHotReloadCoordinator::kMaxReloadBytes + 1, 'x');
    REQUIRE(coordinator.preview(invalid).code == "playtest_reload_payload_too_large");
}

TEST_CASE("Playtest reload restarts required resources and rejects unsafe classes",
          "[playtest][hot_reload][pcq501]") {
    std::vector<urpg::editor::PlaytestReloadAction> actions;
    urpg::editor::PlaytestHotReloadCoordinator coordinator([&](const auto& command) {
        actions.push_back(command.action);
        return urpg::editor::PlaytestReloadRuntimeAck{true, false, "ok", "accepted"};
    });
    REQUIRE(coordinator.negotiate(runtimeCapabilities()));
    const urpg::editor::PlaytestHotReloadRequest quest{"reload.quest",
        urpg::editor::PlaytestResourceClass::Quest, "quest.wisp", "{}", 0,
        urpg::editor::PlaytestReloadStatePolicy::ResetAll, true};
    const auto restarted = coordinator.execute(quest);
    REQUIRE(restarted.status == urpg::editor::PlaytestReloadStatus::RestartQueued);
    REQUIRE(restarted.restart_fallback);
    REQUIRE(actions == std::vector<urpg::editor::PlaytestReloadAction>{urpg::editor::PlaytestReloadAction::Restart});

    auto plugin = quest;
    plugin.request_id = "reload.plugin";
    plugin.resource_class = urpg::editor::PlaytestResourceClass::Plugin;
    plugin.resource_id = "plugin.community";
    plugin.content = "script";
    REQUIRE(coordinator.execute(plugin).code == "playtest_reload_resource_rejected");
}

TEST_CASE("Failed partial hot reload rolls back before safe restart fallback",
          "[playtest][hot_reload][pcq501]") {
    std::vector<urpg::editor::PlaytestReloadAction> actions;
    urpg::editor::PlaytestHotReloadCoordinator coordinator([&](const auto& command) {
        actions.push_back(command.action);
        if (command.action == urpg::editor::PlaytestReloadAction::Apply)
            return urpg::editor::PlaytestReloadRuntimeAck{false, true, "apply_failed", "partial mutation"};
        return urpg::editor::PlaytestReloadRuntimeAck{true, false, "ok", "recovered"};
    });
    REQUIRE(coordinator.negotiate(runtimeCapabilities()));
    const urpg::editor::PlaytestHotReloadRequest localization{"reload.locale",
        urpg::editor::PlaytestResourceClass::Localization, "locale.en-US", "{}", 0,
        urpg::editor::PlaytestReloadStatePolicy::Preserve, true};
    const auto result = coordinator.execute(localization);
    REQUIRE(result.status == urpg::editor::PlaytestReloadStatus::RestartQueued);
    REQUIRE(result.recovery_attempted);
    REQUIRE(result.recovered);
    REQUIRE(result.restart_fallback);
    REQUIRE(result.resulting_revision == 1);
    REQUIRE(actions == std::vector<urpg::editor::PlaytestReloadAction>{
        urpg::editor::PlaytestReloadAction::Apply,
        urpg::editor::PlaytestReloadAction::Rollback,
        urpg::editor::PlaytestReloadAction::Restart});
}

TEST_CASE("Rollback failure leaves revision unchanged and forbids unsafe restart",
          "[playtest][hot_reload][pcq501]") {
    urpg::editor::PlaytestHotReloadCoordinator coordinator([](const auto& command) {
        if (command.action == urpg::editor::PlaytestReloadAction::Apply)
            return urpg::editor::PlaytestReloadRuntimeAck{false, true, "apply_failed", "partial mutation"};
        return urpg::editor::PlaytestReloadRuntimeAck{false, true, "rollback_failed", "state uncertain"};
    });
    REQUIRE(coordinator.negotiate(runtimeCapabilities()));
    const urpg::editor::PlaytestHotReloadRequest audio{"reload.audio",
        urpg::editor::PlaytestResourceClass::Audio, "audio.theme", "bytes", 0,
        urpg::editor::PlaytestReloadStatePolicy::Preserve, true};
    const auto result = coordinator.execute(audio);
    REQUIRE(result.status == urpg::editor::PlaytestReloadStatus::FailedUnrecovered);
    REQUIRE(result.recovery_attempted);
    REQUIRE_FALSE(result.recovered);
    REQUIRE_FALSE(result.restart_fallback);
    REQUIRE(result.code == "rollback_failed");
    REQUIRE(coordinator.revision(audio.resource_class, audio.resource_id) == 0);
}
