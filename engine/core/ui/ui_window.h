#pragma once

#include "engine/core/sprite_batcher.h"
#include "engine/core/math/vector2.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace urpg { class Texture; }

namespace urpg::ui {

/**
 * @brief Base class for all native UI elements.
 */
class UIElement {
public:
    virtual ~UIElement() = default;

    virtual void update(float dt) {}
    virtual void draw(SpriteBatcher& batcher) = 0;

    void setPosition(Vector2f pos) { m_position = pos; }
    void setSize(Vector2f size) { m_size = size; }
    void setVisible(bool visible) { m_visible = visible; }
    void setZIndex(float z) { m_zIndex = z; }

    Vector2f getPosition() const { return m_position; }
    Vector2f getSize() const { return m_size; }
    bool isVisible() const { return m_visible; }
    float getZIndex() const { return m_zIndex; }

protected:
    Vector2f m_position{0.0f, 0.0f};
    Vector2f m_size{0.0f, 0.0f};
    float m_zIndex = 0.8f; // UI is usually above the map (0.0 - 0.7)
    bool m_visible = true;
};

/**
 * @brief Standard RPG Maker style window with frame and background.
 */
class UIWindow : public UIElement {
public:
    UIWindow();
    virtual ~UIWindow() = default;

    virtual void draw(SpriteBatcher& batcher) override;

    /**
     * @brief Sets the text to display in the window.
     * Often used for message or log windows.
     */
    void setText(const std::string& text) { m_text = text; }
    const std::string& getText() const { return m_text; }

    /**
     * @brief Sets the windowskin texture.
     */
    void setWindowskin(const std::shared_ptr<urpg::Texture>& texture);

    /**
     * @brief Sets the opacity of the window background (0.0 to 1.0).
     */
    void setOpacity(float opacity) { m_opacity = opacity; }

    /**
     * @brief Helper to draw HP/MP bars within the window.
     */
    void drawGauge(SpriteBatcher& batcher, float x, float y, float width, float rate, uint32_t color1, uint32_t color2);

protected:
    std::shared_ptr<urpg::Texture> m_windowskin;
    std::string m_text;
    float m_opacity = 1.0f;
    float m_backOpacity = 0.8f;
};

} // namespace urpg::ui
