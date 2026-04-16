#pragma once

#include "engine/core/ui/ui_window.h"
#include <string>

namespace urpg::ui {

/**
 * @brief A specialized UI window for displaying battle-time tactical advice from the AI.
 */
class BattleTacticsWindow : public UIWindow {
public:
    BattleTacticsWindow();
    virtual ~BattleTacticsWindow() = default;

    /**
     * @brief Sets the recommendation text and analysis.
     */
    void setRecommendation(const std::string& move, const std::string& reasoning);

    /**
     * @brief Updates the window's visual state (pulsing, animations).
     */
    void update(float dt) override;

    /**
     * @brief Draws the tactics overlay.
     */
    void draw(SpriteBatcher& batcher) override;

    /**
     * @brief Shows or hides the loading state (waiting for AI).
     */
    void setLoading(bool loading) { m_isLoading = loading; }

private:
    std::string m_recommendedMove;
    std::string m_reasoning;
    bool m_isLoading = false;
    float m_pulseTime = 0.0f;
};

} // namespace urpg::ui