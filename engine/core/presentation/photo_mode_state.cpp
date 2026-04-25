#include "engine/core/presentation/photo_mode_state.h"

#include <algorithm>
#include <utility>

namespace urpg::presentation {

void PhotoModeState::enter(GameplayPresentationState state) {
    active_ = true;
    gameplay_ = state;
    working_ = std::move(state);
    poses_.clear();
}

void PhotoModeState::exit() {
    active_ = false;
    working_ = gameplay_;
    poses_.clear();
}

bool PhotoModeState::isActive() const {
    return active_;
}

GameplayPresentationState PhotoModeState::gameplayState() const {
    return gameplay_;
}

void PhotoModeState::moveCamera(PhotoCamera camera) {
    working_.camera = camera;
}

void PhotoModeState::hideUi(bool hidden) {
    working_.uiHidden = hidden;
}

void PhotoModeState::overrideWeather(std::string weather) {
    working_.weather = std::move(weather);
}

PhotoPoseResult PhotoModeState::setPose(const std::string& actor_id, const std::string& pose) {
    if (!working_.actors.empty() && !working_.actors.contains(actor_id)) {
        return {false, "actor_not_present"};
    }
    poses_.push_back({actor_id, pose});
    return {};
}

std::string PhotoModeState::poseFor(const std::string& actor_id) const {
    for (auto it = poses_.rbegin(); it != poses_.rend(); ++it) {
        if (it->first == actor_id) {
            return it->second;
        }
    }
    return {};
}

PhotoScreenshotState PhotoModeState::exportScreenshotState(std::string output_name) const {
    return {std::move(output_name), working_.camera, working_.uiHidden, working_.weather};
}

} // namespace urpg::presentation
