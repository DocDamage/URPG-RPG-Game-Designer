// InputManager - MZ Input/TouchInput Compatibility Surface - Implementation
// Phase 2 - Compat Layer

#include "input_manager.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> InputManager::methodStatus_;
std::unordered_map<std::string, std::string> InputManager::methodDeviations_;
std::unordered_map<std::string, CompatStatus> TouchInput::methodStatus_;
std::unordered_map<std::string, std::string> TouchInput::methodDeviations_;

// ============================================================================
// InputManagerImpl
// ============================================================================

class InputManagerImpl {
public:
    bool initialized = false;
};

// ============================================================================
// InputManager Implementation
// ============================================================================

InputManager::InputManager()
    : impl_(std::make_unique<InputManagerImpl>())
{
    // Initialize method status registry
    if (methodStatus_.empty()) {
        // Keyboard
        methodStatus_["isPressed"] = CompatStatus::FULL;
        methodStatus_["isTriggered"] = CompatStatus::FULL;
        methodStatus_["isRepeated"] = CompatStatus::FULL;
        methodStatus_["isReleased"] = CompatStatus::FULL;
        
        // Direction
        methodStatus_["dir4"] = CompatStatus::FULL;
        methodStatus_["dir8"] = CompatStatus::FULL;
        methodStatus_["isDirectionPressed"] = CompatStatus::FULL;
        methodStatus_["isDirectionTriggered"] = CompatStatus::FULL;
        
        // Mouse
        methodStatus_["mouseX"] = CompatStatus::FULL;
        methodStatus_["mouseY"] = CompatStatus::FULL;
        methodStatus_["isMousePressed"] = CompatStatus::FULL;
        methodStatus_["isMouseTriggered"] = CompatStatus::FULL;
        methodStatus_["mouseWheel"] = CompatStatus::FULL;
        
        // Touch
        methodStatus_["touchX"] = CompatStatus::FULL;
        methodStatus_["touchY"] = CompatStatus::FULL;
        methodStatus_["isTouchPressed"] = CompatStatus::FULL;
        methodStatus_["isTouchTriggered"] = CompatStatus::FULL;
        methodStatus_["touchCount"] = CompatStatus::FULL;
        
        // Gamepad
        methodStatus_["isGamepadConnected"] = CompatStatus::FULL;
        methodStatus_["isGamepadButtonPressed"] = CompatStatus::FULL;
        methodStatus_["isGamepadButtonTriggered"] = CompatStatus::FULL;
        methodStatus_["gamepadAxis"] = CompatStatus::FULL;
        
        // Actions
        methodStatus_["isActionPressed"] = CompatStatus::FULL;
        methodStatus_["isActionTriggered"] = CompatStatus::FULL;
        methodStatus_["isActionRepeated"] = CompatStatus::FULL;
        methodStatus_["mapKeyToAction"] = CompatStatus::FULL;
        methodStatus_["mapGamepadButtonToAction"] = CompatStatus::FULL;
        methodStatus_["unmapAction"] = CompatStatus::FULL;
        
        // Lifecycle
        methodStatus_["initialize"] = CompatStatus::FULL;
        methodStatus_["update"] = CompatStatus::FULL;
        methodStatus_["clear"] = CompatStatus::FULL;
    }
}

InputManager::~InputManager() = default;

InputManager& InputManager::instance() {
    static InputManager instance;
    return instance;
}

// ============================================================================
// Initialization
// ============================================================================

void InputManager::initialize() {
    impl_->initialized = true;
    clear();
}

void InputManager::shutdown() {
    impl_->initialized = false;
    clear();
}

void InputManager::update() {
    // Store previous frame's key states
    state_.prevKeys = state_.keys;
    
    // Clear triggered and released states
    state_.triggeredKeys.fill(false);
    state_.repeatedKeys.fill(false);
    
    // Update key repeats
    updateKeyRepeats();
    
    // Update direction state
    updateDirectionState();
    
    // Clear mouse wheel
    state_.mouseWheel = 0;
    
    // Clear touch triggered
    state_.touchTriggered = false;
}

void InputManager::clear() {
    state_.keys.fill(false);
    state_.prevKeys.fill(false);
    state_.triggeredKeys.fill(false);
    state_.repeatedKeys.fill(false);
    state_.repeatCounters.fill(0);
    
    state_.mouseX = 0;
    state_.mouseY = 0;
    state_.mousePressed[0] = false;
    state_.mousePressed[1] = false;
    state_.mousePressed[2] = false;
    state_.mouseTriggered[0] = false;
    state_.mouseTriggered[1] = false;
    state_.mouseTriggered[2] = false;
    state_.mouseWheel = 0;
    
    state_.touchX = 0;
    state_.touchY = 0;
    state_.touchPressed = false;
    state_.touchTriggered = false;
    state_.touchCount = 0;
    
    state_.gamepadConnected = false;
    state_.gamepadButtons.fill(false);
    state_.gamepadButtonsTriggered.fill(false);
    state_.gamepadAxes.fill(0.0);
    
    state_.dir4 = 0;
    state_.dir8 = 0;
}

// ============================================================================
// Keyboard Access
// ============================================================================

bool InputManager::isPressed(int32_t keyCode) const {
    if (keyCode < 0 || keyCode >= 256) return false;
    return state_.keys[static_cast<size_t>(keyCode)];
}

bool InputManager::isTriggered(int32_t keyCode) const {
    if (keyCode < 0 || keyCode >= 256) return false;
    return state_.triggeredKeys[static_cast<size_t>(keyCode)];
}

bool InputManager::isRepeated(int32_t keyCode) const {
    if (keyCode < 0 || keyCode >= 256) return false;
    return state_.repeatedKeys[static_cast<size_t>(keyCode)];
}

bool InputManager::isReleased(int32_t keyCode) const {
    if (keyCode < 0 || keyCode >= 256) return false;
    return state_.prevKeys[static_cast<size_t>(keyCode)] && !state_.keys[static_cast<size_t>(keyCode)];
}

void InputManager::setKeyPressed(int32_t keyCode, bool pressed) {
    if (keyCode < 0 || keyCode >= 256) return;
    
    bool wasPressed = state_.keys[static_cast<size_t>(keyCode)];
    state_.keys[static_cast<size_t>(keyCode)] = pressed;
    
    if (pressed && !wasPressed) {
        state_.triggeredKeys[static_cast<size_t>(keyCode)] = true;
        state_.repeatCounters[static_cast<size_t>(keyCode)] = 0;
    }
}

// ============================================================================
// Direction Input
// ============================================================================

int32_t InputManager::getDir4() const {
    return state_.dir4;
}

int32_t InputManager::getDir8() const {
    return state_.dir8;
}

bool InputManager::isDirectionPressed(int32_t dir) const {
    switch (dir) {
        case 2: return isPressed(InputKey::DOWN) || isGamepadButtonPressed(InputKey::GAMEPAD_DOWN);
        case 4: return isPressed(InputKey::LEFT) || isGamepadButtonPressed(InputKey::GAMEPAD_LEFT);
        case 6: return isPressed(InputKey::RIGHT) || isGamepadButtonPressed(InputKey::GAMEPAD_RIGHT);
        case 8: return isPressed(InputKey::UP) || isGamepadButtonPressed(InputKey::GAMEPAD_UP);
        default: return false;
    }
}

bool InputManager::isDirectionTriggered(int32_t dir) const {
    switch (dir) {
        case 2: return isTriggered(InputKey::DOWN) || isGamepadButtonTriggered(InputKey::GAMEPAD_DOWN);
        case 4: return isTriggered(InputKey::LEFT) || isGamepadButtonTriggered(InputKey::GAMEPAD_LEFT);
        case 6: return isTriggered(InputKey::RIGHT) || isGamepadButtonTriggered(InputKey::GAMEPAD_RIGHT);
        case 8: return isTriggered(InputKey::UP) || isGamepadButtonTriggered(InputKey::GAMEPAD_UP);
        default: return false;
    }
}

// ============================================================================
// Mouse Access
// ============================================================================

int32_t InputManager::getMouseX() const {
    return state_.mouseX;
}

int32_t InputManager::getMouseY() const {
    return state_.mouseY;
}

bool InputManager::isMousePressed(int32_t button) const {
    if (button < 0 || button > 2) return false;
    return state_.mousePressed[button];
}

bool InputManager::isMouseTriggered(int32_t button) const {
    if (button < 0 || button > 2) return false;
    return state_.mouseTriggered[button];
}

int32_t InputManager::getMouseWheel() const {
    return state_.mouseWheel;
}

void InputManager::setMousePosition(int32_t x, int32_t y) {
    state_.mouseX = x;
    state_.mouseY = y;
}

void InputManager::setMousePressed(int32_t button, bool pressed) {
    if (button < 0 || button > 2) return;
    bool wasPressed = state_.mousePressed[button];
    state_.mousePressed[button] = pressed;
    if (pressed && !wasPressed) {
        state_.mouseTriggered[button] = true;
    }
}

void InputManager::setMouseWheel(int32_t wheel) {
    state_.mouseWheel = wheel;
}

// ============================================================================
// Touch Access
// ============================================================================

int32_t InputManager::getTouchX() const {
    return state_.touchX;
}

int32_t InputManager::getTouchY() const {
    return state_.touchY;
}

bool InputManager::isTouchPressed() const {
    return state_.touchPressed;
}

bool InputManager::isTouchTriggered() const {
    return state_.touchTriggered;
}

int32_t InputManager::getTouchCount() const {
    return state_.touchCount;
}

void InputManager::setTouchPosition(int32_t x, int32_t y) {
    state_.touchX = x;
    state_.touchY = y;
}

void InputManager::setTouchPressed(bool pressed) {
    bool wasPressed = state_.touchPressed;
    state_.touchPressed = pressed;
    if (pressed && !wasPressed) {
        state_.touchTriggered = true;
        state_.touchCount++;
    }
}

// ============================================================================
// Gamepad Access
// ============================================================================

bool InputManager::isGamepadConnected() const {
    return state_.gamepadConnected;
}

int32_t InputManager::getGamepadId() const {
    return state_.gamepadId;
}

bool InputManager::isGamepadButtonPressed(int32_t button) const {
    if (button < 0 || button >= 16) return false;
    return state_.gamepadButtons[static_cast<size_t>(button)];
}

bool InputManager::isGamepadButtonTriggered(int32_t button) const {
    if (button < 0 || button >= 16) return false;
    return state_.gamepadButtonsTriggered[static_cast<size_t>(button)];
}

double InputManager::getGamepadAxis(int32_t axis) const {
    if (axis < 0 || axis >= 4) return 0.0;
    return state_.gamepadAxes[static_cast<size_t>(axis)];
}

void InputManager::setGamepadConnected(bool connected, int32_t id) {
    state_.gamepadConnected = connected;
    state_.gamepadId = connected ? id : -1;
}

void InputManager::setGamepadButton(int32_t button, bool pressed) {
    if (button < 0 || button >= 16) return;
    bool wasPressed = state_.gamepadButtons[static_cast<size_t>(button)];
    state_.gamepadButtons[static_cast<size_t>(button)] = pressed;
    if (pressed && !wasPressed) {
        state_.gamepadButtonsTriggered[static_cast<size_t>(button)] = true;
    }
}

void InputManager::setGamepadAxis(int32_t axis, double value) {
    if (axis < 0 || axis >= 4) return;
    state_.gamepadAxes[static_cast<size_t>(axis)] = std::clamp(value, -1.0, 1.0);
}

// ============================================================================
// Action Mapping
// ============================================================================

bool InputManager::isActionPressed(const std::string& actionId) const {
    auto it = actionMappings_.find(actionId);
    if (it == actionMappings_.end()) return false;
    
    const ActionMapping& mapping = it->second;
    
    // Check keyboard keys
    for (int32_t keyCode : mapping.keyCodes) {
        if (isPressed(keyCode)) return true;
    }
    
    // Check gamepad buttons
    for (int32_t button : mapping.gamepadButtons) {
        if (isGamepadButtonPressed(button)) return true;
    }
    
    // Check gamepad axis
    if (mapping.gamepadAxis >= 0 && mapping.gamepadAxis < 4) {
        double axisValue = getGamepadAxis(mapping.gamepadAxis);
        if (std::abs(axisValue) >= mapping.axisThreshold) return true;
    }
    
    return false;
}

bool InputManager::isActionTriggered(const std::string& actionId) const {
    auto it = actionMappings_.find(actionId);
    if (it == actionMappings_.end()) return false;
    
    const ActionMapping& mapping = it->second;
    
    for (int32_t keyCode : mapping.keyCodes) {
        if (isTriggered(keyCode)) return true;
    }
    
    for (int32_t button : mapping.gamepadButtons) {
        if (isGamepadButtonTriggered(button)) return true;
    }
    
    return false;
}

bool InputManager::isActionRepeated(const std::string& actionId) const {
    auto it = actionMappings_.find(actionId);
    if (it == actionMappings_.end()) return false;
    
    const ActionMapping& mapping = it->second;
    
    for (int32_t keyCode : mapping.keyCodes) {
        if (isRepeated(keyCode)) return true;
    }
    
    return false;
}

void InputManager::mapKeyToAction(int32_t keyCode, const std::string& actionId) {
    auto& mapping = actionMappings_[actionId];
    mapping.actionId = actionId;
    mapping.keyCodes.push_back(keyCode);
}

void InputManager::mapGamepadButtonToAction(int32_t button, const std::string& actionId) {
    auto& mapping = actionMappings_[actionId];
    mapping.actionId = actionId;
    mapping.gamepadButtons.push_back(button);
}

void InputManager::unmapAction(const std::string& actionId) {
    actionMappings_.erase(actionId);
}

const ActionMapping* InputManager::getActionMapping(const std::string& actionId) const {
    auto it = actionMappings_.find(actionId);
    return it != actionMappings_.end() ? &it->second : nullptr;
}

// ============================================================================
// Helper Methods
// ============================================================================

void InputManager::updateDirectionState() {
    bool down = isDirectionPressed(2);
    bool left = isDirectionPressed(4);
    bool right = isDirectionPressed(6);
    bool up = isDirectionPressed(8);
    
    // Calculate dir4 (prioritize horizontal)
    state_.dir4 = 0;
    if (down) state_.dir4 = 2;
    if (left) state_.dir4 = 4;
    if (right) state_.dir4 = 6;
    if (up) state_.dir4 = 8;
    
    // Calculate dir8
    state_.dir8 = 0;
    if (down && !up) {
        if (left && !right) state_.dir8 = 1;
        else if (right && !left) state_.dir8 = 3;
        else state_.dir8 = 2;
    } else if (up && !down) {
        if (left && !right) state_.dir8 = 7;
        else if (right && !left) state_.dir8 = 9;
        else state_.dir8 = 8;
    } else if (left && !right) {
        state_.dir8 = 4;
    } else if (right && !left) {
        state_.dir8 = 6;
    }
}

void InputManager::updateKeyRepeats() {
    for (size_t i = 0; i < 256; ++i) {
        if (state_.keys[i]) {
            state_.repeatCounters[i]++;
            
            // Trigger on initial press
            if (state_.repeatCounters[i] == 1) {
                state_.triggeredKeys[i] = true;
            }
            // Trigger repeat after delay
            else if (state_.repeatCounters[i] == static_cast<int32_t>(repeatDelay_)) {
                state_.repeatedKeys[i] = true;
            }
            // Trigger repeat at interval
            else if (state_.repeatCounters[i] > repeatDelay_ &&
                     (state_.repeatCounters[i] - repeatDelay_) % repeatInterval_ == 0) {
                state_.repeatedKeys[i] = true;
            }
        } else {
            state_.repeatCounters[i] = 0;
        }
    }
}

// ============================================================================
// Compat Status
// ============================================================================

CompatStatus InputManager::getMethodStatus(const std::string& methodName) {
    auto it = methodStatus_.find(methodName);
    if (it != methodStatus_.end()) {
        return it->second;
    }
    return CompatStatus::UNSUPPORTED;
}

std::string InputManager::getMethodDeviation(const std::string& methodName) {
    auto it = methodDeviations_.find(methodName);
    if (it != methodDeviations_.end()) {
        return it->second;
    }
    return "";
}

void InputManager::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"isPressed", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        // return Value::Int(InputManager::instance().isPressed(keyCode) ? 1 : 0);
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"isTriggered", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        // return Value::Int(InputManager::instance().isTriggered(keyCode) ? 1 : 0);
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"isRepeated", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        // return Value::Int(InputManager::instance().isRepeated(keyCode) ? 1 : 0);
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"dir4", [](const std::vector<Value>&) -> Value {
        // return Value::Int(InputManager::instance().getDir4());
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"dir8", [](const std::vector<Value>&) -> Value {
        // return Value::Int(InputManager::instance().getDir8());
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"update", [](const std::vector<Value>&) -> Value {
        // InputManager::instance().update();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"clear", [](const std::vector<Value>&) -> Value {
        // InputManager::instance().clear();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("Input", methods);
}

// ============================================================================
// TouchInput Implementation
// ============================================================================

TouchInput::TouchInput() {
    // Initialize method status registry
    if (methodStatus_.empty()) {
        methodStatus_["x"] = CompatStatus::FULL;
        methodStatus_["y"] = CompatStatus::FULL;
        methodStatus_["screenX"] = CompatStatus::FULL;
        methodStatus_["screenY"] = CompatStatus::FULL;
        methodStatus_["worldX"] = CompatStatus::STUB;
        methodDeviations_["worldX"] = "Requires camera integration";
        methodStatus_["worldY"] = CompatStatus::STUB;
        methodDeviations_["worldY"] = "Requires camera integration";
        methodStatus_["isPressed"] = CompatStatus::FULL;
        methodStatus_["isTriggered"] = CompatStatus::FULL;
        methodStatus_["isReleased"] = CompatStatus::FULL;
        methodStatus_["isCancelled"] = CompatStatus::FULL;
        methodStatus_["isMoved"] = CompatStatus::FULL;
        methodStatus_["isStayed"] = CompatStatus::FULL;
        methodStatus_["touchCount"] = CompatStatus::FULL;
        methodStatus_["tapCount"] = CompatStatus::FULL;
        methodStatus_["holdTime"] = CompatStatus::FULL;
        methodStatus_["moveDistance"] = CompatStatus::FULL;
        methodStatus_["moveSpeed"] = CompatStatus::FULL;
        methodStatus_["moveDirection"] = CompatStatus::FULL;
        methodStatus_["startX"] = CompatStatus::FULL;
        methodStatus_["startY"] = CompatStatus::FULL;
        methodStatus_["endX"] = CompatStatus::FULL;
        methodStatus_["endY"] = CompatStatus::FULL;
        methodStatus_["clear"] = CompatStatus::FULL;
        methodStatus_["update"] = CompatStatus::FULL;
    }
}

TouchInput& TouchInput::instance() {
    static TouchInput instance;
    return instance;
}

void TouchInput::clear() {
    state_ = TouchState{};
}

void TouchInput::update() {
    // Check for trigger
    if (state_.pressed && !state_.prevPressed) {
        state_.startX = state_.x;
        state_.startY = state_.y;
        state_.holdTime = 0;
    }
    
    // Check for release
    if (!state_.pressed && state_.prevPressed) {
        state_.endX = state_.x;
        state_.endY = state_.y;
    }
    
    // Update hold time
    if (state_.pressed) {
        state_.holdTime++;
    }
    
    // Check for movement
    if (state_.pressed && state_.prevPressed) {
        int32_t dx = state_.x - state_.startX;
        int32_t dy = state_.y - state_.startY;
        state_.moveDistance = std::sqrt(dx * dx + dy * dy);
        state_.moved = state_.moveDistance > 10.0;
        state_.stayed = !state_.moved;
        
        // Calculate move direction (8-direction)
        if (state_.moveDistance > 0) {
            double angle = std::atan2(dy, dx) * 180.0 / 3.14159265;
            if (angle < 0) angle += 360.0;
            
            if (angle >= 337.5 || angle < 22.5) state_.moveDirection = 6;
            else if (angle >= 22.5 && angle < 67.5) state_.moveDirection = 3;
            else if (angle >= 67.5 && angle < 112.5) state_.moveDirection = 2;
            else if (angle >= 112.5 && angle < 157.5) state_.moveDirection = 1;
            else if (angle >= 157.5 && angle < 202.5) state_.moveDirection = 4;
            else if (angle >= 202.5 && angle < 247.5) state_.moveDirection = 7;
            else if (angle >= 247.5 && angle < 292.5) state_.moveDirection = 8;
            else state_.moveDirection = 9;
        }
    }
    
    // Store previous state
    state_.prevPressed = state_.pressed;
}

int32_t TouchInput::getX() const { return state_.x; }
int32_t TouchInput::getY() const { return state_.y; }
int32_t TouchInput::getScreenX() const { return state_.x; }
int32_t TouchInput::getScreenY() const { return state_.y; }
int32_t TouchInput::getWorldX() const { return state_.x; } // TODO: Camera transform
int32_t TouchInput::getWorldY() const { return state_.y; } // TODO: Camera transform

bool TouchInput::isPressed() const { return state_.pressed; }
bool TouchInput::isTriggered() const { return state_.pressed && !state_.prevPressed; }
bool TouchInput::isReleased() const { return !state_.pressed && state_.prevPressed; }
bool TouchInput::isCancelled() const { return state_.cancelled; }
bool TouchInput::isMoved() const { return state_.moved; }
bool TouchInput::isStayed() const { return state_.stayed; }

int32_t TouchInput::getTouchCount() const { return state_.count; }
int32_t TouchInput::getTapCount() const { return state_.tapCount; }
int32_t TouchInput::getHoldTime() const { return state_.holdTime; }

double TouchInput::getMoveDistance() const { return state_.moveDistance; }
double TouchInput::getMoveSpeed() const { return state_.moveSpeed; }
int32_t TouchInput::getMoveDirection() const { return state_.moveDirection; }

int32_t TouchInput::getStartX() const { return state_.startX; }
int32_t TouchInput::getStartY() const { return state_.startY; }
int32_t TouchInput::getEndX() const { return state_.endX; }
int32_t TouchInput::getEndY() const { return state_.endY; }

void TouchInput::setTouchPosition(int32_t x, int32_t y) {
    state_.x = x;
    state_.y = y;
}

void TouchInput::setTouchPressed(bool pressed) {
    state_.pressed = pressed;
    if (pressed) {
        state_.count++;
    }
}

void TouchInput::setTouchCount(int32_t count) {
    state_.count = count;
}

CompatStatus TouchInput::getMethodStatus(const std::string& methodName) {
    auto it = methodStatus_.find(methodName);
    if (it != methodStatus_.end()) {
        return it->second;
    }
    return CompatStatus::UNSUPPORTED;
}

std::string TouchInput::getMethodDeviation(const std::string& methodName) {
    auto it = methodDeviations_.find(methodName);
    if (it != methodDeviations_.end()) {
        return it->second;
    }
    return "";
}

void TouchInput::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"x", [](const std::vector<Value>&) -> Value {
        // return Value::Int(TouchInput::instance().getX());
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"y", [](const std::vector<Value>&) -> Value {
        // return Value::Int(TouchInput::instance().getY());
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"isPressed", [](const std::vector<Value>&) -> Value {
        // return Value::Int(TouchInput::instance().isPressed() ? 1 : 0);
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"isTriggered", [](const std::vector<Value>&) -> Value {
        // return Value::Int(TouchInput::instance().isTriggered() ? 1 : 0);
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"clear", [](const std::vector<Value>&) -> Value {
        // TouchInput::instance().clear();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("TouchInput", methods);
}

} // namespace compat
} // namespace urpg
