#include "engine/core/playtest/playtest_performance_bridge.h"

#include <fstream>

namespace urpg::playtest {
namespace {

constexpr std::string_view kSnapshotSchema = "urpg.playtest_performance_snapshot.v1";
constexpr std::string_view kControlSchema = "urpg.playtest_performance_control.v1";

bool atomicWrite(const std::filesystem::path& target, const nlohmann::json& value, std::string* diagnostic) {
    std::error_code error;
    std::filesystem::create_directories(target.parent_path(), error);
    auto temporary = target;
    temporary += ".tmp";
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (error || !output) {
        if (diagnostic) *diagnostic = "performance_snapshot_open_failed";
        return false;
    }
    output << value.dump() << '\n';
    output.close();
    if (!output) {
        std::filesystem::remove(temporary, error);
        if (diagnostic) *diagnostic = "performance_snapshot_flush_failed";
        return false;
    }
    std::filesystem::remove(target, error);
    error.clear();
    std::filesystem::rename(temporary, target, error);
    if (error) {
        if (diagnostic) *diagnostic = "performance_snapshot_publish_failed:" + error.message();
        return false;
    }
    return true;
}

nlohmann::json overlayToJson(const perf::PerformanceOverlaySnapshot& overlay) {
    nlohmann::json subsystems = nlohmann::json::object();
    for (const auto& [kind, time] : overlay.latest_subsystem_us) {
        subsystems[perf::performanceSubsystemName(kind)] = time;
    }
    nlohmann::json spike;
    if (overlay.latest_spike) {
        spike = {{"frameIndex", overlay.latest_spike->frame_index},
                 {"timestampUs", overlay.latest_spike->timestamp_us},
                 {"frameTimeUs", overlay.latest_spike->frame_time_us},
                 {"thresholdUs", overlay.latest_spike->threshold_us},
                 {"dominantSubsystem", perf::performanceSubsystemName(overlay.latest_spike->dominant_subsystem)},
                 {"dominantTimeUs", overlay.latest_spike->dominant_time_us}};
    }
    return {{"capturing", overlay.capturing}, {"capturedFrames", overlay.captured_frames},
            {"latestFrameUs", overlay.latest_frame_us}, {"averageFrameUs", overlay.average_frame_us},
            {"maximumFrameUs", overlay.maximum_frame_us}, {"latestMemoryBytes", overlay.latest_memory_bytes},
            {"subsystems", std::move(subsystems)}, {"latestSpike", std::move(spike)}};
}

perf::PerformanceSubsystem parseSubsystem(const std::string& value) {
    if (value == "render") return perf::PerformanceSubsystem::Render;
    if (value == "script_event") return perf::PerformanceSubsystem::ScriptEvent;
    if (value == "asset_streaming") return perf::PerformanceSubsystem::AssetStreaming;
    if (value == "audio") return perf::PerformanceSubsystem::Audio;
    return perf::PerformanceSubsystem::Other;
}

perf::PerformanceOverlaySnapshot overlayFromJson(const nlohmann::json& value) {
    perf::PerformanceOverlaySnapshot overlay;
    overlay.capturing = value.value("capturing", false);
    overlay.captured_frames = value.value("capturedFrames", size_t{0});
    overlay.latest_frame_us = value.value("latestFrameUs", uint32_t{0});
    overlay.average_frame_us = value.value("averageFrameUs", uint32_t{0});
    overlay.maximum_frame_us = value.value("maximumFrameUs", uint32_t{0});
    overlay.latest_memory_bytes = value.value("latestMemoryBytes", uint64_t{0});
    const auto subsystems = value.value("subsystems", nlohmann::json::object());
    for (const auto& [name, time] : subsystems.items()) {
        overlay.latest_subsystem_us[parseSubsystem(name)] = time.get<uint32_t>();
    }
    if (const auto spike = value.find("latestSpike"); spike != value.end() && spike->is_object()) {
        overlay.latest_spike = perf::PerformanceSpike{
            spike->value("frameIndex", uint64_t{0}), spike->value("timestampUs", uint64_t{0}),
            spike->value("frameTimeUs", uint32_t{0}), spike->value("thresholdUs", uint32_t{0}),
            parseSubsystem(spike->value("dominantSubsystem", "other")),
            spike->value("dominantTimeUs", uint32_t{0})};
    }
    return overlay;
}

} // namespace

bool PlaytestPerformanceBridge::publish(const LivePerformanceSnapshot& snapshot, std::string* diagnostic) const {
    if (snapshot.revision == 0 || snapshot.session_id.empty() || !snapshot.report.is_object()) return false;
    if (!atomicWrite(session_directory_ / "performance_capture.json", snapshot.report, diagnostic)) return false;
    return atomicWrite(session_directory_ / "performance_snapshot.json",
                       {{"schema", kSnapshotSchema}, {"revision", snapshot.revision},
                        {"sessionId", snapshot.session_id}, {"overlay", overlayToJson(snapshot.overlay)},
                        {"report", snapshot.report}, {"lastControlId", snapshot.last_control_id},
                        {"lastControlCode", snapshot.last_control_code}}, diagnostic);
}

std::optional<LivePerformanceSnapshot> PlaytestPerformanceBridge::readAfter(
    const uint64_t revision, std::string* diagnostic) const {
    std::ifstream input(session_directory_ / "performance_snapshot.json", std::ios::binary);
    if (!input) return std::nullopt;
    const auto value = nlohmann::json::parse(input, nullptr, false);
    if (!value.is_object() || value.value("schema", "") != kSnapshotSchema) {
        if (diagnostic) *diagnostic = "performance_snapshot_malformed";
        return std::nullopt;
    }
    LivePerformanceSnapshot snapshot;
    snapshot.revision = value.value("revision", uint64_t{0});
    snapshot.session_id = value.value("sessionId", "");
    snapshot.overlay = overlayFromJson(value.value("overlay", nlohmann::json::object()));
    snapshot.report = value.value("report", nlohmann::json::object());
    snapshot.last_control_id = value.value("lastControlId", uint64_t{0});
    snapshot.last_control_code = value.value("lastControlCode", "");
    if (snapshot.revision == 0 || snapshot.session_id.empty() || snapshot.revision <= revision) return std::nullopt;
    return snapshot;
}

bool PlaytestPerformanceBridge::appendControl(const PerformanceControl& control, std::string* diagnostic) const {
    if (control.control_id == 0 || control.expected_revision == 0 ||
        (control.action == PerformanceControlAction::Start && control.capture_id.empty())) return false;
    std::error_code error;
    std::filesystem::create_directories(session_directory_, error);
    std::ofstream output(session_directory_ / "performance_controls.jsonl", std::ios::binary | std::ios::app);
    if (error || !output) {
        if (diagnostic) *diagnostic = "performance_control_open_failed";
        return false;
    }
    output << nlohmann::json{{"schema", kControlSchema}, {"controlId", control.control_id},
                             {"expectedRevision", control.expected_revision},
                             {"action", control.action == PerformanceControlAction::Start ? "start" : "stop"},
                             {"captureId", control.capture_id}}.dump() << '\n';
    return static_cast<bool>(output);
}

PerformanceControlPollResult PlaytestPerformanceBridge::pollControls(
    const ControlHandler& handler, const size_t max_controls) {
    PerformanceControlPollResult result;
    if (!handler || max_controls == 0) return result;
    std::ifstream input(session_directory_ / "performance_controls.jsonl", std::ios::binary);
    if (!input) return result;
    input.seekg(static_cast<std::streamoff>(consumed_control_bytes_));
    std::string line;
    while (result.processed < max_controls && std::getline(input, line)) {
        consumed_control_bytes_ += line.size() + 1;
        const auto value = nlohmann::json::parse(line, nullptr, false);
        const auto action = value.is_object() ? value.value("action", "") : std::string{};
        const bool valid = value.is_object() && value.value("schema", "") == kControlSchema &&
                           value.value("controlId", uint64_t{0}) > 0 &&
                           value.value("expectedRevision", uint64_t{0}) > 0 &&
                           (action == "start" || action == "stop");
        if (valid) {
            handler({value.value("controlId", uint64_t{0}), value.value("expectedRevision", uint64_t{0}),
                     action == "start" ? PerformanceControlAction::Start : PerformanceControlAction::Stop,
                     value.value("captureId", "")});
        }
        ++result.processed;
    }
    if (input.bad()) {
        result.io_error = true;
        result.error = "performance_control_read_failed";
    }
    return result;
}

} // namespace urpg::playtest
