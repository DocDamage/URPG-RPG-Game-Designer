// WindowCompat - Core Surface Implementation
// Phase 2 - Compat Layer
//
// Implementation stubs for MZ Window API compatibility surface.

#include "window_compat.h"
#include <algorithm>
#include <cassert>

namespace urpg {
namespace compat {

// ============================================================================
// Window_Base Implementation
// ============================================================================

std::unordered_map<std::string, CompatStatus> Window_Base::methodStatus_;
std::unordered_map<std::string, std::string> Window_Base::methodDeviations_;

// Initialize static method status maps
void Window_Base::initializeMethodStatus() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    
    // FULL status methods
    methodStatus_["drawText"] = CompatStatus::FULL;
    methodStatus_["drawIcon"] = CompatStatus::FULL;
    methodStatus_["drawActorName"] = CompatStatus::FULL;
    methodStatus_["drawActorLevel"] = CompatStatus::FULL;
    methodStatus_["drawGauge"] = CompatStatus::FULL;
    methodStatus_["drawCharacter"] = CompatStatus::FULL;
    methodStatus_["lineHeight"] = CompatStatus::FULL;
    methodStatus_["changeTextColor"] = CompatStatus::FULL;
    methodStatus_["resetTextColor"] = CompatStatus::FULL;
    methodStatus_["textColor"] = CompatStatus::FULL;
    methodStatus_["systemColor"] = CompatStatus::FULL;
    methodStatus_["resetFontSettings"] = CompatStatus::FULL;
    methodStatus_["fontFace"] = CompatStatus::FULL;
    methodStatus_["fontSize"] = CompatStatus::FULL;
    methodStatus_["setFontFace"] = CompatStatus::FULL;
    methodStatus_["setFontSize"] = CompatStatus::FULL;
    methodStatus_["contents"] = CompatStatus::FULL;
    methodStatus_["createContents"] = CompatStatus::FULL;
    methodStatus_["destroyContents"] = CompatStatus::FULL;
    
    // PARTIAL status methods
    methodStatus_["drawActorFace"] = CompatStatus::PARTIAL;
    methodDeviations_["drawActorFace"] = "Non-integer scaling rounds differently than MZ";
    
    methodStatus_["drawActorHp"] = CompatStatus::PARTIAL;
    methodDeviations_["drawActorHp"] = "Default gauge colors match URPG theme, not MZ defaults";
    
    methodStatus_["drawActorMp"] = CompatStatus::PARTIAL;
    methodDeviations_["drawActorMp"] = "Default gauge colors match URPG theme, not MZ defaults";
    
    methodStatus_["drawActorTp"] = CompatStatus::PARTIAL;
    methodDeviations_["drawActorTp"] = "Default gauge colors match URPG theme, not MZ defaults";
    
    methodStatus_["drawTextEx"] = CompatStatus::PARTIAL;
    methodDeviations_["drawTextEx"] = "Only supports basic escape codes (\\C[n], \\I[n], \\G)";
    
    // STUB status methods
    methodStatus_["drawItemName"] = CompatStatus::STUB;
    methodDeviations_["drawItemName"] = "No-op in V1";
    
    methodStatus_["textWidth"] = CompatStatus::STUB;
    methodDeviations_["textWidth"] = "Returns estimated width based on character count";
    
    methodStatus_["textSize"] = CompatStatus::STUB;
    methodDeviations_["textSize"] = "Returns estimated size based on character count";
}

Window_Base::Window_Base(const CreateParams& params)
    : rect_(params.rect)
    , background_(params.transparent ? 2 : 0)
{
    initializeMethodStatus();
    
    // TODO: Create actual bitmap for contents
    // contents_ = BitmapManager::create(rect_.width - padding_ * 2, rect_.height - padding_ * 2);
}

Window_Base::~Window_Base() {
    // TODO: Release bitmap resources
}

void Window_Base::drawText(const std::string& text, int32_t x, int32_t y, 
                            int32_t maxWidth, const std::string& align) {
    // TODO: Actual text rendering to contents bitmap
    // For now, this is a stub that validates inputs
    assert(!text.empty() || maxWidth >= 0);
    
    // MZ behavior: empty text with max width clears the area
    if (text.empty() && maxWidth > 0) {
        // Clear area at (x, y) with width maxWidth and height = lineHeight
    }
}

void Window_Base::drawIcon(int32_t iconIndex, int32_t x, int32_t y) {
    // TODO: Draw icon from icon set
    // Icon is 32x32 in MZ default
    assert(iconIndex >= 0);
}

void Window_Base::drawActorFace(int32_t actorId, int32_t x, int32_t y, 
                                 int32_t width, int32_t height) {
    // TODO: Draw actor face from faceset
    assert(actorId >= 0);
    assert(width > 0 && height > 0);
}

void Window_Base::drawActorName(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    // TODO: Get actor name from database and draw
    assert(actorId >= 0);
}

void Window_Base::drawActorLevel(int32_t actorId, int32_t x, int32_t y) {
    // TODO: Get actor level and draw with "Lv" prefix
    assert(actorId >= 0);
}

void Window_Base::drawActorHp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    // TODO: Get actor HP and draw gauge + text
    assert(actorId >= 0);
    // Default colors: color1 = hpGaugeColor1(), color2 = hpGaugeColor2()
}

void Window_Base::drawActorMp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    // TODO: Get actor MP and draw gauge + text
    assert(actorId >= 0);
}

void Window_Base::drawActorTp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    // TODO: Get actor TP and draw gauge + text
    assert(actorId >= 0);
}

void Window_Base::drawGauge(int32_t x, int32_t y, int32_t width,
                             double rate, const Color& color1, const Color& color2) {
    // TODO: Draw gauge background and fill with gradient
    assert(width > 0);
    assert(rate >= 0.0 && rate <= 1.0);
    
    // Gauge height is typically 12 pixels
    // Background: dark gray
    // Fill: gradient from color1 to color2 based on rate
}

void Window_Base::drawCharacter(const std::string& characterName, 
                                 int32_t index, int32_t x, int32_t y) {
    // TODO: Draw character sprite from character sheet
    assert(!characterName.empty());
    assert(index >= 0);
}

void Window_Base::drawItemName(int32_t itemId, int32_t x, int32_t y, int32_t width) {
    // STUB: No-op in V1
    (void)itemId;
    (void)x;
    (void)y;
    (void)width;
}

void Window_Base::open() {
    isOpen_ = true;
    isVisible_ = true;
}

void Window_Base::close() {
    isOpen_ = false;
}

void Window_Base::show() {
    isVisible_ = true;
}

void Window_Base::hide() {
    isVisible_ = false;
}

Rect Window_Base::getContentRect() const {
    return Rect{
        rect_.x + padding_,
        rect_.y + padding_,
        rect_.width - padding_ * 2,
        rect_.height - padding_ * 2
    };
}

void Window_Base::update() {
    // Base update - override in subclasses
}

// ============================================================================
// Extended Window_Base Methods
// ============================================================================

void Window_Base::drawTextEx(const std::string& text, int32_t x, int32_t y, int32_t width) {
    // PARTIAL: Basic escape code support only
    // TODO: Parse \C[n] color codes, \I[n] icons, \G gold
    (void)width;
    
    // For now, strip escape codes and draw plain text
    std::string plainText;
    plainText.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\\' && i + 1 < text.size()) {
            char next = text[i + 1];
            if (next == 'C' || next == 'c' || next == 'I' || next == 'i' ||
                next == 'G' || next == 'g' || next == 'N' || next == 'n' ||
                next == 'P' || next == 'p' || next == 'V' || next == 'v') {
                // Skip escape sequence
                ++i;
                if (i + 1 < text.size() && text[i + 1] == '[') {
                    // Skip to closing bracket
                    ++i;
                    while (i < text.size() && text[i] != ']') ++i;
                }
                continue;
            }
        }
        plainText += text[i];
    }
    drawText(plainText, x, y, width);
}

int32_t Window_Base::lineHeight() const {
    // MZ default line height
    return 36;
}

int32_t Window_Base::textWidth(const std::string& text) const {
    // STUB: Estimate based on character count
    // TODO: Use actual font metrics
    return static_cast<int32_t>(text.size() * fontSize_ * 0.6);
}

Rect Window_Base::textSize(const std::string& text) const {
    // STUB: Estimate based on character count
    return Rect{0, 0, textWidth(text), lineHeight()};
}

void Window_Base::changeTextColor(const Color& color) {
    textColor_ = color;
}

void Window_Base::changeTextColor(int32_t systemColorIndex) {
    textColor_ = systemColor(systemColorIndex);
}

void Window_Base::resetTextColor() {
    textColor_ = Color{255, 255, 255, 255};
}

Color Window_Base::textColor() const {
    return textColor_;
}

Color Window_Base::systemColor(int32_t index) const {
    // MZ system colors (simplified palette)
    static const Color systemColors[] = {
        {0, 0, 0, 255},         // 0: Black
        {255, 255, 255, 255},   // 1: White
        {128, 128, 128, 255},   // 2: Gray
        {255, 128, 128, 255},   // 3: Light red
        {128, 255, 128, 255},   // 4: Light green
        {128, 128, 255, 255},   // 5: Light blue
        {255, 255, 128, 255},   // 6: Yellow
        {255, 128, 255, 255},   // 7: Magenta
        {128, 255, 255, 255},   // 8: Cyan
        {255, 0, 0, 255},       // 9: Red
        {0, 255, 0, 255},       // 10: Green
        {0, 0, 255, 255},       // 11: Blue
        {255, 200, 0, 255},     // 12: Gold
        {192, 192, 192, 255},   // 13: Silver
        {80, 80, 80, 255},      // 14: Dark gray
        {160, 160, 160, 255},   // 15: Medium gray
        {255, 96, 96, 255},     // 16: HP color
        {96, 192, 255, 255},    // 17: MP color
        {255, 128, 64, 255},    // 18: TP color
        {64, 255, 64, 255},     // 19: Power up
        {255, 64, 64, 255},     // 20: Power down
    };
    constexpr int numColors = sizeof(systemColors) / sizeof(systemColors[0]);
    if (index >= 0 && index < numColors) {
        return systemColors[index];
    }
    return Color{255, 255, 255, 255};  // Default white
}

void Window_Base::resetFontSettings() {
    fontFace_ = "Microsoft YaHei";
    fontSize_ = 22;
    resetTextColor();
}

std::string Window_Base::fontFace() const {
    return fontFace_;
}

int32_t Window_Base::fontSize() const {
    return fontSize_;
}

void Window_Base::setFontFace(const std::string& face) {
    fontFace_ = face;
}

void Window_Base::setFontSize(int32_t size) {
    fontSize_ = size;
}

BitmapHandle Window_Base::contents() const {
    return contents_;
}

void Window_Base::createContents() {
    // TODO: Create actual bitmap
    // contents_ = BitmapManager::create(rect_.width - padding_ * 2, rect_.height - padding_ * 2);
    contents_ = 1;  // Placeholder handle
}

void Window_Base::destroyContents() {
    contents_ = INVALID_BITMAP;
}

CompatStatus Window_Base::getMethodStatus(const std::string& methodName) {
    initializeMethodStatus();  // Ensure initialized
    auto it = methodStatus_.find(methodName);
    return it != methodStatus_.end() ? it->second : CompatStatus::UNSUPPORTED;
}

std::string Window_Base::getMethodDeviation(const std::string& methodName) {
    initializeMethodStatus();  // Ensure initialized
    auto it = methodDeviations_.find(methodName);
    return it != methodDeviations_.end() ? it->second : "";
}

void Window_Base::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"drawText", [](const std::vector<Value>&) -> Value {
        // TODO: Bridge to actual instance
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"drawIcon", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"drawActorFace", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::PARTIAL, "Non-integer scaling rounds differently than MZ"});
    
    methods.push_back({"drawActorName", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"drawActorLevel", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"drawActorHp", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::PARTIAL, "Default gauge colors match URPG theme"});
    
    methods.push_back({"drawActorMp", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::PARTIAL, "Default gauge colors match URPG theme"});
    
    methods.push_back({"drawActorTp", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::PARTIAL, "Default gauge colors match URPG theme"});
    
    methods.push_back({"drawGauge", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"drawCharacter", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"drawItemName", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB, "No-op in V1"});
    
    ctx.registerObject("Window_Base", methods);
}

// ============================================================================
// Window_Selectable Implementation
// ============================================================================

Window_Selectable::Window_Selectable(const CreateParams& params)
    : Window_Base(params)
    , maxCols_(params.maxCols)
    , itemHeight_(params.itemHeight)
    , numVisibleRows_(params.numVisibleRows)
{
}

void Window_Selectable::setIndex(int32_t index) {
    index_ = std::clamp(index, -1, std::max(0, maxItems_ - 1));
    
    // Ensure cursor is visible
    int32_t row = getRow();
    int32_t topRow = getTopRow();
    int32_t maxTopRow = getMaxTopRow();
    
    if (row < topRow) {
        setTopRow(row);
    } else if (row >= topRow + numVisibleRows_) {
        setTopRow(row - numVisibleRows_ + 1);
    }
}

int32_t Window_Selectable::getItemWidth() const {
    int32_t spacing = 8;  // MZ default
    return (getContentRect().width - spacing * (maxCols_ - 1)) / maxCols_;
}

void Window_Selectable::cursorDown(bool wrap) {
    if (maxItems_ <= 0) return;
    
    int32_t maxRow = (maxItems_ + maxCols_ - 1) / maxCols_ - 1;
    int32_t currentRow = getRow();
    
    if (currentRow < maxRow) {
        setIndex(index_ + maxCols_);
    } else if (wrap && maxCols_ == 1) {
        setIndex(0);
    }
}

void Window_Selectable::cursorUp(bool wrap) {
    if (maxItems_ <= 0) return;
    
    int32_t currentRow = getRow();
    
    if (currentRow > 0) {
        setIndex(index_ - maxCols_);
    } else if (wrap && maxCols_ == 1) {
        setIndex(maxItems_ - 1);
    }
}

void Window_Selectable::cursorRight(bool wrap) {
    if (maxCols_ < 2) return;
    
    int32_t currentCol = getCol();
    int32_t maxCol = std::min(maxCols_, maxItems_ - getRow() * maxCols_) - 1;
    
    if (currentCol < maxCol) {
        setIndex(index_ + 1);
    } else if (wrap) {
        cursorDown(false);
        if (index_ < maxItems_) {
            setIndex(getRow() * maxCols_);
        }
    }
}

void Window_Selectable::cursorLeft(bool wrap) {
    if (maxCols_ < 2) return;
    
    int32_t currentCol = getCol();
    
    if (currentCol > 0) {
        setIndex(index_ - 1);
    } else if (wrap) {
        int32_t prevRow = getRow() - 1;
        if (prevRow >= 0) {
            int32_t maxCol = std::min(maxCols_, maxItems_ - prevRow * maxCols_) - 1;
            setIndex(prevRow * maxCols_ + maxCol);
        }
    }
}

void Window_Selectable::cursorPagedown() {
    if (maxItems_ <= 0) return;
    
    int32_t newIndex = index_ + numVisibleRows_ * maxCols_;
    if (newIndex < maxItems_) {
        setIndex(newIndex);
    } else {
        setIndex(maxItems_ - 1);
    }
}

void Window_Selectable::cursorPageup() {
    if (maxItems_ <= 0) return;
    
    int32_t newIndex = index_ - numVisibleRows_ * maxCols_;
    if (newIndex >= 0) {
        setIndex(newIndex);
    } else {
        setIndex(0);
    }
}

int32_t Window_Selectable::getRow() const {
    return index_ >= 0 ? index_ / maxCols_ : 0;
}

int32_t Window_Selectable::getCol() const {
    return index_ >= 0 ? index_ % maxCols_ : 0;
}

void Window_Selectable::setTopRow(int32_t row) {
    int32_t maxTop = getMaxTopRow();
    topRow_ = std::clamp(row, 0, maxTop);
}

int32_t Window_Selectable::getMaxTopRow() const {
    if (maxItems_ <= 0) return 0;
    int32_t maxRow = (maxItems_ + maxCols_ - 1) / maxCols_ - 1;
    return std::max(0, maxRow - numVisibleRows_ + 1);
}

void Window_Selectable::update() {
    Window_Base::update();
    processCursorMove();
    processHandling();
}

void Window_Selectable::processHandling() {
    // Process OK/Cancel input
    // TODO: Connect to input system
}

void Window_Selectable::processCursorMove() {
    // TODO: Check input and move cursor
}

void Window_Selectable::processPagedown() {
    cursorPagedown();
}

void Window_Selectable::processPageup() {
    cursorPageup();
}

void Window_Selectable::registerAPI(QuickJSContext& ctx) {
    // First register Window_Base methods
    Window_Base::registerAPI(ctx);
    
    // Add Window_Selectable specific methods
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"setIndex", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"cursorDown", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"cursorUp", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"cursorLeft", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"cursorRight", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("Window_Selectable", methods);
}

// ============================================================================
// Window_Command Implementation
// ============================================================================

Window_Command::Window_Command(const CreateParams& params)
    : Window_Selectable(params)
    , commands_(params.commands)
{
    setMaxItems(static_cast<int32_t>(commands_.size()));
}

const Window_Command::CommandItem& Window_Command::getCommand(int32_t index) const {
    static CommandItem empty{"", "", false, 0};
    if (index < 0 || index >= static_cast<int32_t>(commands_.size())) {
        return empty;
    }
    return commands_[index];
}

void Window_Command::setCommands(const std::vector<CommandItem>& commands) {
    commands_ = commands;
    setMaxItems(static_cast<int32_t>(commands_.size()));
}

void Window_Command::addCommand(const std::string& name, const std::string& symbol,
                                 bool enabled, int32_t ext) {
    commands_.push_back({name, symbol, enabled, ext});
    setMaxItems(static_cast<int32_t>(commands_.size()));
}

void Window_Command::clearCommands() {
    commands_.clear();
    setMaxItems(0);
    setIndex(-1);
}

const Window_Command::CommandItem& Window_Command::getCurrentCommand() const {
    return getCommand(getIndex());
}

std::string Window_Command::getCurrentSymbol() const {
    if (getIndex() < 0 || getIndex() >= static_cast<int32_t>(commands_.size())) {
        return "";
    }
    return commands_[getIndex()].symbol;
}

void Window_Command::drawItem(int32_t index) {
    if (index < 0 || index >= static_cast<int32_t>(commands_.size())) {
        return;
    }
    
    const auto& cmd = commands_[index];
    Rect itemRect{
        0,  // Will be calculated based on row/col
        (index / getMaxCols()) * getItemHeight(),
        getItemWidth(),
        getItemHeight()
    };
    
    // TODO: Draw text with appropriate color based on enabled state
    // Color: normalColor() if enabled, dimColor() if disabled
}

void Window_Command::drawAllItems() {
    for (int32_t i = 0; i < getCommandCount(); ++i) {
        drawItem(i);
    }
}

void Window_Command::selectSymbol(const std::string& symbol) {
    for (int32_t i = 0; i < static_cast<int32_t>(commands_.size()); ++i) {
        if (commands_[i].symbol == symbol) {
            setIndex(i);
            return;
        }
    }
}

void Window_Command::selectExt(int32_t ext) {
    for (int32_t i = 0; i < static_cast<int32_t>(commands_.size()); ++i) {
        if (commands_[i].ext == ext) {
            setIndex(i);
            return;
        }
    }
}

void Window_Command::registerAPI(QuickJSContext& ctx) {
    Window_Selectable::registerAPI(ctx);
    
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"addCommand", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"clearCommands", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"selectSymbol", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("Window_Command", methods);
}

// ============================================================================
// Sprite_Character Implementation
// ============================================================================

Sprite_Character::Sprite_Character(const CreateParams& params)
    : x_(params.x)
    , y_(params.y)
    , characterName_(params.characterName)
    , characterIndex_(params.characterIndex)
{
}

Sprite_Character::~Sprite_Character() {
    // TODO: Release bitmap
}

void Sprite_Character::setCharacterName(const std::string& name) {
    if (characterName_ != name) {
        characterName_ = name;
        // TODO: Reload bitmap
    }
}

void Sprite_Character::setCharacterIndex(int32_t index) {
    if (characterIndex_ != index) {
        characterIndex_ = index;
        // TODO: Update source rect
    }
}

void Sprite_Character::update() {
    // TODO: Update animation, position, etc.
}

void Sprite_Character::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"setX", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"setY", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"setDirection", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"setPattern", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"setVisible", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"setBlendMode", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"setOpacity", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"setScale", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("Sprite_Character", methods);
}

// ============================================================================
// Sprite_Actor Implementation
// ============================================================================

Sprite_Actor::Sprite_Actor(const CreateParams& params)
    : actorId_(params.actorId)
    , x_(params.x)
    , y_(params.y)
{
}

Sprite_Actor::~Sprite_Actor() {
    // TODO: Release bitmap
}

void Sprite_Actor::startMotion(int32_t motion) {
    motion_ = motion;
    // TODO: Start motion animation
}

void Sprite_Actor::startAnimation(int32_t animationId) {
    // TODO: Play battle animation
    (void)animationId;
}

void Sprite_Actor::startEffect(const std::string& effect) {
    currentEffect_ = effect;
    effecting_ = true;
    // TODO: Start effect
}

void Sprite_Actor::update() {
    // TODO: Update animation, effects
}

void Sprite_Actor::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"setMotion", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"startMotion", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"startAnimation", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::PARTIAL, "Some battle animations may render differently"});
    
    methods.push_back({"startEffect", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::PARTIAL, "Collapse effects may differ visually"});
    
    methods.push_back({"setVisible", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"setBlendMode", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"setOpacity", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("Sprite_Actor", methods);
}

// ============================================================================
// WindowCompatManager Implementation
// ============================================================================

WindowCompatManager::WindowCompatManager() = default;

WindowCompatManager::~WindowCompatManager() {
    destroyAll();
}

uint32_t WindowCompatManager::createWindowBase(const Window_Base::CreateParams& params) {
    uint32_t id = nextId_++;
    windows_[id] = std::make_unique<Window_Base>(params);
    return id;
}

uint32_t WindowCompatManager::createWindowSelectable(const Window_Selectable::CreateParams& params) {
    uint32_t id = nextId_++;
    windows_[id] = std::make_unique<Window_Selectable>(params);
    return id;
}

uint32_t WindowCompatManager::createWindowCommand(const Window_Command::CreateParams& params) {
    uint32_t id = nextId_++;
    windows_[id] = std::make_unique<Window_Command>(params);
    return id;
}

uint32_t WindowCompatManager::createSpriteCharacter(const Sprite_Character::CreateParams& params) {
    uint32_t id = nextId_++;
    characterSprites_[id] = std::make_unique<Sprite_Character>(params);
    return id;
}

uint32_t WindowCompatManager::createSpriteActor(const Sprite_Actor::CreateParams& params) {
    uint32_t id = nextId_++;
    actorSprites_[id] = std::make_unique<Sprite_Actor>(params);
    return id;
}

Window_Base* WindowCompatManager::getWindow(uint32_t id) {
    auto it = windows_.find(id);
    return it != windows_.end() ? it->second.get() : nullptr;
}

Sprite_Character* WindowCompatManager::getSpriteCharacter(uint32_t id) {
    auto it = characterSprites_.find(id);
    return it != characterSprites_.end() ? it->second.get() : nullptr;
}

Sprite_Actor* WindowCompatManager::getSpriteActor(uint32_t id) {
    auto it = actorSprites_.find(id);
    return it != actorSprites_.end() ? it->second.get() : nullptr;
}

void WindowCompatManager::destroyWindow(uint32_t id) {
    windows_.erase(id);
}

void WindowCompatManager::destroySprite(uint32_t id) {
    characterSprites_.erase(id);
    actorSprites_.erase(id);
}

void WindowCompatManager::destroyAll() {
    windows_.clear();
    characterSprites_.clear();
    actorSprites_.clear();
}

void WindowCompatManager::updateAll() {
    for (auto& [id, window] : windows_) {
        window->update();
    }
    for (auto& [id, sprite] : characterSprites_) {
        sprite->update();
    }
    for (auto& [id, sprite] : actorSprites_) {
        sprite->update();
    }
}

void WindowCompatManager::registerAllAPIs(QuickJSContext& ctx) {
    Window_Base::registerAPI(ctx);
    Window_Selectable::registerAPI(ctx);
    Window_Command::registerAPI(ctx);
    Sprite_Character::registerAPI(ctx);
    Sprite_Actor::registerAPI(ctx);
}

std::vector<WindowCompatManager::CompatReport> WindowCompatManager::getCompatReport() const {
    std::vector<CompatReport> report;
    
    // Window_Base methods
    std::vector<std::string> baseMethods = {
        "drawText", "drawIcon", "drawActorFace", "drawActorName", "drawActorLevel",
        "drawActorHp", "drawActorMp", "drawActorTp", "drawGauge", "drawCharacter", "drawItemName"
    };
    
    for (const auto& method : baseMethods) {
        CompatReport entry;
        entry.className = "Window_Base";
        entry.methodName = method;
        entry.status = Window_Base::getMethodStatus(method);
        entry.deviation = Window_Base::getMethodDeviation(method);
        entry.callCount = 0;  // TODO: Track actual call counts
        report.push_back(entry);
    }
    
    return report;
}

} // namespace compat
} // namespace urpg
