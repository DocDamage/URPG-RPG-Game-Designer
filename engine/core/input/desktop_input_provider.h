#pragma once

#include "engine/core/input/input_core.h"
#include <map>

namespace urpg::input {

/**
 * @brief Concrete implementation of InputCore using standard key maps.
 */
class DesktopInputProvider : public InputCore {
public:
    void updateKey(int keyCode, bool pressed) {
        m_keys[keyCode] = pressed;
    }

    void updateMouse(float x, float y, bool leftPressed) {
        m_mouseX = x;
        m_mouseY = y;
        m_mouseLeft = leftPressed;
    }

    bool isKeyPressed(int keyCode) const override {
        auto it = m_keys.find(keyCode);
        return it != m_keys.end() && it->second;
    }

    float getAxis(const std::string& axisName) const override {
        if (axisName == "Horizontal") {
            float val = 0.0f;
            if (isKeyPressed(37) || isKeyPressed(65)) val -= 1.0f; // Left / A
            if (isKeyPressed(39) || isKeyPressed(68)) val += 1.0f; // Right / D
            return val;
        }
        if (axisName == "Vertical") {
            float val = 0.0f;
            if (isKeyPressed(38) || isKeyPressed(87)) val -= 1.0f; // Up / W
            if (isKeyPressed(40) || isKeyPressed(83)) val += 1.0f; // Down / S
            return val;
        }
        return 0.0f;
    }

    bool getButtonDown(const std::string& buttonName) const override {
        if (buttonName == "Interact") return isKeyPressed(69); // E
        if (buttonName == "Attack") return m_mouseLeft;
        return false;
    }

    float getMouseX() const override { return m_mouseX; }
    float getMouseY() const override { return m_mouseY; }

private:
    std::map<int, bool> m_keys;
    float m_mouseX = 0, m_mouseY = 0;
    bool m_mouseLeft = false;
};

} // namespace urpg::input
