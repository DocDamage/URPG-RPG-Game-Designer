#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::perf {

enum class PerformanceSubsystem : uint8_t {
    Render,
    ScriptEvent,
    AssetStreaming,
    Audio,
    Other
};

std::string_view performanceSubsystemName(PerformanceSubsystem subsystem);

struct PerformanceFrameSample {
    uint64_t frame_index = 0;
    uint64_t timestamp_us = 0;
    uint32_t frame_time_us = 0;
    std::map<PerformanceSubsystem, uint32_t> subsystem_time_us;
    uint64_t memory_bytes = 0;
};

struct PerformanceSpike {
    uint64_t frame_index = 0;
    uint64_t timestamp_us = 0;
    uint32_t frame_time_us = 0;
    uint32_t threshold_us = 0;
    PerformanceSubsystem dominant_subsystem = PerformanceSubsystem::Other;
    uint32_t dominant_time_us = 0;
};

struct PerformanceOverlaySnapshot {
    bool capturing = false;
    size_t captured_frames = 0;
    uint32_t latest_frame_us = 0;
    uint32_t average_frame_us = 0;
    uint32_t maximum_frame_us = 0;
    uint64_t latest_memory_bytes = 0;
    std::map<PerformanceSubsystem, uint32_t> latest_subsystem_us;
    std::optional<PerformanceSpike> latest_spike;
};

struct PerformanceCaptureConfig {
    size_t max_frames = 3600;
    uint32_t spike_threshold_us = 33333;
};

struct PerformanceCaptureResult {
    bool success = false;
    std::string code;
    std::string message;
};

class PerformanceCapture {
public:
    explicit PerformanceCapture(PerformanceCaptureConfig config = {});

    PerformanceCaptureResult start(std::string capture_id);
    PerformanceCaptureResult record(PerformanceFrameSample sample);
    PerformanceCaptureResult stop();
    void reset();

    PerformanceOverlaySnapshot overlay() const;
    nlohmann::json report() const;
    const std::vector<PerformanceFrameSample>& frames() const { return frames_; }
    const std::vector<PerformanceSpike>& spikes() const { return spikes_; }

private:
    PerformanceCaptureConfig config_;
    std::string capture_id_;
    bool capturing_ = false;
    std::vector<PerformanceFrameSample> frames_;
    std::vector<PerformanceSpike> spikes_;
};

} // namespace urpg::perf
