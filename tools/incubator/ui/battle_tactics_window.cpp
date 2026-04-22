#include "battle_tactics_window.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/sprite_batcher.h"
#include <cmath>

namespace urpg::ui {

BattleTacticsWindow::BattleTacticsWindow() {
    // Battle Tactics position (Upper-right overlay)
    m_position = {500.0f, 20.0f};
    m_size = {280.0f, 150.0f};
    m_zIndex = 0.9f;
    m_backOpacity = 0.85f;
}

void BattleTacticsWindow::setRecommendation(const std::string& move, const std::string& reasoning) {
    m_recommendedMove = move;
    m_reasoning = reasoning;
}

void BattleTacticsWindow::update(float dt) {
    m_pulseTime += dt;
}

void BattleTacticsWindow::draw(SpriteBatcher& batcher) {
    if (!m_visible) return;

    // Draw frame/background
    UIWindow::draw(batcher);

    auto& layer = urpg::RenderLayer::getInstance();

    // 1. Title with pulsing effect if loading or suggesting
    auto titleCmd = std::make_shared<urpg::TextCommand>();
    titleCmd->text = m_isLoading ? "Analyzing..." : "Tactical Advice";
    titleCmd->x = m_position.x + 15.0f;
    titleCmd->y = m_position.y + 15.0f;
    titleCmd->fontSize = 18;
    titleCmd->zOrder = m_zIndex + 0.1f;
    
    // Add pulsing intensity
    float pulse = (std::sin(m_pulseTime * 4.0f) * 0.5f + 0.5f) * 0.3f;
    titleCmd->r = static_cast<uint8_t>(255.0f * (0.7f + pulse));
    titleCmd->g = 255; titleCmd->b = 255;
    layer.submit(titleCmd);

    if (!m_isLoading && !m_recommendedMove.empty()) {
        // 2. Recommended Action (Featured)
        auto moveCmd = std::make_shared<urpg::TextCommand>();
        moveCmd->text = "TRY: " + m_recommendedMove;
        moveCmd->x = m_position.x + 15.0f;
        moveCmd->y = m_position.y + 45.0f;
        moveCmd->fontSize = 22;
        moveCmd->g = 255; moveCmd->r = 100; moveCmd->b = 100; // Highlight in Green-ish
        moveCmd->zOrder = m_zIndex + 0.1f;
        layer.submit(moveCmd);

        // 3. Reasoning (Smaller summary)
        auto reasonCmd = std::make_shared<urpg::TextCommand>();
        // Simple manual line wrapping for reasoning (first 30 chars or so)
        reasonCmd->text = m_reasoning.length() > 60 ? m_reasoning.substr(0, 57) + "..." : m_reasoning;
        reasonCmd->x = m_position.x + 15.0f;
        reasonCmd->y = m_position.y + 80.0f;
        reasonCmd->fontSize = 16;
        reasonCmd->zOrder = m_zIndex + 0.1f;
        layer.submit(reasonCmd);
    }
}

} // namespace urpg::ui
