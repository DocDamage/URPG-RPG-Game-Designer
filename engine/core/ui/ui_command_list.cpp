#include "ui_command_list.h"
#include "engine/core/platform/gl_texture.h"
#include <algorithm>

namespace urpg::ui {

UICommandList::UICommandList() {
}

void UICommandList::addItem(const std::string& text, std::function<void()> onSelect, bool enabled) {
    m_items.push_back({text, enabled, onSelect});
}

void UICommandList::next() {
    if (m_items.empty()) return;
    m_index = (m_index + 1) % (int)m_items.size();
}

void UICommandList::prev() {
    if (m_items.empty()) return;
    m_index = (m_index - 1 + (int)m_items.size()) % (int)m_items.size();
}

void UICommandList::select() {
    if (m_items.empty() || m_index < 0 || m_index >= (int)m_items.size()) return;
    if (m_items[m_index].enabled && m_items[m_index].onSelect) {
        m_items[m_index].onSelect();
    }
}

void UICommandList::update(float dt) {
    (void)dt;
    // Scroll handling (simplified)
    if (m_index < m_topIndex) {
        m_topIndex = m_index;
    } else if (m_index >= m_topIndex + m_maxVisible) {
        m_topIndex = m_index - m_maxVisible + 1;
    }
}

void UICommandList::draw(SpriteBatcher& batcher) {
    // 1. Draw Window Base
    UIWindow::draw(batcher);

    if (!m_visible || !m_windowskin) return;

    uint32_t texID = m_windowskin->getId();
    int texW = m_windowskin->getWidth();
    int texH = m_windowskin->getHeight();

    // 2. Draw Selection Box
    // MZ/MV Sheet maps 96,64-128,96 as the selection box
    float cursorX = m_position.x + 12.0f;
    float cursorY = m_position.y + 12.0f + (m_index - m_topIndex) * m_itemHeight;
    float cursorW = m_size.x - 24.0f;
    float cursorH = m_itemHeight;

    batcher.submit(texID, 
                  cursorX, cursorY, cursorW, cursorH, 
                  192.0f/texW, 64.0f/texH, 224.0f/texW, 96.0f/texH, 
                  m_zIndex + 0.005f, 
                  1.0f, 1.0f, 1.0f, 0.5f);

    // 3. Draw Items
    for (int i = m_topIndex; i < std::min((int)m_items.size(), m_topIndex + m_maxVisible); ++i) {
        // Here we would call a glyph/font renderer.
        // Since font support is a separate module, we'll placeholder it for now.
    }
}

} // namespace urpg::ui
