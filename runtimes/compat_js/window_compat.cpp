// WindowCompat - Core Surface Implementation
// Phase 2 - Compat Layer
//
// Deterministic implementation of the supported MZ Window API compatibility surface.

#include "window_compat.h"
#include "data_manager.h"
#include "engine/core/message/message_core.h"
#include "engine/core/render/render_layer.h"
#include "input_manager.h"
#include "window_color_parser.h"
#include "window_render_helpers.h"
#include "window_text_layout.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <utility>

namespace urpg {
namespace compat {

namespace {
namespace message = urpg::message;

constexpr int32_t kSelectableItemSpacing = 8;
constexpr int32_t kFontStep = 12;
constexpr int32_t kFontSizeMin = 12;
constexpr int32_t kFontSizeMax = 96;

bool isPrimaryPointerPressed(const InputManager& input) {
    return input.isTouchPressed() || input.isMousePressed(0);
}

bool isPrimaryPointerTriggered(const InputManager& input) {
    return input.isTouchTriggered() || input.isMouseTriggered(0);
}

bool isTouchPrimary(const InputManager& input) {
    return input.isTouchPressed() || input.isTouchTriggered();
}

bool hasPrimaryPointerPosition(const InputManager& input) {
    return isPrimaryPointerPressed(input) || isPrimaryPointerTriggered(input);
}

int32_t getPrimaryPointerX(const InputManager& input, bool preferTouch = false) {
    return (preferTouch || isTouchPrimary(input)) ? input.getTouchX() : input.getMouseX();
}

int32_t getPrimaryPointerY(const InputManager& input, bool preferTouch = false) {
    return (preferTouch || isTouchPrimary(input)) ? input.getTouchY() : input.getMouseY();
}

int32_t hitTestSelectableIndexAt(const Window_Selectable& window, int32_t pointerX, int32_t pointerY) {
    const Rect contentRect = window.getContentRect();
    if (pointerX < contentRect.x || pointerY < contentRect.y || pointerX >= contentRect.x + contentRect.width ||
        pointerY >= contentRect.y + contentRect.height) {
        return -1;
    }

    const int32_t itemWidth = window.getItemWidth();
    const int32_t itemHeight = window.getItemHeight();
    if (itemWidth <= 0 || itemHeight <= 0) {
        return -1;
    }

    const int32_t safeMaxCols = std::max(1, window.getMaxCols());
    const int32_t localX = pointerX - contentRect.x;
    const int32_t localY = pointerY - contentRect.y;
    const int32_t col =
        std::clamp(localX / std::max(1, itemWidth + kSelectableItemSpacing), 0, std::max(0, safeMaxCols - 1));
    const int32_t row = std::max(0, localY / itemHeight) + window.getTopRow();
    const int32_t index = row * safeMaxCols + col;
    if (index < 0 || index >= window.getMaxItems()) {
        return -1;
    }

    const int32_t itemX = col * (itemWidth + kSelectableItemSpacing);
    const int32_t itemY = (row - window.getTopRow()) * itemHeight;
    if (localX < itemX || localX >= itemX + itemWidth || localY < itemY || localY >= itemY + itemHeight) {
        return -1;
    }

    return index;
}

bool isPointerInsideSelectableContent(const Window_Selectable& window, int32_t pointerX, int32_t pointerY) {
    const Rect contentRect = window.getContentRect();
    return pointerX >= contentRect.x && pointerY >= contentRect.y && pointerX < contentRect.x + contentRect.width &&
           pointerY < contentRect.y + contentRect.height;
}

std::string normalizeAlignmentString(const std::string& align) {
    std::string normalized;
    normalized.reserve(align.size());
    for (const char ch : align) {
        normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    if (normalized == "center" || normalized == "right") {
        return normalized;
    }
    return "left";
}

} // namespace

// ============================================================================
// Window_Base Implementation
// ============================================================================

std::unordered_map<std::string, CompatStatus> Window_Base::methodStatus_;
std::unordered_map<std::string, std::string> Window_Base::methodDeviations_;
std::unordered_map<std::string, uint32_t> Window_Base::methodCallCounts_;
std::unordered_map<BitmapHandle, Window_Base::ContentsBitmapInfo> Window_Base::contentsBitmaps_;
Window_Base* Window_Base::defaultInstance_ = nullptr;
BitmapHandle Window_Base::nextBitmapHandle_ = 1;

// Initialize static method status maps
void Window_Base::initializeMethodStatus() {
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    const auto setStatus = [](const std::string& method, CompatStatus status, const std::string& deviation = "") {
        methodStatus_[method] = status;
        if (deviation.empty()) {
            methodDeviations_.erase(method);
        } else {
            methodDeviations_[method] = deviation;
        }
    };

    // FULL status methods
    setStatus("drawText", CompatStatus::FULL);
    setStatus("drawIcon", CompatStatus::FULL);
    setStatus("drawActorName", CompatStatus::FULL);
    setStatus("drawActorLevel", CompatStatus::FULL);
    setStatus("drawGauge", CompatStatus::FULL);
    setStatus("drawCharacter", CompatStatus::FULL);
    setStatus("lineHeight", CompatStatus::FULL);
    setStatus("changeTextColor", CompatStatus::FULL);
    setStatus("resetTextColor", CompatStatus::FULL);
    setStatus("textColor", CompatStatus::FULL);
    setStatus("systemColor", CompatStatus::FULL);
    setStatus("resetFontSettings", CompatStatus::FULL);
    setStatus("fontFace", CompatStatus::FULL);
    setStatus("fontSize", CompatStatus::FULL);
    setStatus("setFontFace", CompatStatus::FULL);
    setStatus("setFontSize", CompatStatus::FULL);
    setStatus("setTextAlignment", CompatStatus::FULL);
    setStatus("textAlignment", CompatStatus::FULL);
    setStatus("contents", CompatStatus::FULL);
    setStatus("createContents", CompatStatus::FULL);
    setStatus("destroyContents", CompatStatus::FULL);
    setStatus("open", CompatStatus::FULL);
    setStatus("close", CompatStatus::FULL);
    setStatus("show", CompatStatus::FULL);
    setStatus("hide", CompatStatus::FULL);
    setStatus("update", CompatStatus::FULL);
    setStatus("getContentRect", CompatStatus::FULL);

    setStatus("drawActorFace", CompatStatus::FULL);

    setStatus("drawActorHp", CompatStatus::FULL);
    setStatus("drawActorMp", CompatStatus::FULL);
    setStatus("drawActorTp", CompatStatus::FULL);

    setStatus("drawTextEx", CompatStatus::FULL);

    setStatus("drawItemName", CompatStatus::FULL);

    setStatus("textWidth", CompatStatus::FULL);

    setStatus("textSize", CompatStatus::FULL);

    for (const auto& [methodName, status] : methodStatus_) {
        (void)status;
        methodCallCounts_[methodName] = 0;
    }
}

Window_Base::Window_Base(const CreateParams& params) : rect_(params.rect), background_(params.transparent ? 2 : 0) {
    initializeMethodStatus();
}

Window_Base::~Window_Base() {
    destroyContents();
}

void Window_Base::setRect(const Rect& rect) {
    rect_ = rect;
    syncContentsBitmap();
}

void Window_Base::setPadding(int32_t padding) {
    padding_ = padding;
    syncContentsBitmap();
}

void Window_Base::drawText(const std::string& text, int32_t x, int32_t y, int32_t maxWidth, const std::string& align) {
    recordMethodCall("drawText");
    const int32_t safeMaxWidth = std::max(0, maxWidth);
    const std::string normalizedAlign = normalizeAlignmentString(align);
    const message::MessageAlignment messageAlign = parseMessageAlignment(normalizedAlign);
    message::RichTextLayoutEngine layout = buildLayoutEngineForWindow(*this, safeMaxWidth, messageAlign);
    const message::RichTextLayoutResult result = layout.layout(text);
    const int32_t offsetX = lineOffsetForFirstLine(result.tokens);

    TextDrawInfo info;
    info.text = text;
    info.align = normalizedAlign;
    info.requestedX = x;
    info.requestedY = y;
    info.resolvedX = x + offsetX;
    info.resolvedY = y;
    info.maxWidth = safeMaxWidth;
    info.measuredWidth = result.metrics.width;
    lastTextDraw_ = std::move(info);
    textDrawHistory_.push_back(*lastTextDraw_);

    urpg::TextCommand textCmd;
    textCmd.text = text;
    textCmd.fontFace = fontFace_;
    textCmd.fontSize = fontSize_;
    textCmd.maxWidth = safeMaxWidth;
    textCmd.x = static_cast<float>(rect_.x + padding_ + lastTextDraw_->resolvedX);
    textCmd.y = static_cast<float>(rect_.y + padding_ + lastTextDraw_->resolvedY);
    textCmd.zOrder = 100;
    textCmd.r = textColor_.r;
    textCmd.g = textColor_.g;
    textCmd.b = textColor_.b;
    textCmd.a = textColor_.a;
    urpg::RenderLayer::getInstance().submit(urpg::toFrameRenderCommand(textCmd));

    assert(!text.empty() || safeMaxWidth >= 0);
}

void Window_Base::drawIcon(int32_t iconIndex, int32_t x, int32_t y) {
    recordMethodCall("drawIcon");
    assert(iconIndex >= 0);

    // MZ icon set layout: 16 icons per row, each 32×32, on a 512×512 sheet.
    constexpr int32_t kIconsPerRow = 16;
    constexpr int32_t kIconSize = 32;

    const int32_t iconX = (iconIndex % kIconsPerRow) * kIconSize;
    const int32_t iconY = (iconIndex / kIconsPerRow) * kIconSize;

    urpg::SpriteCommand spriteCmd;
    spriteCmd.textureId = "IconSet";
    spriteCmd.srcX = iconX;
    spriteCmd.srcY = iconY;
    spriteCmd.width = kIconSize;
    spriteCmd.height = kIconSize;
    spriteCmd.x = static_cast<float>(rect_.x + padding_ + x);
    spriteCmd.y = static_cast<float>(rect_.y + padding_ + y);
    spriteCmd.zOrder = 100;
    urpg::RenderLayer::getInstance().submit(urpg::toFrameRenderCommand(spriteCmd));
}

void Window_Base::drawActorFace(int32_t actorId, int32_t x, int32_t y, int32_t width, int32_t height) {
    recordMethodCall("drawActorFace");
    if (actorId <= 0 || width <= 0 || height <= 0) {
        lastFaceDraw_.reset();
        return;
    }

    DataManager& data = DataManager::instance();
    const ActorData* actor = data.getActor(actorId);
    const int32_t faceIndex = resolveActorFaceIndex(actor, actorId);

    // Mirror MZ face-draw clipping/centering math.
    const int32_t sw = std::min(width, kFaceCellWidth);
    const int32_t sh = std::min(height, kFaceCellHeight);
    const int32_t dx = x + std::max(width - kFaceCellWidth, 0) / 2;
    const int32_t dy = y + std::max(height - kFaceCellHeight, 0) / 2;
    const int32_t sx = (faceIndex % kFaceSheetCols) * kFaceCellWidth + (kFaceCellWidth - sw) / 2;
    const int32_t sy = (faceIndex / kFaceSheetCols) * kFaceCellHeight + (kFaceCellHeight - sh) / 2;

    Window_Base::FaceDrawInfo info;
    info.actorId = actorId;
    info.faceName = resolveActorFaceName(actor, actorId);
    info.faceIndex = faceIndex;
    info.sourceRect = Rect{sx, sy, sw, sh};
    info.destRect = Rect{dx, dy, sw, sh};
    lastFaceDraw_ = info;

    urpg::SpriteCommand spriteCmd;
    spriteCmd.textureId = info.faceName;
    spriteCmd.x = static_cast<float>(rect_.x + padding_ + dx);
    spriteCmd.y = static_cast<float>(rect_.y + padding_ + dy);
    spriteCmd.zOrder = 100;
    spriteCmd.srcX = sx;
    spriteCmd.srcY = sy;
    spriteCmd.width = sw;
    spriteCmd.height = sh;
    urpg::RenderLayer::getInstance().submit(urpg::toFrameRenderCommand(spriteCmd));
}

void Window_Base::drawActorName(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawActorName");
    std::string name = "Actor " + std::to_string(std::max(0, actorId));
    if (const auto* actor = DataManager::instance().getActor(actorId)) {
        if (!actor->name.empty()) {
            name = actor->name;
        }
    }
    drawText(name, x, y, width);
}

void Window_Base::drawActorLevel(int32_t actorId, int32_t x, int32_t y) {
    recordMethodCall("drawActorLevel");
    int32_t level = 1;
    if (const auto* actor = DataManager::instance().getActor(actorId)) {
        level = actor->level;
    }
    drawText("Lv " + std::to_string(level), x, y);
}

void Window_Base::drawActorHp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawActorHp");
    assert(actorId >= 0);
    DataManager& data = DataManager::instance();
    const ActorData* actor = data.getActor(actorId);
    const int32_t maxHp = resolveActorGaugeMax(data, actor, actorId, 0, 100);
    const int32_t currentHp = resolveActorGaugeCurrent(actor, maxHp, maxHp, "hp");
    drawActorResourceGauge(*this, x, y, width, "HP", currentHp, maxHp, systemColor(16), systemColor(20));
}

void Window_Base::drawActorMp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawActorMp");
    assert(actorId >= 0);
    DataManager& data = DataManager::instance();
    const ActorData* actor = data.getActor(actorId);
    const int32_t maxMp = resolveActorGaugeMax(data, actor, actorId, 1, 30);
    const int32_t currentMp = resolveActorGaugeCurrent(actor, maxMp, maxMp, "mp");
    drawActorResourceGauge(*this, x, y, width, "MP", currentMp, maxMp, systemColor(17), systemColor(5));
}

void Window_Base::drawActorTp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawActorTp");
    assert(actorId >= 0);
    DataManager& data = DataManager::instance();
    const ActorData* actor = data.getActor(actorId);
    const int32_t maxTp = 100;
    const int32_t currentTp = resolveActorGaugeCurrent(actor, maxTp, maxTp, "tp");
    drawActorResourceGauge(*this, x, y, width, "TP", currentTp, maxTp, systemColor(18), systemColor(6));
}

void Window_Base::drawGauge(int32_t x, int32_t y, int32_t width, double rate, const Color& color1,
                            const Color& color2) {
    recordMethodCall("drawGauge");
    assert(width > 0);
    assert(rate >= 0.0 && rate <= 1.0);

    constexpr int32_t kGaugeHeight = 12;
    const int32_t fillWidth = static_cast<int32_t>(static_cast<double>(width) * std::clamp(rate, 0.0, 1.0));
    const float baseX = static_cast<float>(rect_.x + padding_ + x);
    const float baseY = static_cast<float>(rect_.y + padding_ + y);

    // Background rect
    urpg::RectCommand bgCmd;
    bgCmd.x = baseX;
    bgCmd.y = baseY;
    bgCmd.w = static_cast<float>(width);
    bgCmd.h = static_cast<float>(kGaugeHeight);
    bgCmd.r = 0.2f;
    bgCmd.g = 0.2f;
    bgCmd.b = 0.2f;
    bgCmd.a = 1.0f;
    bgCmd.zOrder = 100;
    urpg::RenderLayer::getInstance().submit(urpg::toFrameRenderCommand(bgCmd));

    if (fillWidth <= 0) {
        return;
    }

    const int32_t segmentCount = std::min<int32_t>(8, std::max<int32_t>(1, fillWidth));
    for (int32_t segment = 0; segment < segmentCount; ++segment) {
        const int32_t start = (fillWidth * segment) / segmentCount;
        const int32_t end = (fillWidth * (segment + 1)) / segmentCount;
        const int32_t segmentWidth = std::max(0, end - start);
        if (segmentWidth <= 0) {
            continue;
        }

        const double t = segmentCount == 1 ? 0.0 : static_cast<double>(segment) / static_cast<double>(segmentCount - 1);
        const Color segmentColor = lerpColor(color1, color2, t);

        urpg::RectCommand fillCmd;
        fillCmd.x = baseX + static_cast<float>(start);
        fillCmd.y = baseY;
        fillCmd.w = static_cast<float>(segmentWidth);
        fillCmd.h = static_cast<float>(kGaugeHeight);
        fillCmd.r = segmentColor.r / 255.0f;
        fillCmd.g = segmentColor.g / 255.0f;
        fillCmd.b = segmentColor.b / 255.0f;
        fillCmd.a = segmentColor.a / 255.0f;
        fillCmd.zOrder = 101;
        urpg::RenderLayer::getInstance().submit(urpg::toFrameRenderCommand(fillCmd));
    }
}

void Window_Base::drawCharacter(const std::string& characterName, int32_t index, int32_t x, int32_t y) {
    recordMethodCall("drawCharacter");
    assert(!characterName.empty());
    assert(index >= 0);

    // MZ standard character sheet: 4 columns × 2 rows of characters.
    // Each character occupies a 3×4 block of 48×48 cells on a 576×384 sheet
    // (12 cells wide × 8 cells tall). The standing frame is at offset
    // (+1, +0) within the character block.
    constexpr int32_t kCharSheetCols = 4;
    constexpr int32_t kCharCellWidth = 48;
    constexpr int32_t kCharCellHeight = 48;
    const int32_t srcX = ((index % kCharSheetCols) * 3 + 1) * kCharCellWidth;
    const int32_t srcY = (index / kCharSheetCols) * 4 * kCharCellHeight;

    urpg::SpriteCommand spriteCmd;
    spriteCmd.textureId = characterName;
    spriteCmd.x = static_cast<float>(rect_.x + padding_ + x);
    spriteCmd.y = static_cast<float>(rect_.y + padding_ + y);
    spriteCmd.zOrder = 100;
    spriteCmd.srcX = srcX;
    spriteCmd.srcY = srcY;
    spriteCmd.width = kCharCellWidth;
    spriteCmd.height = kCharCellHeight;
    urpg::RenderLayer::getInstance().submit(urpg::toFrameRenderCommand(spriteCmd));
}

void Window_Base::drawItemName(int32_t itemId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawItemName");
    if (itemId <= 0) {
        return;
    }

    DataManager& data = DataManager::instance();
    const ItemData* item = resolveAnyItemById(data, itemId);
    const int32_t iconIndex = resolveItemIconIndex(item);
    const std::string label = resolveItemLabel(item, itemId);
    const int32_t safeWidth = std::max(0, width);

    const int32_t iconY = y + std::max(0, (lineHeight() - kIconWidth) / 2);
    drawIcon(iconIndex, x, iconY);

    const int32_t textX = x + kIconWidth + kIconSpacing;
    const int32_t textWidth = safeWidth > 0 ? std::max(0, safeWidth - (kIconWidth + kIconSpacing)) : 0;
    drawText(label, textX, y, textWidth);
}

void Window_Base::open() {
    recordMethodCall("open");
    isOpen_ = true;
    isVisible_ = true;
}

void Window_Base::close() {
    recordMethodCall("close");
    isOpen_ = false;
}

void Window_Base::show() {
    recordMethodCall("show");
    isVisible_ = true;
}

void Window_Base::hide() {
    recordMethodCall("hide");
    isVisible_ = false;
}

Rect Window_Base::getContentRect() const {
    methodCallCounts_["getContentRect"]++;
    const int32_t innerWidth = std::max(0, rect_.width - padding_ * 2);
    const int32_t innerHeight = std::max(0, rect_.height - padding_ * 2);
    return Rect{rect_.x + padding_, rect_.y + padding_, innerWidth, innerHeight};
}

void Window_Base::update() {
    recordMethodCall("update");
    // Base update - override in subclasses
}

// ============================================================================
// Extended Window_Base Methods
// ============================================================================

void Window_Base::drawTextEx(const std::string& text, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawTextEx");
    const std::string activeAlign = normalizeAlignmentString(textAlignment_);
    message::RichTextLayoutEngine layout =
        buildLayoutEngineForWindow(*this, std::max(0, width), parseMessageAlignment(activeAlign));
    const message::RichTextLayoutResult laidOut = layout.layout(text);
    const auto& tokens = laidOut.tokens;
    const int32_t baseLineHeight = lineHeight();
    int32_t cursorX = x;
    int32_t cursorY = y;
    int32_t currentLineHeight = std::max(1, baseLineHeight);
    int32_t currentFontSize = std::max(1, fontSize_);
    const Color originalColor = textColor_;

    for (const auto& token : tokens) {
        switch (token.type) {
        case message::RichTextTokenType::Text: {
            if (!token.text.empty()) {
                drawText(token.text, cursorX, cursorY, 0, "left");
                cursorX += measurePlainTextRenderer(token.text, currentFontSize);
                currentLineHeight = std::max(currentLineHeight, std::max(baseLineHeight, currentFontSize + 8));
            }
            break;
        }
        case message::RichTextTokenType::Icon:
            drawIcon(token.value, cursorX, cursorY);
            cursorX += kIconWidth + kIconSpacing;
            currentLineHeight = std::max(currentLineHeight, std::max(baseLineHeight, kIconWidth));
            break;
        case message::RichTextTokenType::Color:
            changeTextColor(token.value);
            break;
        case message::RichTextTokenType::FontBigger:
            currentFontSize = std::min(kFontSizeMax, currentFontSize + kFontStep);
            currentLineHeight = std::max(currentLineHeight, std::max(baseLineHeight, currentFontSize + 8));
            break;
        case message::RichTextTokenType::FontSmaller:
            currentFontSize = std::max(kFontSizeMin, currentFontSize - kFontStep);
            currentLineHeight = std::max(currentLineHeight, std::max(baseLineHeight, currentFontSize + 8));
            break;
        case message::RichTextTokenType::NewLine:
            cursorX = x;
            cursorY += currentLineHeight;
            currentLineHeight = std::max(1, baseLineHeight);
            break;
        case message::RichTextTokenType::LineOffset:
            cursorX = x + token.value;
            break;
        }
    }

    textColor_ = originalColor;
}

int32_t Window_Base::lineHeight() const {
    methodCallCounts_["lineHeight"]++;
    // MZ default line height
    return 36;
}

int32_t Window_Base::textWidth(const std::string& text) const {
    methodCallCounts_["textWidth"]++;
    message::RichTextLayoutEngine layout = buildLayoutEngineForWindow(*this, 0, message::MessageAlignment::Left);
    return layout.textWidth(text);
}

Rect Window_Base::textSize(const std::string& text) const {
    methodCallCounts_["textSize"]++;
    message::RichTextLayoutEngine layout = buildLayoutEngineForWindow(*this, 0, message::MessageAlignment::Left);
    const message::RichTextLayoutResult result = layout.layout(text);
    return Rect{0, 0, result.metrics.width, result.metrics.height};
}

void Window_Base::changeTextColor(const Color& color) {
    recordMethodCall("changeTextColor");
    textColor_ = color;
}

void Window_Base::changeTextColor(int32_t systemColorIndex) {
    recordMethodCall("changeTextColor");
    textColor_ = systemColor(systemColorIndex);
}

void Window_Base::resetTextColor() {
    recordMethodCall("resetTextColor");
    textColor_ = Color{255, 255, 255, 255};
}

Color Window_Base::textColor() const {
    methodCallCounts_["textColor"]++;
    return textColor_;
}

Color Window_Base::systemColor(int32_t index) const {
    methodCallCounts_["systemColor"]++;
    // MZ system colors (simplified palette)
    static const Color systemColors[] = {
        {0, 0, 0, 255},       // 0: Black
        {255, 255, 255, 255}, // 1: White
        {128, 128, 128, 255}, // 2: Gray
        {255, 128, 128, 255}, // 3: Light red
        {128, 255, 128, 255}, // 4: Light green
        {128, 128, 255, 255}, // 5: Light blue
        {255, 255, 128, 255}, // 6: Yellow
        {255, 128, 255, 255}, // 7: Magenta
        {128, 255, 255, 255}, // 8: Cyan
        {255, 0, 0, 255},     // 9: Red
        {0, 255, 0, 255},     // 10: Green
        {0, 0, 255, 255},     // 11: Blue
        {255, 200, 0, 255},   // 12: Gold
        {192, 192, 192, 255}, // 13: Silver
        {80, 80, 80, 255},    // 14: Dark gray
        {160, 160, 160, 255}, // 15: Medium gray
        {255, 96, 96, 255},   // 16: HP color
        {96, 192, 255, 255},  // 17: MP color
        {255, 128, 64, 255},  // 18: TP color
        {64, 255, 64, 255},   // 19: Power up
        {255, 64, 64, 255},   // 20: Power down
    };
    constexpr int numColors = sizeof(systemColors) / sizeof(systemColors[0]);
    if (index >= 0 && index < numColors) {
        return systemColors[index];
    }
    return Color{255, 255, 255, 255}; // Default white
}

Color Window_Base::normalColor() const {
    return systemColor(0);
}

Color Window_Base::dimColor() const {
    return Color{128, 128, 128, 255};
}

void Window_Base::resetFontSettings() {
    recordMethodCall("resetFontSettings");
    fontFace_ = "Microsoft YaHei";
    fontSize_ = 22;
    resetTextColor();
}

std::string Window_Base::fontFace() const {
    methodCallCounts_["fontFace"]++;
    return fontFace_;
}

int32_t Window_Base::fontSize() const {
    methodCallCounts_["fontSize"]++;
    return fontSize_;
}

void Window_Base::setFontFace(const std::string& face) {
    recordMethodCall("setFontFace");
    fontFace_ = face;
}

void Window_Base::setFontSize(int32_t size) {
    recordMethodCall("setFontSize");
    fontSize_ = std::max(1, size);
}

void Window_Base::setTextAlignment(const std::string& align) {
    recordMethodCall("setTextAlignment");
    textAlignment_ = normalizeAlignmentString(align);
}

std::string Window_Base::textAlignment() const {
    methodCallCounts_["textAlignment"]++;
    return textAlignment_;
}

std::optional<Window_Base::ContentsBitmapInfo> Window_Base::getContentsBitmapInfo() const {
    if (contents_ == INVALID_BITMAP) {
        return std::nullopt;
    }
    const auto it = contentsBitmaps_.find(contents_);
    if (it == contentsBitmaps_.end()) {
        return std::nullopt;
    }
    return it->second;
}

BitmapHandle Window_Base::contents() const {
    methodCallCounts_["contents"]++;
    return contents_;
}

void Window_Base::syncContentsBitmap() {
    if (contents_ == INVALID_BITMAP) {
        return;
    }

    const Rect contentRect = getContentRect();
    ContentsBitmapInfo& info = contentsBitmaps_[contents_];
    info.handle = contents_;
    info.width = std::max(0, contentRect.width);
    info.height = std::max(0, contentRect.height);
    info.pixels.assign(static_cast<size_t>(info.width) * static_cast<size_t>(info.height), Color{0, 0, 0, 0});
}

void Window_Base::createContents() {
    recordMethodCall("createContents");
    if (contents_ != INVALID_BITMAP) {
        syncContentsBitmap();
        return;
    }

    contents_ = nextBitmapHandle_++;
    if (contents_ == INVALID_BITMAP) {
        contents_ = nextBitmapHandle_++;
    }
    syncContentsBitmap();
}

void Window_Base::destroyContents() {
    recordMethodCall("destroyContents");
    if (contents_ != INVALID_BITMAP) {
        contentsBitmaps_.erase(contents_);
    }
    contents_ = INVALID_BITMAP;
}

void Window_Base::recordMethodCall(const std::string& methodName) {
    initializeMethodStatus(); // Ensure initialized
    auto it = methodCallCounts_.find(methodName);
    if (it != methodCallCounts_.end()) {
        ++it->second;
    }
}

CompatStatus Window_Base::getMethodStatus(const std::string& methodName) {
    initializeMethodStatus(); // Ensure initialized
    auto it = methodStatus_.find(methodName);
    return it != methodStatus_.end() ? it->second : CompatStatus::UNSUPPORTED;
}

std::string Window_Base::getMethodDeviation(const std::string& methodName) {
    initializeMethodStatus(); // Ensure initialized
    auto it = methodDeviations_.find(methodName);
    return it != methodDeviations_.end() ? it->second : "";
}

uint32_t Window_Base::getMethodCallCount(const std::string& methodName) {
    initializeMethodStatus(); // Ensure initialized
    auto it = methodCallCounts_.find(methodName);
    return it != methodCallCounts_.end() ? it->second : 0;
}

std::vector<std::string> Window_Base::getTrackedMethods() {
    initializeMethodStatus(); // Ensure initialized
    std::vector<std::string> methodNames;
    methodNames.reserve(methodStatus_.size());
    for (const auto& [methodName, status] : methodStatus_) {
        (void)status;
        methodNames.push_back(methodName);
    }
    std::sort(methodNames.begin(), methodNames.end());
    return methodNames;
}

void Window_Base::setDefaultInstance(Window_Base* instance) {
    defaultInstance_ = instance;
}

void Window_Base::registerAPI(QuickJSContext& ctx) {
    initializeMethodStatus(); // Ensure initialized

    auto getInt = [](const Value& v, int64_t defaultVal = 0) -> int64_t {
        if (auto* p = std::get_if<int64_t>(&v.v))
            return *p;
        if (auto* p = std::get_if<double>(&v.v))
            return static_cast<int64_t>(*p);
        return defaultVal;
    };
    auto getDouble = [](const Value& v, double defaultVal = 0.0) -> double {
        if (auto* p = std::get_if<double>(&v.v))
            return *p;
        if (auto* p = std::get_if<int64_t>(&v.v))
            return static_cast<double>(*p);
        return defaultVal;
    };
    auto getString = [](const Value& v, const std::string& defaultVal = "") -> std::string {
        if (auto* p = std::get_if<std::string>(&v.v))
            return *p;
        return defaultVal;
    };

    auto dispatch = [](std::function<Value(Window_Base&)> fn) -> Value {
        if (!defaultInstance_)
            return Value::Nil();
        return fn(*defaultInstance_);
    };

    std::vector<QuickJSContext::MethodDef> methods;

    auto add = [&](const std::string& name, std::function<Value(const std::vector<Value>&)> fn) {
        QuickJSContext::MethodDef method;
        method.name = name;
        method.fn = fn;
        method.status = getMethodStatus(name);
        method.deviationNote = getMethodDeviation(name);
        methods.push_back(method);
    };

    add("drawText", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const std::string text = args.size() > 0 ? getString(args[0]) : "";
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const int32_t maxWidth = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 0);
            const std::string align = args.size() > 4 ? getString(args[4]) : "left";
            w.drawText(text, x, y, maxWidth, align);
            return Value::Int(1);
        });
    });

    add("drawIcon", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t iconIndex = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            w.drawIcon(iconIndex, x, y);
            return Value::Int(1);
        });
    });

    add("drawActorFace", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t actorId = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const int32_t width = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 144);
            const int32_t height = static_cast<int32_t>(args.size() > 4 ? getInt(args[4]) : 144);
            w.drawActorFace(actorId, x, y, width, height);
            return Value::Int(1);
        });
    });

    add("drawActorName", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t actorId = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const int32_t width = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 150);
            w.drawActorName(actorId, x, y, width);
            return Value::Int(1);
        });
    });

    add("drawActorLevel", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t actorId = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            w.drawActorLevel(actorId, x, y);
            return Value::Int(1);
        });
    });

    add("drawActorHp", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t actorId = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const int32_t width = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 128);
            w.drawActorHp(actorId, x, y, width);
            return Value::Int(1);
        });
    });

    add("drawActorMp", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t actorId = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const int32_t width = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 128);
            w.drawActorMp(actorId, x, y, width);
            return Value::Int(1);
        });
    });

    add("drawActorTp", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t actorId = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const int32_t width = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 128);
            w.drawActorTp(actorId, x, y, width);
            return Value::Int(1);
        });
    });

    add("drawGauge", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t x = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t width = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const double rate = args.size() > 3 ? getDouble(args[3]) : 0.0;
            Color color1 = Color{255, 255, 255, 255};
            Color color2 = Color{255, 255, 255, 255};
            if (args.size() > 4) {
                color1 = colorFromValue(args[4], &w).value_or(color1);
            }
            if (args.size() > 5) {
                color2 = colorFromValue(args[5], &w).value_or(color2);
            }
            w.drawGauge(x, y, width, rate, color1, color2);
            return Value::Int(1);
        });
    });

    add("drawCharacter", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const std::string name = args.size() > 0 ? getString(args[0]) : "";
            const int32_t index = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t x = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 0);
            w.drawCharacter(name, index, x, y);
            return Value::Int(1);
        });
    });

    add("drawItemName", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t itemId = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const int32_t width = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 312);
            w.drawItemName(itemId, x, y, width);
            return Value::Int(1);
        });
    });

    add("drawTextEx", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const std::string text = args.size() > 0 ? getString(args[0]) : "";
            const int32_t x = static_cast<int32_t>(args.size() > 1 ? getInt(args[1]) : 0);
            const int32_t y = static_cast<int32_t>(args.size() > 2 ? getInt(args[2]) : 0);
            const int32_t width = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 0);
            w.drawTextEx(text, x, y, width);
            return Value::Int(1);
        });
    });

    add("lineHeight", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) { return Value::Int(w.lineHeight()); });
    });

    add("textWidth", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const std::string text = args.size() > 0 ? getString(args[0]) : "";
            return Value::Int(w.textWidth(text));
        });
    });

    add("textSize", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const std::string text = args.size() > 0 ? getString(args[0]) : "";
            const Rect r = w.textSize(text);
            urpg::Object obj;
            obj["x"] = Value::Int(r.x);
            obj["y"] = Value::Int(r.y);
            obj["width"] = Value::Int(r.width);
            obj["height"] = Value::Int(r.height);
            return Value::Obj(std::move(obj));
        });
    });

    add("changeTextColor", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            if (args.size() > 0) {
                if (auto* p = std::get_if<int64_t>(&args[0].v)) {
                    w.changeTextColor(static_cast<int32_t>(*p));
                } else if (const auto color = colorFromValue(args[0], &w)) {
                    w.changeTextColor(*color);
                }
            }
            return Value::Int(1);
        });
    });

    add("resetTextColor", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            w.resetTextColor();
            return Value::Int(1);
        });
    });

    add("textColor", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            const Color c = w.textColor();
            urpg::Object obj;
            obj["r"] = Value::Int(c.r);
            obj["g"] = Value::Int(c.g);
            obj["b"] = Value::Int(c.b);
            obj["a"] = Value::Int(c.a);
            return Value::Obj(std::move(obj));
        });
    });

    add("systemColor", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            const int32_t index = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            const Color c = w.systemColor(index);
            urpg::Object obj;
            obj["r"] = Value::Int(c.r);
            obj["g"] = Value::Int(c.g);
            obj["b"] = Value::Int(c.b);
            obj["a"] = Value::Int(c.a);
            return Value::Obj(std::move(obj));
        });
    });

    add("resetFontSettings", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            w.resetFontSettings();
            return Value::Int(1);
        });
    });

    add("fontFace", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            urpg::Value v;
            v.v = w.fontFace();
            return v;
        });
    });

    add("fontSize", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) { return Value::Int(w.fontSize()); });
    });

    add("setFontFace", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            w.setFontFace(args.size() > 0 ? getString(args[0]) : "");
            return Value::Int(1);
        });
    });

    add("setFontSize", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            w.setFontSize(static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 22));
            return Value::Int(1);
        });
    });

    add("setTextAlignment", [&](const std::vector<Value>& args) -> Value {
        return dispatch([&](Window_Base& w) {
            w.setTextAlignment(args.size() > 0 ? getString(args[0]) : "left");
            return Value::Int(1);
        });
    });

    add("textAlignment", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            urpg::Value v;
            v.v = w.textAlignment();
            return v;
        });
    });

    add("contents", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) { return Value::Int(static_cast<int64_t>(w.contents())); });
    });

    add("createContents", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            w.createContents();
            return Value::Int(1);
        });
    });

    add("destroyContents", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            w.destroyContents();
            return Value::Int(1);
        });
    });

    add("open", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            w.open();
            return Value::Int(1);
        });
    });

    add("close", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            w.close();
            return Value::Int(1);
        });
    });

    add("show", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            w.show();
            return Value::Int(1);
        });
    });

    add("hide", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            w.hide();
            return Value::Int(1);
        });
    });

    add("update", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            w.update();
            return Value::Int(1);
        });
    });

    add("getContentRect", [&](const std::vector<Value>&) -> Value {
        return dispatch([&](Window_Base& w) {
            const Rect r = w.getContentRect();
            urpg::Object obj;
            obj["x"] = Value::Int(r.x);
            obj["y"] = Value::Int(r.y);
            obj["width"] = Value::Int(r.width);
            obj["height"] = Value::Int(r.height);
            return Value::Obj(std::move(obj));
        });
    });

    ctx.registerObject("Window_Base", methods);
}

// ============================================================================
// Window_Selectable Implementation
// ============================================================================

Window_Selectable::Window_Selectable(const CreateParams& params)
    : Window_Base(params), maxCols_(std::max(1, params.maxCols)), itemHeight_(std::max(1, params.itemHeight)),
      numVisibleRows_(std::max(1, params.numVisibleRows)) {}

void Window_Selectable::setMaxItems(int32_t count) {
    const bool hadNoItems = (maxItems_ == 0);
    maxItems_ = std::max(0, count);
    if (maxItems_ == 0) {
        index_ = -1;
        topRow_ = 0;
        return;
    }
    if (hadNoItems && index_ < 0) {
        index_ = 0;
    }
    setIndex(index_);
    setTopRow(topRow_);
}

void Window_Selectable::setMaxCols(int32_t cols) {
    maxCols_ = std::max(1, cols);
    setIndex(index_);
    setTopRow(topRow_);
}

void Window_Selectable::setItemHeight(int32_t height) {
    itemHeight_ = std::max(1, height);
}

void Window_Selectable::setIndex(int32_t index) {
    const int32_t previousIndex = index_;
    index_ = std::clamp(index, -1, std::max(-1, maxItems_ - 1));

    // Ensure cursor is visible
    const int32_t row = getRow();
    const int32_t topRow = getTopRow();

    if (row < topRow) {
        setTopRow(row);
    } else if (row >= topRow + numVisibleRows_) {
        setTopRow(row - numVisibleRows_ + 1);
    }

    if (index_ != previousIndex && onSelect_) {
        onSelect_(index_);
    }
}

int32_t Window_Selectable::getItemWidth() const {
    const int32_t safeMaxCols = std::max(1, maxCols_);
    const int32_t spacing = 8; // MZ default
    const int32_t totalSpacing = spacing * (safeMaxCols - 1);
    const int32_t availableWidth = std::max(0, getContentRect().width - totalSpacing);
    return availableWidth / safeMaxCols;
}

void Window_Selectable::cursorDown(bool wrap) {
    if (maxItems_ <= 0)
        return;

    const int32_t safeMaxCols = std::max(1, maxCols_);
    int32_t maxRow = (maxItems_ + safeMaxCols - 1) / safeMaxCols - 1;
    int32_t currentRow = getRow();

    if (currentRow < maxRow) {
        setIndex(index_ + safeMaxCols);
    } else if (wrap && safeMaxCols == 1) {
        setIndex(0);
    }
}

void Window_Selectable::cursorUp(bool wrap) {
    if (maxItems_ <= 0)
        return;

    const int32_t safeMaxCols = std::max(1, maxCols_);
    int32_t currentRow = getRow();

    if (currentRow > 0) {
        setIndex(index_ - safeMaxCols);
    } else if (wrap && safeMaxCols == 1) {
        setIndex(maxItems_ - 1);
    }
}

void Window_Selectable::cursorRight(bool wrap) {
    const int32_t safeMaxCols = std::max(1, maxCols_);
    if (safeMaxCols < 2)
        return;

    int32_t currentCol = getCol();
    int32_t maxCol = std::min(safeMaxCols, maxItems_ - getRow() * safeMaxCols) - 1;

    if (currentCol < maxCol) {
        setIndex(index_ + 1);
    } else if (wrap) {
        cursorDown(false);
        if (index_ < maxItems_) {
            setIndex(getRow() * safeMaxCols);
        }
    }
}

void Window_Selectable::cursorLeft(bool wrap) {
    const int32_t safeMaxCols = std::max(1, maxCols_);
    if (safeMaxCols < 2)
        return;

    int32_t currentCol = getCol();

    if (currentCol > 0) {
        setIndex(index_ - 1);
    } else if (wrap) {
        int32_t prevRow = getRow() - 1;
        if (prevRow >= 0) {
            int32_t maxCol = std::min(safeMaxCols, maxItems_ - prevRow * safeMaxCols) - 1;
            setIndex(prevRow * safeMaxCols + maxCol);
        }
    }
}

void Window_Selectable::cursorPagedown() {
    if (maxItems_ <= 0)
        return;

    const int32_t safeMaxCols = std::max(1, maxCols_);
    int32_t newIndex = index_ + numVisibleRows_ * safeMaxCols;
    if (newIndex < maxItems_) {
        setIndex(newIndex);
    } else {
        setIndex(maxItems_ - 1);
    }
}

void Window_Selectable::cursorPageup() {
    if (maxItems_ <= 0)
        return;

    const int32_t safeMaxCols = std::max(1, maxCols_);
    int32_t newIndex = index_ - numVisibleRows_ * safeMaxCols;
    if (newIndex >= 0) {
        setIndex(newIndex);
    } else {
        setIndex(0);
    }
}

int32_t Window_Selectable::getRow() const {
    const int32_t safeMaxCols = std::max(1, maxCols_);
    return index_ >= 0 ? index_ / safeMaxCols : 0;
}

int32_t Window_Selectable::getCol() const {
    const int32_t safeMaxCols = std::max(1, maxCols_);
    return index_ >= 0 ? index_ % safeMaxCols : 0;
}

void Window_Selectable::setTopRow(int32_t row) {
    int32_t maxTop = getMaxTopRow();
    topRow_ = std::clamp(row, 0, maxTop);
}

int32_t Window_Selectable::getMaxTopRow() const {
    if (maxItems_ <= 0)
        return 0;
    const int32_t safeMaxCols = std::max(1, maxCols_);
    int32_t maxRow = (maxItems_ + safeMaxCols - 1) / safeMaxCols - 1;
    return std::max(0, maxRow - numVisibleRows_ + 1);
}

void Window_Selectable::update() {
    Window_Base::update();
    processCursorMove();
    processHandling();
}

void Window_Selectable::processHandling() {
    if (!isOpen() || !isActive()) {
        pointerPressActive_ = false;
        pointerPressIsTouch_ = false;
        pointerPressMoved_ = false;
        pointerPressIndex_ = -1;
        pointerLastIndex_ = -1;
        return;
    }

    InputManager& input = InputManager::instance();
    if (input.isTriggered(InputKey::DECISION) || input.isRepeated(InputKey::DECISION)) {
        processOk();
    }
    if (input.isTriggered(InputKey::CANCEL) || input.isRepeated(InputKey::CANCEL)) {
        processCancel();
    }

    if (!isPrimaryPointerPressed(input) && pointerPressActive_) {
        const int32_t releaseIndex = hitTestSelectableIndexAt(*this, getPrimaryPointerX(input, pointerPressIsTouch_),
                                                              getPrimaryPointerY(input, pointerPressIsTouch_));
        if (!pointerPressMoved_ && pointerPressIndex_ >= 0 && releaseIndex == pointerPressIndex_ &&
            getIndex() == pointerPressIndex_) {
            processOk();
        }

        pointerPressActive_ = false;
        pointerPressIsTouch_ = false;
        pointerPressMoved_ = false;
        pointerPressIndex_ = -1;
        pointerLastIndex_ = -1;
    }
}

void Window_Selectable::processCursorMove() {
    if (!isCursorMovable()) {
        return;
    }

    InputManager& input = InputManager::instance();
    const bool pointerPressed = isPrimaryPointerPressed(input);
    const bool pointerTriggered = isPrimaryPointerTriggered(input);
    if (hasPrimaryPointerPosition(input)) {
        const int32_t pointerX = getPrimaryPointerX(input);
        const int32_t pointerY = getPrimaryPointerY(input);
        const int32_t pointedIndex = hitTestSelectableIndexAt(*this, pointerX, pointerY);

        if (pointerTriggered || (pointerPressed && !pointerPressActive_)) {
            pointerPressActive_ = pointerPressed || pointerTriggered;
            pointerPressIsTouch_ = isTouchPrimary(input);
            pointerPressMoved_ = false;
            pointerPressIndex_ = pointedIndex;
            pointerLastIndex_ = pointedIndex;
        }

        if (pointedIndex >= 0) {
            if (pointerPressActive_) {
                if ((pointerPressIndex_ >= 0 && pointedIndex != pointerPressIndex_) ||
                    (pointerLastIndex_ >= 0 && pointedIndex != pointerLastIndex_)) {
                    pointerPressMoved_ = true;
                }
                pointerLastIndex_ = pointedIndex;
            }
            setIndex(pointedIndex);
            return;
        }

        if (pointerPressed) {
            const Rect contentRect = getContentRect();
            if (pointerX >= contentRect.x && pointerX < contentRect.x + contentRect.width) {
                const int32_t safeMaxCols = std::max(1, getMaxCols());
                if (pointerY < contentRect.y && getTopRow() > 0) {
                    pointerPressMoved_ = true;
                    setTopRow(getTopRow() - 1);
                    setIndex(std::max(0, getIndex() - safeMaxCols));
                    return;
                }
                if (pointerY >= contentRect.y + contentRect.height && getTopRow() < getMaxTopRow()) {
                    pointerPressMoved_ = true;
                    setTopRow(getTopRow() + 1);
                    setIndex(std::min(getMaxItems() - 1, getIndex() + safeMaxCols));
                    return;
                }
            }
        }
    }

    const int32_t wheelDelta = input.getMouseWheel();
    if (wheelDelta != 0 && isPointerInsideSelectableContent(*this, input.getMouseX(), input.getMouseY())) {
        if (wheelDelta < 0) {
            cursorDown(false);
        } else {
            cursorUp(false);
        }
        return;
    }

    if (input.isDirectionTriggered(2) || input.isDirectionTriggered(6) || input.isDirectionTriggered(4) ||
        input.isDirectionTriggered(8)) {
        if (input.isDirectionTriggered(2)) {
            cursorDown(true);
        } else if (input.isDirectionTriggered(8)) {
            cursorUp(true);
        } else if (input.isDirectionTriggered(6)) {
            cursorRight(true);
        } else if (input.isDirectionTriggered(4)) {
            cursorLeft(true);
        }
        return;
    }

    if (input.isDirectionPressed(2) && input.isRepeated(InputKey::DOWN)) {
        cursorDown(true);
    } else if (input.isDirectionPressed(8) && input.isRepeated(InputKey::UP)) {
        cursorUp(true);
    } else if (input.isDirectionPressed(6) && input.isRepeated(InputKey::RIGHT)) {
        cursorRight(true);
    } else if (input.isDirectionPressed(4) && input.isRepeated(InputKey::LEFT)) {
        cursorLeft(true);
    }
}

void Window_Selectable::processPagedown() {
    cursorPagedown();
}

void Window_Selectable::processPageup() {
    cursorPageup();
}

bool Window_Selectable::isCursorMovable() const {
    return isOpen() && isActive() && maxItems_ > 0;
}

bool Window_Selectable::isHandled(const std::string& symbol) const {
    (void)symbol;
    return false;
}

void Window_Selectable::processOk() {
    // Base implementation: nothing
}

void Window_Selectable::processCancel() {
    // Base implementation: nothing
}

void Window_Selectable::registerAPI(QuickJSContext& ctx) {
    // First register Window_Base methods
    Window_Base::registerAPI(ctx);

    auto getInt = [](const Value& v, int64_t defaultVal = 0) -> int64_t {
        if (auto* p = std::get_if<int64_t>(&v.v))
            return *p;
        if (auto* p = std::get_if<double>(&v.v))
            return static_cast<int64_t>(*p);
        return defaultVal;
    };
    auto getString = [](const Value& v, const std::string& defaultVal = "") -> std::string {
        if (auto* p = std::get_if<std::string>(&v.v))
            return *p;
        return defaultVal;
    };

    auto dispatchSelectable = [&](std::function<Value(Window_Selectable&)> fn) -> Value {
        if (!defaultInstance_)
            return Value::Nil();
        auto* sel = dynamic_cast<Window_Selectable*>(defaultInstance_);
        if (!sel)
            return Value::Nil();
        return fn(*sel);
    };

    std::vector<QuickJSContext::MethodDef> methods;

    auto add = [&](const std::string& name, std::function<Value(const std::vector<Value>&)> fn) {
        QuickJSContext::MethodDef method;
        method.name = name;
        method.fn = fn;
        method.status = CompatStatus::FULL;
        methods.push_back(method);
    };

    add("index", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getIndex()); });
    });

    add("select", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const int32_t idx = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            w.select(idx);
            return Value::Int(1);
        });
    });

    add("topRow", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getTopRow()); });
    });

    add("maxItems", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getMaxItems()); });
    });

    add("maxCols", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getMaxCols()); });
    });

    add("itemHeight", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getItemHeight()); });
    });

    add("itemWidth", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getItemWidth()); });
    });

    add("cursorDown", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const bool wrap = args.size() > 0 ? (getInt(args[0]) != 0) : true;
            w.cursorDown(wrap);
            return Value::Int(1);
        });
    });

    add("cursorUp", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const bool wrap = args.size() > 0 ? (getInt(args[0]) != 0) : true;
            w.cursorUp(wrap);
            return Value::Int(1);
        });
    });

    add("cursorRight", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const bool wrap = args.size() > 0 ? (getInt(args[0]) != 0) : true;
            w.cursorRight(wrap);
            return Value::Int(1);
        });
    });

    add("cursorLeft", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const bool wrap = args.size() > 0 ? (getInt(args[0]) != 0) : true;
            w.cursorLeft(wrap);
            return Value::Int(1);
        });
    });

    add("isCursorMovable", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.isCursorMovable() ? 1 : 0); });
    });

    add("isHandled", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const std::string symbol = args.size() > 0 ? getString(args[0]) : "";
            return Value::Int(w.isHandled(symbol) ? 1 : 0);
        });
    });

    add("processOk", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) {
            w.processOk();
            return Value::Int(1);
        });
    });

    add("processCancel", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) {
            w.processCancel();
            return Value::Int(1);
        });
    });

    // Legacy / helper bindings
    add("setIndex", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const int32_t idx = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            w.setIndex(idx);
            return Value::Int(1);
        });
    });

    add("setMaxItems", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const int32_t count = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            w.setMaxItems(count);
            return Value::Int(1);
        });
    });

    add("setMaxCols", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const int32_t cols = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 1);
            w.setMaxCols(cols);
            return Value::Int(1);
        });
    });

    add("getItemWidth", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getItemWidth()); });
    });

    add("cursorPagedown", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) {
            w.cursorPagedown();
            return Value::Int(1);
        });
    });

    add("cursorPageup", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) {
            w.cursorPageup();
            return Value::Int(1);
        });
    });

    add("getRow", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getRow()); });
    });

    add("getCol", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getCol()); });
    });

    add("setTopRow", [&](const std::vector<Value>& args) -> Value {
        return dispatchSelectable([&](Window_Selectable& w) {
            const int32_t row = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            w.setTopRow(row);
            return Value::Int(1);
        });
    });

    add("getTopRow", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) { return Value::Int(w.getTopRow()); });
    });

    add("processPagedown", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) {
            w.processPagedown();
            return Value::Int(1);
        });
    });

    add("processPageup", [&](const std::vector<Value>&) -> Value {
        return dispatchSelectable([](Window_Selectable& w) {
            w.processPageup();
            return Value::Int(1);
        });
    });

    ctx.registerObject("Window_Selectable", methods);
}

// ============================================================================
// Window_Command Implementation
// ============================================================================

Window_Command::Window_Command(const CreateParams& params) : Window_Selectable(params), commands_(params.commands) {
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

void Window_Command::addCommand(const std::string& name, const std::string& symbol, bool enabled, int32_t ext) {
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

bool Window_Command::isCommandEnabled(int32_t index) const {
    if (index < 0 || index >= static_cast<int32_t>(commands_.size())) {
        return false;
    }
    return commands_[index].enabled;
}

bool Window_Command::isCurrentItemEnabled() const {
    return isCommandEnabled(getIndex());
}

std::string Window_Command::getCurrentSymbol() const {
    if (getIndex() < 0 || getIndex() >= static_cast<int32_t>(commands_.size())) {
        return "";
    }
    return commands_[getIndex()].symbol;
}

int32_t Window_Command::findSymbol(const std::string& symbol) const {
    for (int32_t i = 0; i < static_cast<int32_t>(commands_.size()); ++i) {
        if (commands_[i].symbol == symbol) {
            return i;
        }
    }
    return -1;
}

int32_t Window_Command::findExt(int32_t ext) const {
    for (int32_t i = 0; i < static_cast<int32_t>(commands_.size()); ++i) {
        if (commands_[i].ext == ext) {
            return i;
        }
    }
    return -1;
}

void Window_Command::callOkHandler() {
    if (!onCommand_ || !isCurrentItemEnabled()) {
        return;
    }
    onCommand_(getCurrentSymbol());
}

void Window_Command::drawItem(int32_t index) {
    if (index < 0 || index >= static_cast<int32_t>(commands_.size())) {
        return;
    }

    const auto& cmd = commands_[index];
    const int32_t maxCols = std::max(1, getMaxCols());
    Rect itemRect{(index % maxCols) * getItemWidth(), (index / maxCols) * getItemHeight(), getItemWidth(),
                  getItemHeight()};

    const Color color = cmd.enabled ? normalColor() : dimColor();
    changeTextColor(color);
    drawText(cmd.name, itemRect.x, itemRect.y, itemRect.width, "left");
    resetTextColor();
}

void Window_Command::drawAllItems() {
    for (int32_t i = 0; i < getCommandCount(); ++i) {
        drawItem(i);
    }
}

void Window_Command::selectSymbol(const std::string& symbol) {
    const int32_t index = findSymbol(symbol);
    if (index >= 0) {
        setIndex(index);
    }
}

void Window_Command::selectExt(int32_t ext) {
    const int32_t index = findExt(ext);
    if (index >= 0) {
        setIndex(index);
    }
}

int32_t Window_Command::getCurrentExt() const {
    return getCurrentCommand().ext;
}

void Window_Command::makeCommandList() {
    // Override in subclasses to populate commands
}

void Window_Command::processOk() {
    callOkHandler();
}

void Window_Command::registerAPI(QuickJSContext& ctx) {
    Window_Selectable::registerAPI(ctx);

    auto getInt = [](const Value& v, int64_t defaultVal = 0) -> int64_t {
        if (auto* p = std::get_if<int64_t>(&v.v))
            return *p;
        if (auto* p = std::get_if<double>(&v.v))
            return static_cast<int64_t>(*p);
        return defaultVal;
    };
    auto getString = [](const Value& v, const std::string& defaultVal = "") -> std::string {
        if (auto* p = std::get_if<std::string>(&v.v))
            return *p;
        return defaultVal;
    };
    auto getBool = [](const Value& v, bool defaultVal = false) -> bool {
        if (auto* p = std::get_if<bool>(&v.v))
            return *p;
        if (auto* p = std::get_if<int64_t>(&v.v))
            return *p != 0;
        return defaultVal;
    };

    auto dispatchCommand = [&](std::function<Value(Window_Command&)> fn) -> Value {
        if (!defaultInstance_)
            return Value::Nil();
        auto* cmd = dynamic_cast<Window_Command*>(defaultInstance_);
        if (!cmd)
            return Value::Nil();
        return fn(*cmd);
    };

    std::vector<QuickJSContext::MethodDef> methods;

    auto add = [&](const std::string& name, std::function<Value(const std::vector<Value>&)> fn) {
        QuickJSContext::MethodDef method;
        method.name = name;
        method.fn = fn;
        method.status = CompatStatus::FULL;
        methods.push_back(method);
    };

    add("addCommand", [&](const std::vector<Value>& args) -> Value {
        return dispatchCommand([&](Window_Command& w) {
            const std::string name = args.size() > 0 ? getString(args[0]) : "";
            const std::string symbol = args.size() > 1 ? getString(args[1]) : "";
            const bool enabled = args.size() > 2 ? getBool(args[2]) : true;
            const int32_t ext = static_cast<int32_t>(args.size() > 3 ? getInt(args[3]) : 0);
            w.addCommand(name, symbol, enabled, ext);
            return Value::Int(1);
        });
    });

    add("clearCommandList", [&](const std::vector<Value>&) -> Value {
        return dispatchCommand([](Window_Command& w) {
            w.clearCommands();
            return Value::Int(1);
        });
    });

    add("clearCommands", [&](const std::vector<Value>&) -> Value {
        return dispatchCommand([](Window_Command& w) {
            w.clearCommands();
            return Value::Int(1);
        });
    });

    add("selectSymbol", [&](const std::vector<Value>& args) -> Value {
        return dispatchCommand([&](Window_Command& w) {
            const std::string symbol = args.size() > 0 ? getString(args[0]) : "";
            w.selectSymbol(symbol);
            return Value::Int(1);
        });
    });

    add("findSymbol", [&](const std::vector<Value>& args) -> Value {
        return dispatchCommand([&](Window_Command& w) {
            const std::string symbol = args.size() > 0 ? getString(args[0]) : "";
            return Value::Int(w.findSymbol(symbol));
        });
    });

    add("ext", [&](const std::vector<Value>&) -> Value {
        return dispatchCommand([](Window_Command& w) { return Value::Int(w.getCurrentExt()); });
    });

    add("makeCommandList", [&](const std::vector<Value>&) -> Value {
        return dispatchCommand([](Window_Command& w) {
            w.makeCommandList();
            return Value::Int(1);
        });
    });

    // Helper bindings
    add("getCommandCount", [&](const std::vector<Value>&) -> Value {
        return dispatchCommand([](Window_Command& w) { return Value::Int(w.getCommandCount()); });
    });

    add("isCommandEnabled", [&](const std::vector<Value>& args) -> Value {
        return dispatchCommand([&](Window_Command& w) {
            const int32_t idx = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            return Value::Int(w.isCommandEnabled(idx) ? 1 : 0);
        });
    });

    add("isCurrentItemEnabled", [&](const std::vector<Value>&) -> Value {
        return dispatchCommand([](Window_Command& w) { return Value::Int(w.isCurrentItemEnabled() ? 1 : 0); });
    });

    add("getCurrentSymbol", [&](const std::vector<Value>&) -> Value {
        return dispatchCommand([](Window_Command& w) {
            urpg::Value v;
            v.v = w.getCurrentSymbol();
            return v;
        });
    });

    add("drawItem", [&](const std::vector<Value>& args) -> Value {
        return dispatchCommand([&](Window_Command& w) {
            const int32_t idx = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            w.drawItem(idx);
            return Value::Int(1);
        });
    });

    add("drawAllItems", [&](const std::vector<Value>&) -> Value {
        return dispatchCommand([](Window_Command& w) {
            w.drawAllItems();
            return Value::Int(1);
        });
    });

    add("selectExt", [&](const std::vector<Value>& args) -> Value {
        return dispatchCommand([&](Window_Command& w) {
            const int32_t ext = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            w.selectExt(ext);
            return Value::Int(1);
        });
    });

    add("findExt", [&](const std::vector<Value>& args) -> Value {
        return dispatchCommand([&](Window_Command& w) {
            const int32_t ext = static_cast<int32_t>(args.size() > 0 ? getInt(args[0]) : 0);
            return Value::Int(w.findExt(ext));
        });
    });

    add("callOkHandler", [&](const std::vector<Value>&) -> Value {
        return dispatchCommand([](Window_Command& w) {
            w.callOkHandler();
            return Value::Int(1);
        });
    });

    ctx.registerObject("Window_Command", methods);
}

// ============================================================================
// Window_Message Implementation
// ============================================================================

Window_Message::Window_Message(const CreateParams& params)
    : Window_Base(params), messageX_(params.messageX), messageY_(params.messageY), messageWidth_(params.messageWidth) {}

void Window_Message::setMessageRect(int32_t x, int32_t y, int32_t width) {
    messageX_ = x;
    messageY_ = y;
    messageWidth_ = std::max(0, width);
}

void Window_Message::setMessageText(std::string text) {
    messageText_ = std::move(text);
}

void Window_Message::setMessageAlignment(const std::string& align) {
    setTextAlignment(align);
}

void Window_Message::drawMessageBody() {
    clearTextDrawHistory();
    const Rect content = getContentRect();
    const int32_t width = messageWidth_ > 0 ? messageWidth_ : content.width;
    drawTextEx(messageText_, messageX_, messageY_, width);
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

uint32_t WindowCompatManager::createWindowMessage(const Window_Message::CreateParams& params) {
    uint32_t id = nextId_++;
    windows_[id] = std::make_unique<Window_Message>(params);
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
    for (auto& entry : windows_) {
        entry.second->update();
    }
    for (auto& entry : characterSprites_) {
        entry.second->update();
    }
    for (auto& entry : actorSprites_) {
        entry.second->update();
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

    for (const auto& method : Window_Base::getTrackedMethods()) {
        CompatReport entry;
        entry.className = "Window_Base";
        entry.methodName = method;
        entry.status = Window_Base::getMethodStatus(method);
        entry.deviation = Window_Base::getMethodDeviation(method);
        entry.callCount = Window_Base::getMethodCallCount(method);
        report.push_back(entry);
    }

    return report;
}

} // namespace compat
} // namespace urpg
