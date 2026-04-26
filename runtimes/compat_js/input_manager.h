#pragma once

// InputManager - MZ Input/TouchInput Compatibility Surface
// Phase 2 - Compat Layer
//
// This defines the MZ Input and TouchInput API compatibility surface.
// Per Section 4 - WindowCompat Explicit Surface:
// Input / TouchInput behavior matching is required for input plugins.

#include "engine/runtimes/bridge/value.h"
#include "quickjs_runtime.h"
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg {
namespace compat {

// Forward declarations
class InputManagerImpl;

// Key state constants (MZ compatible)
namespace InputKey {
// Direction keys
constexpr int32_t DOWN = 2;
constexpr int32_t LEFT = 4;
constexpr int32_t RIGHT = 6;
constexpr int32_t UP = 8;

// Action keys
constexpr int32_t DECISION = 13; // Enter/Z
constexpr int32_t CANCEL = 27;   // Esc/X
constexpr int32_t SHIFT = 16;
constexpr int32_t CONTROL = 17;
constexpr int32_t ALT = 18;

// Function keys
constexpr int32_t F1 = 112;
constexpr int32_t F2 = 113;
constexpr int32_t F3 = 114;
constexpr int32_t F4 = 115;
constexpr int32_t F5 = 116;
constexpr int32_t F6 = 117;
constexpr int32_t F7 = 118;
constexpr int32_t F8 = 119;
constexpr int32_t F9 = 120;
constexpr int32_t F10 = 121;
constexpr int32_t F11 = 122;
constexpr int32_t F12 = 123;

// Gamepad buttons
constexpr int32_t GAMEPAD_A = 1000;
constexpr int32_t GAMEPAD_B = 1001;
constexpr int32_t GAMEPAD_X = 1002;
constexpr int32_t GAMEPAD_Y = 1003;
constexpr int32_t GAMEPAD_LB = 1004;
constexpr int32_t GAMEPAD_RB = 1005;
constexpr int32_t GAMEPAD_LT = 1006;
constexpr int32_t GAMEPAD_RT = 1007;
constexpr int32_t GAMEPAD_SELECT = 1008;
constexpr int32_t GAMEPAD_START = 1009;
constexpr int32_t GAMEPAD_L3 = 1010;
constexpr int32_t GAMEPAD_R3 = 1011;
constexpr int32_t GAMEPAD_UP = 1012;
constexpr int32_t GAMEPAD_DOWN = 1013;
constexpr int32_t GAMEPAD_LEFT = 1014;
constexpr int32_t GAMEPAD_RIGHT = 1015;
} // namespace InputKey

// Action mapping structure
struct ActionMapping {
    std::string actionId;
    std::vector<int32_t> keyCodes;
    std::vector<int32_t> gamepadButtons;
    int32_t gamepadAxis = -1; // -1 = none, 0 = left X, 1 = left Y, 2 = right X, 3 = right Y
    double axisThreshold = 0.5;
};

// Input state for current frame
struct InputState {
    // Keyboard state
    std::array<bool, 256> keys = {};
    std::array<bool, 256> prevKeys = {};
    std::array<bool, 256> triggeredKeys = {};
    std::array<bool, 256> repeatedKeys = {};
    std::array<int32_t, 256> repeatCounters = {};

    // Mouse state
    int32_t mouseX = 0;
    int32_t mouseY = 0;
    bool mousePressed[3] = {false, false, false}; // Left, Middle, Right
    bool mouseTriggered[3] = {false, false, false};
    int32_t mouseWheel = 0;

    // Touch state
    int32_t touchX = 0;
    int32_t touchY = 0;
    bool touchPressed = false;
    bool touchTriggered = false;
    int32_t touchCount = 0;

    // Gamepad state
    int32_t gamepadId = -1;
    bool gamepadConnected = false;
    std::array<bool, 16> gamepadButtons = {};
    std::array<bool, 16> gamepadButtonsTriggered = {};
    std::array<double, 4> gamepadAxes = {0.0, 0.0, 0.0, 0.0};

    // Direction state
    int32_t dir4 = 0; // 2=down, 4=left, 6=right, 8=up
    int32_t dir8 = 0; // 1=down-left, 2=down, 3=down-right, 4=left, 6=right, 7=up-left, 8=up, 9=up-right
};

// InputManager - MZ Input compatibility
class InputManager {
  public:
    InputManager();
    ~InputManager();

    // Non-copyable
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // Singleton access for compatibility
    static InputManager& instance();

    // ========================================================================
    // Initialization
    // ========================================================================

    // Status: PARTIAL - Initializes deterministic compat input state, not OS/platform input polling
    void initialize();
    void shutdown();

    // Status: PARTIAL - Updates deterministic compat input state each frame
    void update();

    // Status: PARTIAL - Clears deterministic compat input state
    void clear();

    // ========================================================================
    // Keyboard Access
    // ========================================================================

    // Status: PARTIAL - Key state queries reflect deterministic compat state only
    bool isPressed(int32_t keyCode) const;
    bool isTriggered(int32_t keyCode) const;
    bool isRepeated(int32_t keyCode) const;
    bool isReleased(int32_t keyCode) const;

    // Status: PARTIAL - Key state management mutates compat test state, not live platform input
    void setKeyPressed(int32_t keyCode, bool pressed);

    // ========================================================================
    // Direction Input
    // ========================================================================

    // Status: PARTIAL - Direction input resolves from compat key/gamepad state
    int32_t getDir4() const; // Returns 2, 4, 6, 8 or 0
    int32_t getDir8() const; // Returns 1-9 (excluding 5) or 0

    // Status: PARTIAL - Direction trigger queries reflect compat state only
    bool isDirectionPressed(int32_t dir) const;
    bool isDirectionTriggered(int32_t dir) const;

    // ========================================================================
    // Mouse Access
    // ========================================================================

    // Status: PARTIAL - Mouse state queries reflect compat test state only
    int32_t getMouseX() const;
    int32_t getMouseY() const;
    bool isMousePressed(int32_t button = 0) const;
    bool isMouseTriggered(int32_t button = 0) const;
    int32_t getMouseWheel() const;

    // Status: PARTIAL - Mouse state management mutates compat test state, not a live window backend
    void setMousePosition(int32_t x, int32_t y);
    void setMousePressed(int32_t button, bool pressed);
    void setMouseWheel(int32_t wheel);

    // ========================================================================
    // Touch Access
    // ========================================================================

    // Status: PARTIAL - Touch state queries reflect compat test state only
    int32_t getTouchX() const;
    int32_t getTouchY() const;
    bool isTouchPressed() const;
    bool isTouchTriggered() const;
    int32_t getTouchCount() const;

    // Status: PARTIAL - Touch state management mutates compat test state, not a live touch backend
    void setTouchPosition(int32_t x, int32_t y);
    void setTouchPressed(bool pressed);

    // ========================================================================
    // Gamepad Access
    // ========================================================================

    // Status: PARTIAL - Gamepad state queries reflect compat test state only
    bool isGamepadConnected() const;
    int32_t getGamepadId() const;
    bool isGamepadButtonPressed(int32_t button) const;
    bool isGamepadButtonTriggered(int32_t button) const;
    double getGamepadAxis(int32_t axis) const;

    // Status: PARTIAL - Gamepad state management mutates compat test state, not a live gamepad backend
    void setGamepadConnected(bool connected, int32_t id = 0);
    void setGamepadButton(int32_t button, bool pressed);
    void setGamepadAxis(int32_t axis, double value);

    // ========================================================================
    // Action Mapping
    // ========================================================================

    // Status: PARTIAL - Action queries resolve against compat mappings and compat input state
    bool isActionPressed(const std::string& actionId) const;
    bool isActionTriggered(const std::string& actionId) const;
    bool isActionRepeated(const std::string& actionId) const;

    // Status: PARTIAL - Action mapping management affects compat-only mappings
    void mapKeyToAction(int32_t keyCode, const std::string& actionId);
    void mapGamepadButtonToAction(int32_t button, const std::string& actionId);
    void unmapAction(const std::string& actionId);

    // Status: PARTIAL - Returns compat-only action mappings
    const ActionMapping* getActionMapping(const std::string& actionId) const;

    // ========================================================================
    // Compat Status
    // ========================================================================

    // Register InputManager API with QuickJS context
    static void registerAPI(QuickJSContext& ctx);

    // Get compat status for a method
    static CompatStatus getMethodStatus(const std::string& methodName);
    static std::string getMethodDeviation(const std::string& methodName);

  private:
    std::unique_ptr<InputManagerImpl> impl_;
    InputState state_;
    std::unordered_map<std::string, ActionMapping> actionMappings_;

    // Key repeat configuration
    int32_t repeatDelay_ = 20;   // Frames before repeat starts
    int32_t repeatInterval_ = 6; // Frames between repeats

    // API status registry
    static std::unordered_map<std::string, CompatStatus> methodStatus_;
    static std::unordered_map<std::string, std::string> methodDeviations_;

    // Helper methods
    void updateDirectionState();
    void updateKeyRepeats();
};

// TouchInput - MZ TouchInput compatibility
class TouchInput {
  public:
    TouchInput();
    ~TouchInput() = default;

    // Singleton access
    static TouchInput& instance();

    // Status: PARTIAL - Clears deterministic compat touch state
    void clear();

    // Status: PARTIAL - Updates deterministic compat touch state each frame
    void update();

    // ========================================================================
    // Touch Position
    // ========================================================================

    // Status: PARTIAL - Touch position reflects compat state only
    int32_t getX() const;
    int32_t getY() const;

    // Status: PARTIAL - Screen coordinates reflect compat state only
    int32_t getScreenX() const;
    int32_t getScreenY() const;

    // Status: PARTIAL - Applies deterministic camera transform to compat touch state
    int32_t getWorldX() const;
    int32_t getWorldY() const;

    // ========================================================================
    // Touch State
    // ========================================================================

    // Status: PARTIAL - Touch state queries reflect compat touch state only
    bool isPressed() const;
    bool isTriggered() const;
    bool isReleased() const;
    bool isCancelled() const;
    bool isMoved() const;
    bool isStayed() const;

    // ========================================================================
    // Touch Timing
    // ========================================================================

    // Status: PARTIAL - Touch timing is deterministic compat telemetry
    int32_t getTouchCount() const;
    int32_t getTapCount() const;
    int32_t getHoldTime() const;

    // ========================================================================
    // Touch Movement
    // ========================================================================

    // Status: PARTIAL - Touch movement is deterministic compat telemetry
    double getMoveDistance() const;
    double getMoveSpeed() const;
    int32_t getMoveDirection() const;

    // ========================================================================
    // Touch Area
    // ========================================================================

    // Status: PARTIAL - Touch area reflects compat touch state only
    int32_t getStartX() const;
    int32_t getStartY() const;
    int32_t getEndX() const;
    int32_t getEndY() const;

    // ========================================================================
    // Touch State Management (for testing)
    // ========================================================================

    // Status: PARTIAL - Set touch state mutates compat test state, not a live touch backend
    void setTouchPosition(int32_t x, int32_t y);
    void setTouchPressed(bool pressed);
    void setTouchCount(int32_t count);
    void setCameraTransform(double scaleX, double scaleY, int32_t offsetX, int32_t offsetY);
    void resetCameraTransform();

    // ========================================================================
    // Compat Status
    // ========================================================================

    // Register TouchInput API with QuickJS context
    static void registerAPI(QuickJSContext& ctx);

    // Get compat status for a method
    static CompatStatus getMethodStatus(const std::string& methodName);
    static std::string getMethodDeviation(const std::string& methodName);

  private:
    struct TouchState {
        int32_t x = 0;
        int32_t y = 0;
        int32_t prevX = 0;
        int32_t prevY = 0;
        int32_t startX = 0;
        int32_t startY = 0;
        int32_t endX = 0;
        int32_t endY = 0;
        bool pressed = false;
        bool prevPressed = false;
        int32_t count = 0;
        int32_t holdTime = 0;
        int32_t tapCount = 0;
        bool cancelled = false;
        bool moved = false;
        bool stayed = false;
        double moveDistance = 0.0;
        double moveSpeed = 0.0;
        int32_t moveDirection = 0;
        double cameraScaleX = 1.0;
        double cameraScaleY = 1.0;
        int32_t cameraOffsetX = 0;
        int32_t cameraOffsetY = 0;
    };

    TouchState state_;

    // API status registry
    static std::unordered_map<std::string, CompatStatus> methodStatus_;
    static std::unordered_map<std::string, std::string> methodDeviations_;
};

} // namespace compat
} // namespace urpg
