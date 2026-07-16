#include "engine/core/scene/first_run_calibration.h"

#include <algorithm>
#include <utility>

namespace urpg::scene {
namespace {

constexpr uint32_t kCalibrationRevision = 1;

CalibrationStep nextStep(const CalibrationStep step) {
    switch (step) {
    case CalibrationStep::Welcome: return CalibrationStep::Text;
    case CalibrationStep::Text: return CalibrationStep::Audio;
    case CalibrationStep::Audio: return CalibrationStep::DisplaySafeArea;
    case CalibrationStep::DisplaySafeArea: return CalibrationStep::InputDevice;
    case CalibrationStep::InputDevice: return CalibrationStep::Accessibility;
    case CalibrationStep::Accessibility: return CalibrationStep::Review;
    case CalibrationStep::Review:
    case CalibrationStep::Complete: return CalibrationStep::Complete;
    }
    return CalibrationStep::Complete;
}

CalibrationStep previousStep(const CalibrationStep step) {
    switch (step) {
    case CalibrationStep::Text: return CalibrationStep::Welcome;
    case CalibrationStep::Audio: return CalibrationStep::Text;
    case CalibrationStep::DisplaySafeArea: return CalibrationStep::Audio;
    case CalibrationStep::InputDevice: return CalibrationStep::DisplaySafeArea;
    case CalibrationStep::Accessibility: return CalibrationStep::InputDevice;
    case CalibrationStep::Review: return CalibrationStep::Accessibility;
    case CalibrationStep::Welcome:
    case CalibrationStep::Complete: return step;
    }
    return step;
}

const char* inputDeviceName(const CalibrationInputDevice device) {
    switch (device) {
    case CalibrationInputDevice::Auto: return "auto";
    case CalibrationInputDevice::KeyboardMouse: return "keyboard_mouse";
    case CalibrationInputDevice::Controller: return "controller";
    }
    return "auto";
}

} // namespace

const char* calibrationStepName(const CalibrationStep step) {
    switch (step) {
    case CalibrationStep::Welcome: return "welcome";
    case CalibrationStep::Text: return "text";
    case CalibrationStep::Audio: return "audio";
    case CalibrationStep::DisplaySafeArea: return "display_safe_area";
    case CalibrationStep::InputDevice: return "input_device";
    case CalibrationStep::Accessibility: return "accessibility";
    case CalibrationStep::Review: return "review";
    case CalibrationStep::Complete: return "complete";
    }
    return "unknown";
}

FirstRunCalibrationFlow::FirstRunCalibrationFlow(urpg::settings::RuntimeSettings settings,
                                                 std::filesystem::path settings_path,
                                                 Callbacks callbacks)
    : settings_(std::move(settings)), baseline_(settings_), settings_path_(std::move(settings_path)),
      callbacks_(std::move(callbacks)) {}

CalibrationResult FirstRunCalibrationFlow::begin(const bool replay) {
    if (active_) return result(false, "calibration_already_active", "Calibration is already active.");
    if (!replay && settings_.calibration.completed) {
        return result(false, "calibration_not_required", "First-run calibration is already complete.");
    }
    baseline_ = settings_;
    replay_ = replay;
    active_ = true;
    step_ = CalibrationStep::Welcome;
    return result(true, replay ? "calibration_replay_started" : "calibration_started",
                  replay ? "Calibration replay started." : "First-run calibration started.");
}

CalibrationResult FirstRunCalibrationFlow::next() {
    if (!active_) return result(false, "calibration_inactive", "Calibration is not active.");
    if (step_ == CalibrationStep::Review) {
        return result(false, "calibration_finish_required", "Review settings and explicitly finish calibration.");
    }
    step_ = nextStep(step_);
    return result(true, "calibration_step_advanced", std::string("Calibration step: ") + calibrationStepName(step_));
}

CalibrationResult FirstRunCalibrationFlow::back() {
    if (!active_) return result(false, "calibration_inactive", "Calibration is not active.");
    if (step_ == CalibrationStep::Welcome) {
        return result(false, "calibration_first_step", "Calibration is already at the first step.");
    }
    step_ = previousStep(step_);
    return result(true, "calibration_step_returned", std::string("Calibration step: ") + calibrationStepName(step_));
}

CalibrationResult FirstRunCalibrationFlow::skip() {
    if (!active_) return result(false, "calibration_inactive", "Calibration is not active.");
    return persist(true);
}

CalibrationResult FirstRunCalibrationFlow::finish() {
    if (!active_) return result(false, "calibration_inactive", "Calibration is not active.");
    if (step_ != CalibrationStep::Review) {
        return result(false, "calibration_review_required", "Complete every calibration step before finishing.");
    }
    return persist(false);
}

CalibrationResult FirstRunCalibrationFlow::cancelReplay() {
    if (!active_ || !replay_) {
        return result(false, "calibration_replay_not_active", "No calibration replay is active.");
    }
    settings_ = baseline_;
    active_ = false;
    replay_ = false;
    step_ = CalibrationStep::Complete;
    return result(true, "calibration_replay_cancelled", "Calibration replay cancelled without changing settings.");
}

void FirstRunCalibrationFlow::setTextScale(const float scale) {
    if (!active_) return;
    settings_.accessibility.text_scale = std::clamp(scale, 0.75F, 2.0F);
}

void FirstRunCalibrationFlow::setMasterVolume(const float volume) {
    if (!active_) return;
    settings_.audio.master_volume = std::clamp(volume, 0.0F, 1.0F);
    if (callbacks_.preview_audio) callbacks_.preview_audio(settings_.audio.master_volume);
}

void FirstRunCalibrationFlow::setDisplay(const uint32_t width, const uint32_t height, const bool fullscreen,
                                         const float safe_area_scale) {
    if (!active_) return;
    settings_.window.width = std::clamp(width, uint32_t{320}, uint32_t{16384});
    settings_.window.height = std::clamp(height, uint32_t{320}, uint32_t{16384});
    settings_.window.fullscreen = fullscreen;
    settings_.window.safe_area_scale = std::clamp(safe_area_scale, 0.80F, 1.0F);
}

void FirstRunCalibrationFlow::setInputDevice(const CalibrationInputDevice device) {
    if (!active_) return;
    settings_.calibration.preferred_input_device = inputDeviceName(device);
}

void FirstRunCalibrationFlow::setAccessibility(const bool high_contrast, const bool reduce_motion,
                                               const bool shortcuts_enabled) {
    if (!active_) return;
    settings_.accessibility.high_contrast = high_contrast;
    settings_.accessibility.reduce_motion = reduce_motion;
    settings_.accessibility.shortcuts_enabled = shortcuts_enabled;
}

CalibrationSnapshot FirstRunCalibrationFlow::snapshot() const {
    return {step_, active_, replay_, true, active_ && step_ != CalibrationStep::Welcome,
            active_ && step_ == CalibrationStep::Review, settings_};
}

CalibrationResult FirstRunCalibrationFlow::persist(const bool skipped) {
    auto candidate = settings_;
    candidate.calibration.completed = true;
    candidate.calibration.skipped = skipped;
    candidate.calibration.revision = kCalibrationRevision;
    std::string error;
    if (settings_path_.empty() || !urpg::settings::saveRuntimeSettings(settings_path_, candidate, &error)) {
        return result(false, "calibration_save_failed",
                      error.empty() ? "Calibration settings path is not configured." : error);
    }
    settings_ = std::move(candidate);
    active_ = false;
    replay_ = false;
    step_ = CalibrationStep::Complete;
    if (callbacks_.settings_applied) callbacks_.settings_applied(settings_);
    return result(true, skipped ? "calibration_skipped" : "calibration_completed",
                  skipped ? "Calibration skipped; defaults were saved and it remains replayable."
                          : "Calibration completed and settings were saved.");
}

CalibrationResult FirstRunCalibrationFlow::result(const bool success, std::string code, std::string message) const {
    return {true, success, std::move(code), std::move(message)};
}

} // namespace urpg::scene
