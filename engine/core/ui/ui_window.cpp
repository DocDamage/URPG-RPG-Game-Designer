#include "ui_window.h"
#include "engine/core/platform/gl_texture.h"

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
    (void)batcher;
    (void)x;
    (void)y;
    (void)width;
    (void)rate;
    (void)color1;
    (void)color2;
    // 1. Draw Gauge Background (darker strip)
    // Using a solid color batcher entry if the batcher supports it or a white 1x1 area
    // For now: placeholder draw with a semi-transparent black strip
    // batcher.drawRect(x, y, width, 12, 0.0f, 0.0f, 0.0f, 0.5f);

    // 2. Draw Gauge Filled portion
    // batcher.drawRect(x, y, width * rate, 12, (float)(color1 >> 16)/255.0f, ...);
}

} // namespace urpg::ui
