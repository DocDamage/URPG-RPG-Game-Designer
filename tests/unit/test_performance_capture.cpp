#include "engine/core/perf/performance_capture.h"

#include <catch2/catch_test_macros.hpp>

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
