#pragma once

#include <set>
#include <string>
#include <vector>

namespace urpg::presentation {

struct PhotoCamera {
    float x{0.0f};
    float y{0.0f};
    float zoom{1.0f};
};

struct GameplayPresentationState {
    std::string sceneId;
    PhotoCamera camera;
    bool uiHidden{false};
    std::string weather;
    std::set<std::string> actors;
};

struct PhotoPoseResult {
    bool accepted{true};
    std::string code;
};

struct PhotoScreenshotState {
    std::string outputName;
    PhotoCamera camera;
    bool uiHidden{false};
    std::string weather;
};

class PhotoModeState {
public:
    void enter(GameplayPresentationState state);
    void exit();
    [[nodiscard]] bool isActive() const;
    [[nodiscard]] GameplayPresentationState gameplayState() const;

    void moveCamera(PhotoCamera camera);
    void hideUi(bool hidden);
    void overrideWeather(std::string weather);
    [[nodiscard]] PhotoPoseResult setPose(const std::string& actor_id, const std::string& pose);
    [[nodiscard]] std::string poseFor(const std::string& actor_id) const;
    [[nodiscard]] PhotoScreenshotState exportScreenshotState(std::string output_name) const;

private:
    bool active_{false};
    GameplayPresentationState gameplay_;
    GameplayPresentationState working_;
    std::vector<std::pair<std::string, std::string>> poses_;
};

} // namespace urpg::presentation
