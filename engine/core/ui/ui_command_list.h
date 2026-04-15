#pragma once

#include "ui_window.h"
#include <vector>

namespace urpg::ui {

/**
 * @brief Represents a single list entry in a selection menu.
 */
struct MenuItem {
    std::string text;
    bool enabled = true;
    std::function<void()> onSelect;
};

/**
 * @brief Standard scrollable command window.
 */
class UICommandList : public UIWindow {
public:
    UICommandList();
    ~UICommandList() = default;

    void update(float dt) override;
    void draw(SpriteBatcher& batcher) override;

    void addItem(const std::string& text, std::function<void()> onSelect, bool enabled = true);
    void clearItems() { m_items.clear(); m_index = 0; }

    void next();
    void prev();
    void select();

    int getIndex() const { return m_index; }
    void setIndex(int index) { m_index = index; }

protected:
    std::vector<MenuItem> m_items;
    int m_index = 0;
    int m_topIndex = 0;
    int m_maxVisible = 8;
    float m_itemHeight = 32.0f;
};

} // namespace urpg::ui
