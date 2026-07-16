#include "engine/core/presentation/runtime_feedback_stack.h"

#include <algorithm>
#include <utility>

namespace urpg::presentation {
namespace {

uint32_t numericPriority(const RuntimeFeedbackPriority priority) {
    switch (priority) {
    case RuntimeFeedbackPriority::Ambient: return 100;
    case RuntimeFeedbackPriority::Secondary: return 200;
    case RuntimeFeedbackPriority::Primary: return 300;
    case RuntimeFeedbackPriority::Critical: return 400;
    }
    return 100;
}

const char* tierName(const RuntimeFeedbackPriority priority) {
    switch (priority) {
    case RuntimeFeedbackPriority::Ambient: return "ambient";
    case RuntimeFeedbackPriority::Secondary: return "secondary";
    case RuntimeFeedbackPriority::Primary: return "primary";
    case RuntimeFeedbackPriority::Critical: return "critical";
    }
    return "ambient";
}

uint32_t maximumDuration(const RuntimePresentationBible& bible, const RuntimeFeedbackPriority priority) {
    const auto found = std::find_if(bible.feedback_hierarchy.begin(), bible.feedback_hierarchy.end(),
                                    [&](const auto& tier) { return tier.id == tierName(priority); });
    return found == bible.feedback_hierarchy.end() ? uint32_t{500} : found->maximum_duration_ms;
}

RuntimeFeedbackTreatment presetTreatment(const RuntimeFeedbackSignal signal, const RuntimePresentationBible& bible) {
    RuntimeFeedbackTreatment treatment;
    switch (signal) {
    case RuntimeFeedbackSignal::Focus:
        treatment.sound = bible.sound_motifs.at("focus");
        treatment.focus_animation = "focus_ring_in";
        treatment.duration_ms = bible.default_tokens.motion_ms.at("feedback");
        break;
    case RuntimeFeedbackSignal::Confirm:
        treatment.sound = bible.sound_motifs.at("confirm");
        treatment.haptic = "urpg.haptic.confirm_tick";
        treatment.focus_animation = "confirm_press";
        treatment.duration_ms = bible.default_tokens.motion_ms.at("feedback");
        break;
    case RuntimeFeedbackSignal::Cancel:
        treatment.sound = bible.sound_motifs.at("cancel");
        treatment.haptic = "urpg.haptic.cancel_tick";
        treatment.focus_animation = "cancel_release";
        treatment.duration_ms = bible.default_tokens.motion_ms.at("feedback");
        break;
    case RuntimeFeedbackSignal::Error:
        treatment.sound = bible.sound_motifs.at("error");
        treatment.haptic = "urpg.haptic.warning_pulse";
        treatment.particle = "ink_arc";
        treatment.focus_animation = "error_notch";
        treatment.duration_ms = bible.default_tokens.motion_ms.at("emphasis");
        break;
    case RuntimeFeedbackSignal::Custom: break;
    }
    return treatment;
}

} // namespace

const char* runtimeFeedbackSignalName(const RuntimeFeedbackSignal signal) {
    switch (signal) {
    case RuntimeFeedbackSignal::Focus: return "focus";
    case RuntimeFeedbackSignal::Confirm: return "confirm";
    case RuntimeFeedbackSignal::Cancel: return "cancel";
    case RuntimeFeedbackSignal::Error: return "error";
    case RuntimeFeedbackSignal::Custom: return "custom";
    }
    return "unknown";
}

RuntimeFeedbackTreatment applyRuntimeFeedbackConstraints(RuntimeFeedbackTreatment treatment,
                                                         const RuntimeFeedbackSettings& settings,
                                                         const RuntimePresentationBible& bible,
                                                         const RuntimeFeedbackPriority priority) {
    const auto intensity = std::clamp(settings.intensity_scale, 0.0F, 1.0F);
    treatment.duration_ms = std::min(treatment.duration_ms, maximumDuration(bible, priority));
    treatment.screen_shake_pixels = std::clamp(treatment.screen_shake_pixels * intensity, 0.0F,
                                               bible.camera.maximum_shake_pixels);
    treatment.hit_stop_ms = std::min(treatment.hit_stop_ms, bible.camera.maximum_hit_stop_ms);
    if (!settings.audio_enabled) treatment.sound.clear();
    if (!settings.haptics_enabled) treatment.haptic.clear();
    if (!settings.particles_enabled) treatment.particle.clear();
    if (!settings.screen_shake_enabled) {
        treatment.screen_shake_pixels = 0.0F;
        treatment.camera.clear();
    }
    if (!settings.hit_stop_enabled) treatment.hit_stop_ms = 0;
    if (settings.reduced_motion) {
        treatment.particle.clear();
        treatment.camera.clear();
        treatment.focus_animation = "instant_focus";
        treatment.screen_shake_pixels = 0.0F;
        treatment.hit_stop_ms = 0;
    }
    return treatment;
}

RuntimeFeedbackStack::RuntimeFeedbackStack(RuntimePresentationBible bible) : bible_(std::move(bible)) {}

void RuntimeFeedbackStack::setSettings(RuntimeFeedbackSettings settings) {
    settings.intensity_scale = std::clamp(settings.intensity_scale, 0.0F, 1.0F);
    settings_ = settings;
}

RuntimeFeedbackResult RuntimeFeedbackStack::submit(RuntimeFeedbackRequest request) {
    if (request.id.empty() || request.visible_text.empty() || request.id.size() > 128 ||
        request.visible_text.size() > 512) {
        return {false, "runtime_feedback_invalid", "Feedback requires a bounded ID and visible explanation.", std::nullopt};
    }
    const auto duplicate = [&](const RuntimeFeedbackCue& cue) { return cue.request_id == request.id; };
    if ((active_ && duplicate(*active_)) || std::any_of(queue_.begin(), queue_.end(), duplicate)) {
        return {false, "runtime_feedback_duplicate", "Feedback request ID is already active or queued.", std::nullopt};
    }
    if (queue_.size() >= 31) {
        return {false, "runtime_feedback_queue_full", "Runtime feedback stack is full.", std::nullopt};
    }

    auto cue = buildCue(request);
    const auto sequence = cue.sequence;
    if (active_ && numericPriority(cue.priority) > numericPriority(active_->priority)) {
        queue_.push_back(std::move(*active_));
        active_ = std::move(cue);
    } else {
        queue_.push_back(std::move(cue));
    }
    std::stable_sort(queue_.begin(), queue_.end(), [](const auto& left, const auto& right) {
        const auto left_priority = numericPriority(left.priority);
        const auto right_priority = numericPriority(right.priority);
        return left_priority == right_priority ? left.sequence < right.sequence : left_priority > right_priority;
    });
    startNext();
    return {true, "runtime_feedback_acknowledged", "Feedback was acknowledged without delaying control.", sequence};
}

void RuntimeFeedbackStack::advance(uint32_t delta_ms) {
    while (delta_ms > 0 && active_) {
        const auto remaining = active_->treatment.duration_ms - active_->elapsed_ms;
        const auto consumed = std::min(delta_ms, remaining);
        active_->elapsed_ms += consumed;
        delta_ms -= consumed;
        if (active_->elapsed_ms >= active_->treatment.duration_ms) {
            active_->completed = true;
            timeline_.push_back(*active_);
            if (timeline_.size() > 256) timeline_.erase(timeline_.begin());
            active_.reset();
            startNext();
        }
    }
}

void RuntimeFeedbackStack::clear() {
    active_.reset();
    queue_.clear();
    timeline_.clear();
    next_sequence_ = 1;
}

RuntimeFeedbackSnapshot RuntimeFeedbackStack::snapshot() const {
    return {settings_, active_, queue_, timeline_};
}

RuntimeFeedbackCue RuntimeFeedbackStack::buildCue(const RuntimeFeedbackRequest& request) {
    RuntimeFeedbackCue cue;
    cue.sequence = next_sequence_++;
    cue.request_id = request.id;
    cue.signal = request.signal;
    cue.priority = request.priority;
    cue.visible_text = request.visible_text;
    cue.treatment = request.signal == RuntimeFeedbackSignal::Custom ? request.treatment
                                                                    : presetTreatment(request.signal, bible_);
    if (cue.treatment.duration_ms == 0) cue.treatment.duration_ms = 1;
    cue.treatment = applyRuntimeFeedbackConstraints(std::move(cue.treatment), settings_, bible_, cue.priority);
    if (cue.treatment.duration_ms == 0) cue.treatment.duration_ms = 1;
    return cue;
}

void RuntimeFeedbackStack::startNext() {
    if (active_ || queue_.empty()) return;
    active_ = std::move(queue_.front());
    queue_.erase(queue_.begin());
}

} // namespace urpg::presentation
