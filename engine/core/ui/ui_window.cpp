#include "ui_window.h"
#include "engine/core/platform/gl_texture.h"

#include <algorithm>

namespace urpg::ui {

UIWindow::UIWindow() {
}

void UIWindow::setWindowskin(const std::shared_ptr<urpg::Texture>& texture) {
    m_windowskin = texture;
}

/**
 * @brief Standard RM Window Layout Mapping (MZ/MV Standard):
 * 0,0-96,96: Background (Tiled)
 * 0,96-96,192: Border and arrows
 * 96,0-128,128: Selection box
 */
void UIWindow::draw(SpriteBatcher& batcher) {
    if (!m_visible || !m_windowskin) return;

    uint32_t texID = m_windowskin->getId();
    int texW = m_windowskin->getWidth();
    int texH = m_windowskin->getHeight();

    // 1. Draw Background (Tiled)
    // Map windowskin 0,0-96,96 to full window size minus small border padding
    float bgU1 = 0.0f;
    float bgV1 = 0.0f;
    float bgU2 = 96.0f / (float)texW;
    float bgV2 = 96.0f / (float)texH;
    
    // Background is Z=m_zIndex
    batcher.submit(texID, 
                  m_position.x + 4.0f, m_position.y + 4.0f, 
                  m_size.x - 8.0f, m_size.y - 8.0f, 
                  bgU1, bgV1, bgU2, bgV2, 
                  m_zIndex, 
                  1.0f, 1.0f, 1.0f, m_backOpacity);

    // 2. Draw Borders (9-Slice approach)
    // Corner size in RM windowskins is 24x24 px (mapped from 64,96 on the sheet)
    // For simplicity, we draw the 4 corners and 4 edges.
    
    // Corner: Top-Left
    batcher.submit(texID, 
                  m_position.x, m_position.y, 
                  24.0f, 24.0f, 
                  96.0f/texW, 0.0f/texH, 120.0f/texW, 24.0f/texH, 
                  m_zIndex + 0.01f);
                  
    // Corner: Top-Right
    batcher.submit(texID, 
                  m_position.x + m_size.x - 24.0f, m_position.y, 
                  24.0f, 24.0f, 
                  168.0f/texW, 0.0f/texH, 192.0f/texW, 24.0f/texH, 
                  m_zIndex + 0.01f);
                  
    // Corner: Bottom-Left
    batcher.submit(texID, 
                  m_position.x, m_position.y + m_size.y - 24.0f, 
                  24.0f, 24.0f, 
                  96.0f/texW, 72.0f/texH, 120.0f/texW, 96.0f/texH, 
                  m_zIndex + 0.01f);
                  
    // Corner: Bottom-Right
    batcher.submit(texID, 
                  m_position.x + m_size.x - 24.0f, m_position.y + m_size.y - 24.0f, 
                  24.0f, 24.0f, 
                  168.0f/texW, 72.0f/texH, 192.0f/texW, 96.0f/texH, 
                  m_zIndex + 0.01f);

    // 3. Draw Text
    if (!m_text.empty()) {
        // Placeholder text rendering: using a standard debug draw or similar
        // For now, we'll just log or draw a colored box representing the text
        // In a real implementation: batcher.drawText(m_text, m_position.x + 10, m_position.y + 10);
    }
}

void UIWindow::drawGauge(SpriteBatcher& batcher, float x, float y, float width, float rate, uint32_t color1, uint32_t color2) {
    const auto toChannel = [](uint32_t color, int shift) {
        return static_cast<float>((color >> shift) & 0xFFu) / 255.0f;
    };

    const float clampedRate = std::clamp(rate, 0.0f, 1.0f);
    constexpr uint32_t kSolidQuadTextureId = 1;
    constexpr uint32_t kGaugeBackground = 0x1B1B24CCu;
    constexpr float gaugeHeight = 12.0f;

    batcher.submit(kSolidQuadTextureId,
                   x, y, width, gaugeHeight,
                   0.0f, 0.0f, 1.0f, 1.0f,
                   m_zIndex + 0.03f,
                   toChannel(kGaugeBackground, 24),
                   toChannel(kGaugeBackground, 16),
                   toChannel(kGaugeBackground, 8),
                   toChannel(kGaugeBackground, 0));

    const float fillWidth = width * clampedRate;
    if (fillWidth <= 0.0f) {
        return;
    }

    batcher.submit(kSolidQuadTextureId,
                   x, y, fillWidth, gaugeHeight,
                   0.0f, 0.0f, 1.0f, 1.0f,
                   m_zIndex + 0.04f,
                   toChannel(color1, 24),
                   toChannel(color1, 16),
                   toChannel(color1, 8),
                   toChannel(color1, 0));

    const float highlightWidth = fillWidth * 0.45f;
    if (highlightWidth > 0.0f) {
        batcher.submit(kSolidQuadTextureId,
                       x, y, highlightWidth, gaugeHeight * 0.45f,
                       0.0f, 0.0f, 1.0f, 1.0f,
                       m_zIndex + 0.05f,
                       toChannel(color2, 24),
                       toChannel(color2, 16),
                       toChannel(color2, 8),
                       toChannel(color2, 0));
    }
}

} // namespace urpg::ui
