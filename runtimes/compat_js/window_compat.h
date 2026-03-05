#pragma once

// WindowCompat - Core Surface Stubs
// Phase 2 - Compat Layer
//
// This defines the MZ Window API compatibility surface.
// Each surface is tagged with CompatStatus to indicate implementation level.
//
// Per Section 4 - WindowCompat Explicit Surface:
// V1 must implement at minimum:
// - Window_Base: drawText, drawIcon, drawActorFace, drawActorName, drawActorLevel, drawActorHp/Mp/Tp gauges
// - Window_Selectable + Window_Command — required for any menu plugin
// - Sprite_Character, Sprite_Actor — required for most battle visual plugins

#include "quickjs_runtime.h"
#include "engine/runtimes/bridge/value.h"
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
class WindowCompatManager;

// Rectangle for window dimensions
struct Rect {
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
    
    static Rect fromValues(int32_t x, int32_t y, int32_t w, int32_t h) {
        return Rect{x, y, w, h};
    }
};

// Color for drawing operations
struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
    
    static Color fromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return Color{r, g, b, a};
    }
    
    static Color fromHex(uint32_t hex) {
        return Color{
            static_cast<uint8_t>((hex >> 24) & 0xFF),
            static_cast<uint8_t>((hex >> 16) & 0xFF),
            static_cast<uint8_t>((hex >> 8) & 0xFF),
            static_cast<uint8_t>(hex & 0xFF)
        };
    }
};

// Bitmap reference (opaque handle to texture/surface)
using BitmapHandle = uint32_t;
constexpr BitmapHandle INVALID_BITMAP = 0;

// Window_Base - Base class for all MZ windows
//
// This is the foundation for menu plugins. The compatibility layer
// maps MZ Window_Base calls to URPG's native UI system.
//
class Window_Base {
public:
    // Constructor parameters as MZ expects
    struct CreateParams {
        Rect rect;
        std::optional<std::string> skinBitmap;
        bool transparent = false;
    };
    
    explicit Window_Base(const CreateParams& params);
    virtual ~Window_Base();
    
    // MZ API: Core drawing operations
    // Status: FULL - Behaves identically to MZ
    virtual void drawText(const std::string& text, int32_t x, int32_t y, 
                          int32_t maxWidth = 0, 
                          const std::string& align = "left");
    
    // Status: FULL - Behaves identically to MZ
    virtual void drawIcon(int32_t iconIndex, int32_t x, int32_t y);
    
    // Status: PARTIAL - Face scaling may differ slightly
    // Deviation: Non-integer scaling rounds differently than MZ
    virtual void drawActorFace(int32_t actorId, int32_t x, int32_t y, 
                               int32_t width = 144, int32_t height = 144);
    
    // Status: FULL
    virtual void drawActorName(int32_t actorId, int32_t x, int32_t y, 
                               int32_t width = 150);
    
    // Status: FULL
    virtual void drawActorLevel(int32_t actorId, int32_t x, int32_t y);
    
    // Status: FULL - Uses MZ-compatible default gauge colors
    virtual void drawActorHp(int32_t actorId, int32_t x, int32_t y, 
                             int32_t width = 128);
    
    // Status: FULL - Uses MZ-compatible default gauge colors
    virtual void drawActorMp(int32_t actorId, int32_t x, int32_t y, 
                             int32_t width = 128);
    
    // Status: FULL - Uses MZ-compatible default gauge colors
    virtual void drawActorTp(int32_t actorId, int32_t x, int32_t y, 
                             int32_t width = 128);
    
    // Status: FULL
    virtual void drawGauge(int32_t x, int32_t y, int32_t width,
                           double rate, const Color& color1, const Color& color2);
    
    // Status: FULL
    virtual void drawCharacter(const std::string& characterName,
                               int32_t index, int32_t x, int32_t y);
    
    // Status: PARTIAL - Placeholder labels until item database integration
    virtual void drawItemName(int32_t itemId, int32_t x, int32_t y,
                              int32_t width = 312);
    
    // === Extended drawing methods (commonly used by MZ plugins) ===
    
    // Status: PARTIAL - Basic escape codes only
    // Deviation: Only supports \C[n], \I[n], \G basic codes
    virtual void drawTextEx(const std::string& text, int32_t x, int32_t y,
                            int32_t width = 0);
    
    // Status: FULL
    virtual int32_t lineHeight() const;
    
    // Status: PARTIAL - Uses deterministic width heuristics instead of renderer glyph metrics
    virtual int32_t textWidth(const std::string& text) const;

    // Status: PARTIAL - Uses deterministic width/line-height heuristics
    virtual Rect textSize(const std::string& text) const;
    
    // Text color management
    // Status: FULL
    virtual void changeTextColor(const Color& color);
    virtual void changeTextColor(int32_t systemColorIndex);
    virtual void resetTextColor();
    virtual Color textColor() const;
    
    // Status: FULL
    virtual Color systemColor(int32_t index) const;
    
    // Font management
    // Status: PARTIAL - Font changes not fully applied
    virtual void resetFontSettings();
    virtual std::string fontFace() const;
    virtual int32_t fontSize() const;
    virtual void setFontFace(const std::string& face);
    virtual void setFontSize(int32_t size);
    
    // Contents bitmap
    // Status: STUB - Returns placeholder
    virtual BitmapHandle contents() const;
    virtual void createContents();
    virtual void destroyContents();
    
    // Window state
    virtual void open();
    virtual void close();
    virtual void show();
    virtual void hide();
    
    bool isOpen() const { return isOpen_; }
    bool isVisible() const { return isVisible_; }
    bool isActive() const { return isActive_; }
    void setActive(bool active) { isActive_ = active; }
    
    // Position and size
    Rect getRect() const { return rect_; }
    void setRect(const Rect& rect) { rect_ = rect; }
    
    // Content area (inside padding)
    Rect getContentRect() const;
    
    // Opacity
    int32_t getOpacity() const { return opacity_; }
    void setOpacity(int32_t opacity) { opacity_ = opacity; }
    
    // Background type: 0=window, 1=dim, 2=transparent
    int32_t getBackground() const { return background_; }
    void setBackground(int32_t bg) { background_ = bg; }
    
    // Padding
    int32_t getPadding() const { return padding_; }
    void setPadding(int32_t padding) { padding_ = padding; }
    
    // Update called each frame
    virtual void update();
    
    // Register this class's API with a QuickJS context
    static void registerAPI(QuickJSContext& ctx);
    
    // Get compat status for a method
    static CompatStatus getMethodStatus(const std::string& methodName);
    static std::string getMethodDeviation(const std::string& methodName);
    static uint32_t getMethodCallCount(const std::string& methodName);
    static std::vector<std::string> getTrackedMethods();

protected:
    Rect rect_;
    bool isOpen_ = false;
    bool isVisible_ = true;
    bool isActive_ = true;
    int32_t opacity_ = 255;
    int32_t background_ = 0;
    int32_t padding_ = 12;
    BitmapHandle contents_ = INVALID_BITMAP;
    
    // Text/font state
    Color textColor_ = Color{255, 255, 255, 255};
    std::string fontFace_ = "Microsoft YaHei";
    int32_t fontSize_ = 22;
    
    // API status registry - must be public for static initialization
    static std::unordered_map<std::string, CompatStatus> methodStatus_;
    static std::unordered_map<std::string, std::string> methodDeviations_;
    static std::unordered_map<std::string, uint32_t> methodCallCounts_;
    
    // Initialize static method status maps
    static void initializeMethodStatus();
    static void recordMethodCall(const std::string& methodName);
};

// Window_Selectable - Base for menus with selectable items
//
// Required for any menu plugin. Extends Window_Base with cursor
// navigation and item selection.
//
class Window_Selectable : public Window_Base {
public:
    struct CreateParams : Window_Base::CreateParams {
        int32_t maxCols = 1;
        int32_t numVisibleRows = 4;
        int32_t itemHeight = 36;
    };
    
    explicit Window_Selectable(const CreateParams& params);
    ~Window_Selectable() override = default;
    
    // Selection
    int32_t getIndex() const { return index_; }
    virtual void setIndex(int32_t index);
    void select(int32_t index) { setIndex(index); }
    void deselect() { index_ = -1; }
    
    // Item count
    int32_t getMaxItems() const { return maxItems_; }
    void setMaxItems(int32_t count);
    
    // Column layout
    int32_t getMaxCols() const { return maxCols_; }
    void setMaxCols(int32_t cols);
    
    // Item dimensions
    int32_t getItemHeight() const { return itemHeight_; }
    void setItemHeight(int32_t height);
    int32_t getItemWidth() const;
    
    // Navigation
    virtual void cursorDown(bool wrap = true);
    virtual void cursorUp(bool wrap = true);
    virtual void cursorRight(bool wrap = true);
    virtual void cursorLeft(bool wrap = true);
    
    // Paging
    void cursorPagedown();
    void cursorPageup();
    
    // Current row/col
    int32_t getRow() const;
    int32_t getCol() const;
    
    // Top row (for scrolling)
    int32_t getTopRow() const { return topRow_; }
    void setTopRow(int32_t row);
    int32_t getMaxTopRow() const;
    
    // Update
    void update() override;
    
    // Process handling
    virtual void processHandling();
    virtual void processCursorMove();
    virtual void processPagedown();
    virtual void processPageup();
    
    // Callbacks
    using SelectHandler = std::function<void(int32_t index)>;
    void setOnSelect(SelectHandler handler) { onSelect_ = std::move(handler); }
    
    // Register API
    static void registerAPI(QuickJSContext& ctx);
    
protected:
    int32_t index_ = 0;
    int32_t maxItems_ = 0;
    int32_t maxCols_ = 1;
    int32_t itemHeight_ = 36;
    int32_t topRow_ = 0;
    int32_t numVisibleRows_ = 4;
    SelectHandler onSelect_;
};

// Window_Command - Menu with text commands
//
// The most common window type for menu plugins.
// Extends Window_Selectable with a list of command strings.
//
class Window_Command : public Window_Selectable {
public:
    struct CommandItem {
        std::string name;
        std::string symbol;
        bool enabled = true;
        int32_t ext = 0;  // Extension data
    };
    
    struct CreateParams : Window_Selectable::CreateParams {
        std::vector<CommandItem> commands;
    };
    
    explicit Window_Command(const CreateParams& params);
    ~Window_Command() override = default;
    
    // Command list
    int32_t getCommandCount() const { return static_cast<int32_t>(commands_.size()); }
    const CommandItem& getCommand(int32_t index) const;
    void setCommands(const std::vector<CommandItem>& commands);
    void addCommand(const std::string& name, const std::string& symbol, 
                    bool enabled = true, int32_t ext = 0);
    void clearCommands();
    
    // Current command
    const CommandItem& getCurrentCommand() const;
    bool isCommandEnabled(int32_t index) const;
    bool isCurrentItemEnabled() const;
    std::string getCurrentSymbol() const;
    int32_t findSymbol(const std::string& symbol) const;
    int32_t findExt(int32_t ext) const;
    void callOkHandler();
    
    // Draw
    void drawItem(int32_t index);
    void drawAllItems();
    
    // Selection by symbol
    void selectSymbol(const std::string& symbol);
    void selectExt(int32_t ext);
    
    // Callbacks
    using CommandHandler = std::function<void(const std::string& symbol)>;
    void setOnCommand(CommandHandler handler) { onCommand_ = std::move(handler); }
    
    // Register API
    static void registerAPI(QuickJSContext& ctx);
    
protected:
    std::vector<CommandItem> commands_;
    CommandHandler onCommand_;
};

// Sprite_Character - Map character sprite
//
// Required for plugins that modify character appearance on maps.
//
class Sprite_Character {
public:
    struct CreateParams {
        int32_t characterId = 0;
        std::string characterName;
        int32_t characterIndex = 0;
        int32_t x = 0;
        int32_t y = 0;
    };
    
    explicit Sprite_Character(const CreateParams& params);
    ~Sprite_Character();
    
    // Position
    int32_t getX() const { return x_; }
    int32_t getY() const { return y_; }
    void setX(int32_t x) { x_ = x; }
    void setY(int32_t y) { y_ = y; }
    
    // Character
    std::string getCharacterName() const { return characterName_; }
    void setCharacterName(const std::string& name);
    int32_t getCharacterIndex() const { return characterIndex_; }
    void setCharacterIndex(int32_t index);
    
    // Direction (2=down, 4=left, 6=right, 8=up)
    int32_t getDirection() const { return direction_; }
    void setDirection(int32_t dir) { direction_ = dir; }
    
    // Pattern (animation frame)
    int32_t getPattern() const { return pattern_; }
    void setPattern(int32_t pattern) { pattern_ = pattern; }
    
    // Visibility
    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }
    
    // Blend mode: 0=normal, 1=additive, 2=multiply, 3=screen
    int32_t getBlendMode() const { return blendMode_; }
    void setBlendMode(int32_t mode) { blendMode_ = mode; }
    
    // Opacity
    int32_t getOpacity() const { return opacity_; }
    void setOpacity(int32_t opacity) { opacity_ = opacity; }
    
    // Scale
    double getScaleX() const { return scaleX_; }
    double getScaleY() const { return scaleY_; }
    void setScale(double x, double y) { scaleX_ = x; scaleY_ = y; }
    
    // Update
    void update();
    
    // Register API
    static void registerAPI(QuickJSContext& ctx);
    
private:
    int32_t x_ = 0;
    int32_t y_ = 0;
    std::string characterName_;
    int32_t characterIndex_ = 0;
    int32_t direction_ = 2;
    int32_t pattern_ = 0;
    bool visible_ = true;
    int32_t blendMode_ = 0;
    int32_t opacity_ = 255;
    double scaleX_ = 1.0;
    double scaleY_ = 1.0;
    BitmapHandle bitmap_ = INVALID_BITMAP;
};

// Sprite_Actor - Battle actor sprite
//
// Required for most battle visual plugins.
//
class Sprite_Actor {
public:
    struct CreateParams {
        int32_t actorId = 0;
        int32_t battlerName = 0;
        int32_t x = 0;
        int32_t y = 0;
    };
    
    explicit Sprite_Actor(const CreateParams& params);
    ~Sprite_Actor();
    
    // Actor reference
    int32_t getActorId() const { return actorId_; }
    
    // Position
    int32_t getX() const { return x_; }
    int32_t getY() const { return y_; }
    void setX(int32_t x) { x_ = x; }
    void setY(int32_t y) { y_ = y; }
    
    // Motion (idle, walk, chant, guard, damage, evade, thrust, swing, missile, skill, spell, item, escape, victory, dying, abnormal, sleep, dead)
    int32_t getMotion() const { return motion_; }
    void setMotion(int32_t motion) { motion_ = motion; }
    
    // Visibility
    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }
    
    // Blend mode
    int32_t getBlendMode() const { return blendMode_; }
    void setBlendMode(int32_t mode) { blendMode_ = mode; }
    
    // Opacity
    int32_t getOpacity() const { return opacity_; }
    void setOpacity(int32_t opacity) { opacity_ = opacity; }
    
    // Animation
    void startMotion(int32_t motion);
    void startAnimation(int32_t animationId);
    
    // Effect (whiten, blink, collapse, bossCollapse, instantCollapse)
    void startEffect(const std::string& effect);
    bool isEffecting() const { return effecting_; }
    
    // Update
    void update();
    
    // Register API
    static void registerAPI(QuickJSContext& ctx);
    
private:
    int32_t actorId_ = 0;
    int32_t x_ = 0;
    int32_t y_ = 0;
    int32_t motion_ = 0;
    bool visible_ = true;
    int32_t blendMode_ = 0;
    int32_t opacity_ = 255;
    bool effecting_ = false;
    std::string currentEffect_;
    BitmapHandle bitmap_ = INVALID_BITMAP;
};

// WindowCompatManager - Manages all window instances
//
// Tracks created windows, handles updates, and provides
// access for the compat layer to native rendering.
//
class WindowCompatManager {
public:
    WindowCompatManager();
    ~WindowCompatManager();
    
    // Create windows
    uint32_t createWindowBase(const Window_Base::CreateParams& params);
    uint32_t createWindowSelectable(const Window_Selectable::CreateParams& params);
    uint32_t createWindowCommand(const Window_Command::CreateParams& params);
    uint32_t createSpriteCharacter(const Sprite_Character::CreateParams& params);
    uint32_t createSpriteActor(const Sprite_Actor::CreateParams& params);
    
    // Access
    Window_Base* getWindow(uint32_t id);
    Sprite_Character* getSpriteCharacter(uint32_t id);
    Sprite_Actor* getSpriteActor(uint32_t id);
    
    // Destroy
    void destroyWindow(uint32_t id);
    void destroySprite(uint32_t id);
    void destroyAll();
    
    // Update all
    void updateAll();
    
    // Register all window APIs with a QuickJS context
    void registerAllAPIs(QuickJSContext& ctx);
    
    // Get compat report data
    struct CompatReport {
        std::string className;
        std::string methodName;
        CompatStatus status;
        std::string deviation;
        uint32_t callCount;
    };
    std::vector<CompatReport> getCompatReport() const;
    
private:
    uint32_t nextId_ = 1;
    std::unordered_map<uint32_t, std::unique_ptr<Window_Base>> windows_;
    std::unordered_map<uint32_t, std::unique_ptr<Sprite_Character>> characterSprites_;
    std::unordered_map<uint32_t, std::unique_ptr<Sprite_Actor>> actorSprites_;
};

} // namespace compat
} // namespace urpg
