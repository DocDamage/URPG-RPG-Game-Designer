#pragma once

#include "engine/core/presentation/runtime_presentation_bible.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::presentation {

enum class RuntimeFeedbackSignal : uint8_t { Focus, Confirm, Cancel, Error, Custom };
enum class RuntimeFeedbackPriority : uint8_t { Ambient, Secondary, Primary, Critical };

struct RuntimeFeedbackSettings {
    bool audio_enabled = true;
    bool haptics_enabled = true;
    bool particles_enabled = true;
    bool screen_shake_enabled = true;
    bool hit_stop_enabled = true;
    bool reduced_motion = false;
    float intensity_scale = 1.0F;
};

struct RuntimeFeedbackTreatment {
    std::string sound;
    std::string haptic;
    std::string particle;
    std::string camera;
    std::string focus_animation;
    float screen_shake_pixels = 0.0F;
    uint32_t hit_stop_ms = 0;
    uint32_t duration_ms = 0;
};

RuntimeFeedbackTreatment applyRuntimeFeedbackConstraints(RuntimeFeedbackTreatment treatment,
                                                         const RuntimeFeedbackSettings& settings,
                                                         const RuntimePresentationBible& bible,
                                                         RuntimeFeedbackPriority priority);

struct RuntimeFeedbackRequest {
    std::string id;
    RuntimeFeedbackSignal signal = RuntimeFeedbackSignal::Custom;
    RuntimeFeedbackPriority priority = RuntimeFeedbackPriority::Secondary;
    std::string visible_text;
    RuntimeFeedbackTreatment treatment;
};

struct RuntimeFeedbackCue {
    uint64_t sequence = 0;
    std::string request_id;
    RuntimeFeedbackSignal signal = RuntimeFeedbackSignal::Custom;
    RuntimeFeedbackPriority priority = RuntimeFeedbackPriority::Secondary;
    std::string visible_text;
    RuntimeFeedbackTreatment treatment;
    uint32_t elapsed_ms = 0;
    bool acknowledged_immediately = true;
    bool control_blocking = false;
    bool completed = false;
};

struct RuntimeFeedbackResult {
    bool success = false;
    std::string code;
    std::string message;
    std::optional<uint64_t> sequence;
};

struct RuntimeFeedbackSnapshot {
    RuntimeFeedbackSettings settings;
    std::optional<RuntimeFeedbackCue> active;
    std::vector<RuntimeFeedbackCue> queued;
    std::vector<RuntimeFeedbackCue> timeline;
};

class RuntimeFeedbackStack {
public:
    explicit RuntimeFeedbackStack(RuntimePresentationBible bible = makeRuntimePresentationBible());

    void setSettings(RuntimeFeedbackSettings settings);
    RuntimeFeedbackResult submit(RuntimeFeedbackRequest request);
    void advance(uint32_t delta_ms);
    void clear();
    RuntimeFeedbackSnapshot snapshot() const;

private:
    RuntimeFeedbackCue buildCue(const RuntimeFeedbackRequest& request);
    void startNext();

    RuntimePresentationBible bible_;
    RuntimeFeedbackSettings settings_;
    std::optional<RuntimeFeedbackCue> active_;
    std::vector<RuntimeFeedbackCue> queue_;
    std::vector<RuntimeFeedbackCue> timeline_;
    uint64_t next_sequence_ = 1;
};

const char* runtimeFeedbackSignalName(RuntimeFeedbackSignal signal);

} // namespace urpg::presentation
