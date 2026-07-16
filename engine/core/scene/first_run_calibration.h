#pragma once

#include "engine/core/settings/app_settings_store.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

namespace urpg::scene {

enum class CalibrationStep : uint8_t {
    Welcome,
    Text,
    Audio,
    DisplaySafeArea,
    InputDevice,
    Accessibility,
    Review,
    Complete
};

enum class CalibrationInputDevice : uint8_t { Auto, KeyboardMouse, Controller };

struct CalibrationResult {
    bool handled = false;
    bool success = false;
    std::string code;
    std::string message;
};

struct CalibrationSnapshot {
    CalibrationStep step = CalibrationStep::Complete;
    bool active = false;
    bool replay = false;
    bool skippable = true;
    bool can_go_back = false;
    bool can_finish = false;
    urpg::settings::RuntimeSettings settings;
};

class FirstRunCalibrationFlow {
public:
    struct Callbacks {
        std::function<void(float master_volume)> preview_audio;
        std::function<void(const urpg::settings::RuntimeSettings&)> settings_applied;
    };

    FirstRunCalibrationFlow(urpg::settings::RuntimeSettings settings,
                            std::filesystem::path settings_path,
                            Callbacks callbacks = {});

    bool shouldRunForFreshProfile() const { return !settings_.calibration.completed; }
    CalibrationResult begin(bool replay = false);
    CalibrationResult next();
    CalibrationResult back();
    CalibrationResult skip();
    CalibrationResult finish();
    CalibrationResult cancelReplay();

    void setTextScale(float scale);
    void setMasterVolume(float volume);
    void setDisplay(uint32_t width, uint32_t height, bool fullscreen, float safe_area_scale);
    void setInputDevice(CalibrationInputDevice device);
    void setAccessibility(bool high_contrast, bool reduce_motion, bool shortcuts_enabled);

    CalibrationSnapshot snapshot() const;
    const urpg::settings::RuntimeSettings& settings() const { return settings_; }

private:
    CalibrationResult persist(bool skipped);
    CalibrationResult result(bool success, std::string code, std::string message) const;

    urpg::settings::RuntimeSettings settings_;
    urpg::settings::RuntimeSettings baseline_;
    std::filesystem::path settings_path_;
    Callbacks callbacks_;
    CalibrationStep step_ = CalibrationStep::Complete;
    bool active_ = false;
    bool replay_ = false;
};

const char* calibrationStepName(CalibrationStep step);

} // namespace urpg::scene
