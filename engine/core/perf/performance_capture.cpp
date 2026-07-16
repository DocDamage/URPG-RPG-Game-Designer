#include "engine/core/perf/performance_capture.h"

#include <algorithm>
#include <utility>

namespace urpg::perf {

std::string_view performanceSubsystemName(const PerformanceSubsystem subsystem) {
    switch (subsystem) {
    case PerformanceSubsystem::Render: return "render";
    case PerformanceSubsystem::ScriptEvent: return "script_event";
    case PerformanceSubsystem::AssetStreaming: return "asset_streaming";
    case PerformanceSubsystem::Audio: return "audio";
    case PerformanceSubsystem::Other: return "other";
    }
    return "other";
}

PerformanceCapture::PerformanceCapture(PerformanceCaptureConfig config) : config_(config) {
    if (config_.max_frames == 0) config_.max_frames = 1;
    if (config_.spike_threshold_us == 0) config_.spike_threshold_us = 1;
}

PerformanceCaptureResult PerformanceCapture::start(std::string capture_id) {
    if (capturing_) return {false, "performance_capture_already_active", "A performance capture is already active."};
    if (capture_id.empty()) return {false, "performance_capture_id_missing", "Performance capture requires an ID."};
    capture_id_ = std::move(capture_id);
    frames_.clear();
    spikes_.clear();
    capturing_ = true;
    return {true, "performance_capture_started", "Performance capture started."};
}

PerformanceCaptureResult PerformanceCapture::record(PerformanceFrameSample sample) {
    if (!capturing_) return {false, "performance_capture_not_active", "Start capture before recording frames."};
    if ((!frames_.empty() && (sample.frame_index <= frames_.back().frame_index ||
                              sample.timestamp_us < frames_.back().timestamp_us)) || sample.frame_time_us == 0) {
        return {false, "performance_sample_invalid", "Frame samples require increasing frame IDs, monotonic timestamps, and time."};
    }
    if (sample.frame_time_us >= config_.spike_threshold_us) {
        PerformanceSubsystem dominant = PerformanceSubsystem::Other;
        uint32_t dominant_time = 0;
        for (const auto& [subsystem, time] : sample.subsystem_time_us) {
            if (time > dominant_time || (time == dominant_time && subsystem < dominant)) {
                dominant = subsystem;
                dominant_time = time;
            }
        }
        spikes_.push_back({sample.frame_index, sample.timestamp_us, sample.frame_time_us,
                           config_.spike_threshold_us, dominant, dominant_time});
    }
    frames_.push_back(std::move(sample));
    if (frames_.size() > config_.max_frames) frames_.erase(frames_.begin());
    while (!spikes_.empty() && (frames_.empty() || spikes_.front().frame_index < frames_.front().frame_index)) {
        spikes_.erase(spikes_.begin());
    }
    return {true, "performance_sample_recorded", "Performance frame sample recorded."};
}

PerformanceCaptureResult PerformanceCapture::stop() {
    if (!capturing_) return {false, "performance_capture_not_active", "No performance capture is active."};
    capturing_ = false;
    return {true, "performance_capture_stopped", "Performance capture stopped."};
}

void PerformanceCapture::reset() {
    capture_id_.clear();
    capturing_ = false;
    frames_.clear();
    spikes_.clear();
}

PerformanceOverlaySnapshot PerformanceCapture::overlay() const {
    PerformanceOverlaySnapshot snapshot;
    snapshot.capturing = capturing_;
    snapshot.captured_frames = frames_.size();
    if (frames_.empty()) return snapshot;
    const auto& latest = frames_.back();
    snapshot.latest_frame_us = latest.frame_time_us;
    snapshot.latest_memory_bytes = latest.memory_bytes;
    snapshot.latest_subsystem_us = latest.subsystem_time_us;
    uint64_t total = 0;
    for (const auto& frame : frames_) {
        total += frame.frame_time_us;
        snapshot.maximum_frame_us = std::max(snapshot.maximum_frame_us, frame.frame_time_us);
    }
    snapshot.average_frame_us = static_cast<uint32_t>(total / frames_.size());
    if (!spikes_.empty()) snapshot.latest_spike = spikes_.back();
    return snapshot;
}

nlohmann::json PerformanceCapture::report() const {
    nlohmann::json frames = nlohmann::json::array();
    for (const auto& frame : frames_) {
        nlohmann::json subsystems = nlohmann::json::object();
        for (const auto& [subsystem, time] : frame.subsystem_time_us) {
            subsystems[performanceSubsystemName(subsystem)] = time;
        }
        frames.push_back({{"frame_index", frame.frame_index}, {"timestamp_us", frame.timestamp_us},
                          {"frame_time_us", frame.frame_time_us}, {"memory_bytes", frame.memory_bytes},
                          {"subsystems", std::move(subsystems)}});
    }
    nlohmann::json spikes = nlohmann::json::array();
    for (const auto& spike : spikes_) {
        spikes.push_back({{"frame_index", spike.frame_index}, {"timestamp_us", spike.timestamp_us},
                          {"frame_time_us", spike.frame_time_us}, {"threshold_us", spike.threshold_us},
                          {"dominant_subsystem", performanceSubsystemName(spike.dominant_subsystem)},
                          {"dominant_time_us", spike.dominant_time_us}});
    }
    return {{"schema", "urpg.performance_capture.v1"}, {"capture_id", capture_id_},
            {"capturing", capturing_}, {"frame_count", frames_.size()}, {"frames", std::move(frames)},
            {"spikes", std::move(spikes)}};
}

} // namespace urpg::perf
