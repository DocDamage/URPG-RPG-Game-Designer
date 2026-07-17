#include "engine/core/perf/performance_capture.h"
#include "engine/core/playtest/playtest_performance_runtime.h"
#include "engine/core/engine_shell.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/scene/scene_manager.h"
#include "editor/perf/perf_diagnostics_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <thread>

namespace {

class MeasuredHitchScene final : public urpg::scene::GameScene {
public:
    urpg::scene::SceneType getType() const override { return urpg::scene::SceneType::MAP; }
    std::string getName() const override { return "MeasuredHitchScene"; }
    void onUpdate(float) override { std::this_thread::sleep_for(std::chrono::milliseconds(40)); }
};

void clearPerformanceTestScenes() {
    auto& scenes = urpg::scene::SceneManager::getInstance();
    while (scenes.stackSize() > 0) scenes.popScene();
}

} // namespace

TEST_CASE("Performance capture overlays every required subsystem and memory", "[perf][capture][pcq505]") {
    using namespace urpg::perf;
    PerformanceCapture capture({4, 30000});
    REQUIRE(capture.start("hitch-report").success);
    REQUIRE_FALSE(capture.start("second").success);
    REQUIRE(capture.record({1, 1000000, 16000,
                            {{PerformanceSubsystem::Render, 7000}, {PerformanceSubsystem::ScriptEvent, 2000},
                             {PerformanceSubsystem::AssetStreaming, 1000}, {PerformanceSubsystem::Audio, 500}},
                            512u * 1024u * 1024u}).success);
    const auto overlay = capture.overlay();
    REQUIRE(overlay.capturing);
    REQUIRE(overlay.captured_frames == 1);
    REQUIRE(overlay.latest_frame_us == 16000);
    REQUIRE(overlay.latest_memory_bytes == 512u * 1024u * 1024u);
    REQUIRE(overlay.latest_subsystem_us.at(PerformanceSubsystem::Render) == 7000);
    REQUIRE(overlay.latest_subsystem_us.at(PerformanceSubsystem::ScriptEvent) == 2000);
    REQUIRE(overlay.latest_subsystem_us.at(PerformanceSubsystem::AssetStreaming) == 1000);
    REQUIRE(overlay.latest_subsystem_us.at(PerformanceSubsystem::Audio) == 500);
    REQUIRE_FALSE(overlay.latest_spike);
}

TEST_CASE("Performance capture links a visible hitch to subsystem and timestamp", "[perf][capture][pcq505]") {
    using namespace urpg::perf;
    PerformanceCapture capture({3, 30000});
    REQUIRE(capture.start("streaming-hitch").success);
    REQUIRE(capture.record({10, 2000000, 17000, {{PerformanceSubsystem::Render, 8000}}, 100}).success);
    REQUIRE(capture.record({11, 2017000, 48000,
                            {{PerformanceSubsystem::Render, 12000}, {PerformanceSubsystem::AssetStreaming, 29000},
                             {PerformanceSubsystem::Audio, 2000}}, 140}).success);
    REQUIRE(capture.record({12, 2065000, 18000, {{PerformanceSubsystem::Render, 9000}}, 120}).success);
    const auto overlay = capture.overlay();
    REQUIRE(overlay.average_frame_us == 27666);
    REQUIRE(overlay.maximum_frame_us == 48000);
    REQUIRE(overlay.latest_spike);
    REQUIRE(overlay.latest_spike->frame_index == 11);
    REQUIRE(overlay.latest_spike->timestamp_us == 2017000);
    REQUIRE(overlay.latest_spike->dominant_subsystem == PerformanceSubsystem::AssetStreaming);
    REQUIRE(overlay.latest_spike->dominant_time_us == 29000);

    const auto report = capture.report();
    REQUIRE(report["schema"] == "urpg.performance_capture.v1");
    REQUIRE(report["spikes"][0]["dominant_subsystem"] == "asset_streaming");
    REQUIRE(report["spikes"][0]["timestamp_us"] == 2017000);
}

TEST_CASE("Performance capture is bounded and rejects ambiguous samples", "[perf][capture][pcq505]") {
    using namespace urpg::perf;
    PerformanceCapture capture({2, 100});
    REQUIRE_FALSE(capture.record({1, 1, 1, {}, 1}).success);
    REQUIRE(capture.start("bounded").success);
    REQUIRE(capture.record({1, 10, 90, {}, 1}).success);
    REQUIRE(capture.record({2, 20, 110, {{PerformanceSubsystem::ScriptEvent, 80}}, 2}).success);
    REQUIRE_FALSE(capture.record({2, 21, 120, {}, 3}).success);
    REQUIRE(capture.record({3, 30, 130, {{PerformanceSubsystem::Audio, 70}}, 4}).success);
    REQUIRE(capture.frames().size() == 2);
    REQUIRE(capture.frames().front().frame_index == 2);
    REQUIRE(capture.spikes().size() == 2);
    REQUIRE(capture.stop().success);
    REQUIRE_FALSE(capture.record({4, 40, 100, {}, 5}).success);
    REQUIRE_FALSE(capture.stop().success);
    capture.reset();
    REQUIRE(capture.report()["capture_id"] == "");
}

TEST_CASE("Live playtest performance capture publishes overlay controls and governed hitch report",
          "[perf][capture][pcq505][live]") {
    using namespace urpg;
    const auto root = std::filesystem::temp_directory_path() /
        ("urpg_pcq505_live_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    playtest::PlaytestPerformanceRuntime runtime(root, {8, 30000});
    std::string diagnostic;
    REQUIRE(runtime.start("session-a", &diagnostic));
    REQUIRE(runtime.record({1, 1000, 40000,
                            {{perf::PerformanceSubsystem::Render, 9000},
                             {perf::PerformanceSubsystem::ScriptEvent, 28000},
                             {perf::PerformanceSubsystem::AssetStreaming, 1000},
                             {perf::PerformanceSubsystem::Audio, 500},
                             {perf::PerformanceSubsystem::Other, 1500}},
                            128ULL * 1024ULL * 1024ULL}, &diagnostic));

    editor::PerfDiagnosticsPanel panel;
    panel.bindLiveSession(root);
    REQUIRE(panel.refreshLive(&diagnostic));
    panel.render();
    const auto live = panel.lastRenderSnapshot();
    REQUIRE(live["status"] == "live");
    REQUIRE(live["capturing"] == true);
    REQUIRE(live["captured_frames"] == 1);
    REQUIRE(live["memory_bytes"] == 128ULL * 1024ULL * 1024ULL);
    REQUIRE(live["latest_spike"]["frame_index"] == 1);
    REQUIRE(live["latest_spike"]["dominant_subsystem"] == "script_event");
    REQUIRE(std::filesystem::is_regular_file(root / "performance_capture.json"));

    REQUIRE(panel.requestLiveCapture(false, {}, &diagnostic));
    REQUIRE(runtime.poll(&diagnostic));
    REQUIRE(panel.refreshLive(&diagnostic));
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["capturing"] == false);
    REQUIRE(panel.lastRenderSnapshot()["last_control_code"] == "performance_capture_stopped");

    REQUIRE(panel.requestLiveCapture(true, "reported-hitch", &diagnostic));
    REQUIRE(runtime.poll(&diagnostic));
    REQUIRE(panel.refreshLive(&diagnostic));
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["capturing"] == true);
    REQUIRE(panel.lastRenderSnapshot()["captured_frames"] == 0);
    REQUIRE(panel.lastRenderSnapshot()["last_control_code"] == "performance_capture_started");

    std::error_code cleanup_error;
    std::filesystem::remove_all(root, cleanup_error);
}

TEST_CASE("EngineShell measured hitch is attributed to the delayed runtime subsystem",
          "[perf][capture][pcq505][engine_shell]") {
    using namespace urpg;
    auto& shell = EngineShell::getInstance();
    shell.shutdown();
    clearPerformanceTestScenes();
    REQUIRE(shell.startup(std::make_unique<HeadlessSurface>(), std::make_unique<HeadlessRenderer>()));
    scene::SceneManager::getInstance().pushScene(std::make_shared<MeasuredHitchScene>());
    shell.tick(1.0F / 60.0F);
    const auto timing = shell.getLastFrameTimings();
    REQUIRE(timing.scene_update_us >= 30000);
    REQUIRE(timing.total_us >= timing.scene_update_us);

    perf::PerformanceCapture capture({4, 30000});
    REQUIRE(capture.start("measured-engine-hitch").success);
    REQUIRE(capture.record({1, 1000, std::max<uint32_t>(1, timing.total_us),
                            {{perf::PerformanceSubsystem::Render, timing.render_us},
                             {perf::PerformanceSubsystem::ScriptEvent, timing.scene_update_us},
                             {perf::PerformanceSubsystem::Other, timing.input_us}}, 1}).success);
    REQUIRE(capture.spikes().size() == 1);
    REQUIRE(capture.spikes().front().dominant_subsystem == perf::PerformanceSubsystem::ScriptEvent);
    REQUIRE(capture.spikes().front().timestamp_us == 1000);
    shell.shutdown();
    clearPerformanceTestScenes();
}
