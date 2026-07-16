#include "engine/core/scene/first_run_calibration.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

std::filesystem::path calibrationSettingsPath(const char* name) {
    const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           (std::string(name) + "_" + std::to_string(tick)) / "runtime.json";
}

void advanceToReview(urpg::scene::FirstRunCalibrationFlow& flow) {
    while (flow.snapshot().step != urpg::scene::CalibrationStep::Review) {
        REQUIRE(flow.next().success);
    }
}

} // namespace

TEST_CASE("Fresh profile calibration covers text audio display input and accessibility",
          "[scene][runtime][calibration][pcq606]") {
    const auto path = calibrationSettingsPath("urpg_calibration_complete");
    std::filesystem::remove_all(path.parent_path());
    float previewedVolume = -1.0F;
    bool applied = false;
    urpg::scene::FirstRunCalibrationFlow flow(
        urpg::settings::defaultRuntimeSettings(), path,
        {[&previewedVolume](const float volume) { previewedVolume = volume; },
         [&applied](const urpg::settings::RuntimeSettings&) { applied = true; }});

    REQUIRE(flow.shouldRunForFreshProfile());
    REQUIRE(flow.begin().success);
    REQUIRE(flow.snapshot().step == urpg::scene::CalibrationStep::Welcome);
    flow.setTextScale(1.4F);
    flow.setMasterVolume(0.35F);
    flow.setDisplay(1920, 1080, true, 0.9F);
    flow.setInputDevice(urpg::scene::CalibrationInputDevice::Controller);
    flow.setAccessibility(true, true, false);
    REQUIRE(previewedVolume == Catch::Approx(0.35F));
    REQUIRE_FALSE(flow.finish().success);

    advanceToReview(flow);
    REQUIRE(flow.snapshot().can_finish);
    REQUIRE_FALSE(flow.next().success);
    REQUIRE(flow.finish().success);
    REQUIRE(applied);
    REQUIRE_FALSE(flow.shouldRunForFreshProfile());

    const auto loaded = urpg::settings::loadRuntimeSettings(path);
    REQUIRE(loaded.report.loaded);
    REQUIRE(loaded.settings.accessibility.text_scale == Catch::Approx(1.4F));
    REQUIRE(loaded.settings.audio.master_volume == Catch::Approx(0.35F));
    REQUIRE(loaded.settings.window.width == 1920);
    REQUIRE(loaded.settings.window.height == 1080);
    REQUIRE(loaded.settings.window.fullscreen);
    REQUIRE(loaded.settings.window.safe_area_scale == Catch::Approx(0.9F));
    REQUIRE(loaded.settings.calibration.preferred_input_device == "controller");
    REQUIRE(loaded.settings.accessibility.high_contrast);
    REQUIRE(loaded.settings.accessibility.reduce_motion);
    REQUIRE_FALSE(loaded.settings.accessibility.shortcuts_enabled);
    REQUIRE(loaded.settings.calibration.completed);
    REQUIRE_FALSE(loaded.settings.calibration.skipped);
    REQUIRE(loaded.settings.calibration.revision == 1);

    std::filesystem::remove_all(path.parent_path());
}

TEST_CASE("Skipped calibration remains replayable and replay cancellation restores settings",
          "[scene][runtime][calibration][pcq606]") {
    const auto path = calibrationSettingsPath("urpg_calibration_replay");
    std::filesystem::remove_all(path.parent_path());
    urpg::scene::FirstRunCalibrationFlow first(urpg::settings::defaultRuntimeSettings(), path);
    REQUIRE(first.begin().success);
    REQUIRE(first.skip().success);
    REQUIRE(first.settings().calibration.completed);
    REQUIRE(first.settings().calibration.skipped);

    const auto loaded = urpg::settings::loadRuntimeSettings(path);
    urpg::scene::FirstRunCalibrationFlow replay(loaded.settings, path);
    REQUIRE_FALSE(replay.shouldRunForFreshProfile());
    REQUIRE_FALSE(replay.begin().success);
    REQUIRE(replay.begin(true).success);
    replay.setTextScale(1.75F);
    REQUIRE(replay.cancelReplay().success);
    REQUIRE(replay.settings().accessibility.text_scale == Catch::Approx(1.0F));
    REQUIRE(replay.settings().calibration.skipped);

    REQUIRE(replay.begin(true).success);
    replay.setTextScale(1.25F);
    advanceToReview(replay);
    REQUIRE(replay.finish().success);
    REQUIRE_FALSE(replay.settings().calibration.skipped);
    REQUIRE(replay.settings().accessibility.text_scale == Catch::Approx(1.25F));

    std::filesystem::remove_all(path.parent_path());
}

TEST_CASE("Calibration clamps unsafe values and rejects invalid transitions",
          "[scene][runtime][calibration][pcq606]") {
    const auto path = calibrationSettingsPath("urpg_calibration_bounds");
    urpg::scene::FirstRunCalibrationFlow flow(urpg::settings::defaultRuntimeSettings(), path);
    REQUIRE_FALSE(flow.next().success);
    REQUIRE_FALSE(flow.skip().success);
    REQUIRE(flow.begin().success);
    REQUIRE_FALSE(flow.back().success);
    REQUIRE_FALSE(flow.begin().success);

    flow.setTextScale(10.0F);
    flow.setMasterVolume(-2.0F);
    flow.setDisplay(1, 99999, false, 0.1F);
    const auto snapshot = flow.snapshot();
    REQUIRE(snapshot.settings.accessibility.text_scale == Catch::Approx(2.0F));
    REQUIRE(snapshot.settings.audio.master_volume == Catch::Approx(0.0F));
    REQUIRE(snapshot.settings.window.width == 320);
    REQUIRE(snapshot.settings.window.height == 16384);
    REQUIRE(snapshot.settings.window.safe_area_scale == Catch::Approx(0.8F));

    REQUIRE(flow.skip().success);
    REQUIRE_FALSE(flow.cancelReplay().success);
    std::filesystem::remove_all(path.parent_path());
}
