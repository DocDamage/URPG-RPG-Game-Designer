// WindowCompat - Core Surface Implementation
// Phase 2 - Compat Layer
//
// Implementation stubs for MZ Window API compatibility surface.

#include "window_compat.h"
#include "data_manager.h"
#include "engine/core/message/message_core.h"
#include "engine/core/render/render_layer.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <string_view>

namespace urpg {
namespace compat {

namespace {
namespace message = urpg::message;

constexpr int32_t kIconWidth = 32;
constexpr int32_t kIconSpacing = 4;
constexpr int32_t kFaceCellWidth = 144;
constexpr int32_t kFaceCellHeight = 144;
constexpr int32_t kFaceSheetCols = 4;
constexpr int32_t kFaceSheetRows = 2;
constexpr int32_t kFontStep = 12;
constexpr int32_t kFontSizeMin = 12;
constexpr int32_t kFontSizeMax = 96;

char32_t decodeUtf8Codepoint(const std::string& text, size_t& cursor) {
    if (cursor >= text.size()) {
        return U'\0';
    }

    const unsigned char lead = static_cast<unsigned char>(text[cursor]);
    if (lead < 0x80) {
        ++cursor;
        return static_cast<char32_t>(lead);
    }

    if ((lead >> 5) == 0x6 && cursor + 1 < text.size()) {
        const unsigned char b1 = static_cast<unsigned char>(text[cursor + 1]);
        if ((b1 & 0xC0) == 0x80) {
            cursor += 2;
            return static_cast<char32_t>(((lead & 0x1F) << 6) | (b1 & 0x3F));
        }
    } else if ((lead >> 4) == 0xE && cursor + 2 < text.size()) {
        const unsigned char b1 = static_cast<unsigned char>(text[cursor + 1]);
        const unsigned char b2 = static_cast<unsigned char>(text[cursor + 2]);
        if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80) {
            cursor += 3;
            return static_cast<char32_t>(((lead & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F));
        }
    } else if ((lead >> 3) == 0x1E && cursor + 3 < text.size()) {
        const unsigned char b1 = static_cast<unsigned char>(text[cursor + 1]);
        const unsigned char b2 = static_cast<unsigned char>(text[cursor + 2]);
        const unsigned char b3 = static_cast<unsigned char>(text[cursor + 3]);
        if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80 && (b3 & 0xC0) == 0x80) {
            cursor += 4;
            return static_cast<char32_t>(
                ((lead & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F)
            );
        }
    }

    ++cursor;
    return static_cast<char32_t>(lead);
}

int32_t rendererGlyphAdvance(char32_t cp, int32_t fontSize) {
    const double size = static_cast<double>(std::max(1, fontSize));
    if (cp == U'\t') {
        return static_cast<int32_t>(std::lround(size * 2.2));
    }
    if (cp == U' ') {
        return static_cast<int32_t>(std::lround(size * 0.35));
    }

    if (cp < 0x80) {
        const unsigned char ch = static_cast<unsigned char>(cp);
        if (std::isdigit(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.56));
        }
        if (std::ispunct(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.45));
        }
        if (std::isupper(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.62));
        }
        return static_cast<int32_t>(std::lround(size * 0.56));
    }

    // Treat CJK ranges as full-width glyphs.
    if ((cp >= 0x2E80 && cp <= 0x9FFF) || (cp >= 0xF900 && cp <= 0xFAFF)) {
        return static_cast<int32_t>(std::lround(size));
    }
    // Emoji and symbol-heavy planes tend to render wider.
    if (cp >= 0x1F000) {
        return static_cast<int32_t>(std::lround(size * 1.1));
    }
    // Fallback: proportional non-ASCII glyph.
    return static_cast<int32_t>(std::lround(size * 0.75));
}

int32_t measurePlainTextRenderer(const std::string& text, int32_t fontSize) {
    if (text.empty()) {
        return 0;
    }
    int32_t width = 0;
    size_t cursor = 0;
    while (cursor < text.size()) {
        const char32_t cp = decodeUtf8Codepoint(text, cursor);
        if (cp == U'\0' || cp == U'\r' || cp == U'\n') {
            continue;
        }
        width += rendererGlyphAdvance(cp, fontSize);
    }
    return width;
}

const ItemData* resolveAnyItemById(DataManager& data, int32_t itemId) {
    if (const ItemData* item = data.getItem(itemId)) {
        return item;
    }
    if (const ItemData* item = data.getWeapon(itemId)) {
        return item;
    }
    return data.getArmor(itemId);
}

std::string resolveItemLabel(const ItemData* item, int32_t itemId) {
    if (item != nullptr && !item->name.empty()) {
        return item->name;
    }
    return "Item " + std::to_string(std::max(0, itemId));
}

int32_t resolveItemIconIndex(const ItemData* item) {
    if (item != nullptr) {
        return std::max(0, item->iconIndex);
    }
    return 0;
}

int32_t resolveEffectDurationFrames(std::string_view effect) {
    if (effect == "whiten") {
        return 16;
    }
    if (effect == "blink") {
        return 20;
    }
    if (effect == "collapse") {
        return 32;
    }
    if (effect == "bossCollapse") {
        return 80;
    }
    if (effect == "instantCollapse") {
        return 1;
    }
    return 0;
}

int32_t resolveAnimationDurationFrames(int32_t animationId) {
    if (animationId <= 0) {
        return 0;
    }
    constexpr int32_t baseFrames = 24;
    constexpr int32_t stepFrames = 6;
    constexpr int32_t variationCount = 5;
    return baseFrames + ((animationId % variationCount) * stepFrames);
}

int32_t normalizeFaceIndex(int32_t faceIndex) {
    constexpr int32_t faceCells = kFaceSheetCols * kFaceSheetRows;
    if (faceCells <= 0) {
        return 0;
    }
    if (faceIndex < 0) {
        return 0;
    }
    return faceIndex % faceCells;
}

int32_t resolveActorFaceIndex(const ActorData* actor, int32_t actorId) {
    if (actor != nullptr) {
        return normalizeFaceIndex(actor->faceIndex);
    }
    // Deterministic fallback mapping when actor DB is unavailable.
    return normalizeFaceIndex(std::max(0, actorId - 1));
}

std::string resolveActorFaceName(const ActorData* actor, int32_t actorId) {
    if (actor != nullptr && !actor->faceName.empty()) {
        return actor->faceName;
    }
    return "ActorFace_" + std::to_string(std::max(0, actorId));
}

message::MessageAlignment parseMessageAlignment(const std::string& align) {
    std::string normalized;
    normalized.reserve(align.size());
    for (const char ch : align) {
        normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    if (normalized == "center") {
        return message::MessageAlignment::Center;
    }
    if (normalized == "right") {
        return message::MessageAlignment::Right;
    }
    return message::MessageAlignment::Left;
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

message::RichTextLayoutEngine buildLayoutEngineForWindow(const Window_Base& window,
                                                         int32_t maxWidth,
                                                         message::MessageAlignment alignment) {
    message::RichTextLayoutEngine layout;
    layout.setBaseFontSize(window.fontSize());
    layout.setLineHeight(window.lineHeight());
    layout.setMaxWidth(maxWidth);
    layout.setAlignment(alignment);
    layout.setVariableResolver([](int32_t id) -> int32_t {
        return DataManager::instance().getVariable(id);
    });
    layout.setActorNameResolver([](int32_t id) -> std::string {
        if (const auto* actor = DataManager::instance().getActor(id)) {
            if (!actor->name.empty()) {
                return actor->name;
            }
        }
        return "Actor " + std::to_string(std::max(0, id));
    });
    layout.setPartyMemberResolver([](int32_t index) -> int32_t {
        return DataManager::instance().getPartyMember(index);
    });
    return layout;
}

int32_t lineOffsetForFirstLine(const std::vector<message::RichTextToken>& tokens) {
    for (const auto& token : tokens) {
        if (token.type == message::RichTextTokenType::LineOffset) {
            return token.value;
        }
        if (token.type == message::RichTextTokenType::NewLine) {
            break;
        }
    }
    return 0;
}

} // namespace

// ============================================================================
// Window_Base Implementation
// ============================================================================

std::unordered_map<std::string, CompatStatus> Window_Base::methodStatus_;
std::unordered_map<std::string, std::string> Window_Base::methodDeviations_;
std::unordered_map<std::string, uint32_t> Window_Base::methodCallCounts_;
Window_Base* Window_Base::defaultInstance_ = nullptr;

// Initialize static method status maps
void Window_Base::initializeMethodStatus() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    const auto setStatus = [](const std::string& method,
                              CompatStatus status,
                              const std::string& deviation = "") {
        methodStatus_[method] = status;
        if (deviation.empty()) {
            methodDeviations_.erase(method);
        } else {
            methodDeviations_[method] = deviation;
        }
    };
    
    // FULL status methods
    setStatus("drawText", CompatStatus::FULL);
    setStatus("drawIcon", CompatStatus::PARTIAL,
              "Tracks icon draw intent, but icon-set bitmap rendering is still TODO.");
    setStatus("drawActorName", CompatStatus::PARTIAL,
              "Actor-name lookup and text draw are wired; full window chrome is not implemented.");
    setStatus("drawActorLevel", CompatStatus::PARTIAL,
              "Actor-level lookup and text draw are wired; full window chrome is not implemented.");
    setStatus("drawGauge", CompatStatus::PARTIAL,
              "Background and fill RectCommands are submitted; gradient fill is still TODO.");
    setStatus("drawCharacter", CompatStatus::PARTIAL,
              "SpriteCommand is submitted; character-sheet index mapping is still TODO.");
    setStatus("lineHeight", CompatStatus::STUB);
    setStatus("changeTextColor", CompatStatus::FULL);
    setStatus("resetTextColor", CompatStatus::FULL);
    setStatus("textColor", CompatStatus::FULL);
    setStatus("systemColor", CompatStatus::PARTIAL);
    setStatus("resetFontSettings", CompatStatus::FULL);
    setStatus("fontFace", CompatStatus::FULL);
    setStatus("fontSize", CompatStatus::FULL);
    setStatus("setFontFace", CompatStatus::FULL);
    setStatus("setFontSize", CompatStatus::FULL);
    setStatus("setTextAlignment", CompatStatus::FULL);
    setStatus("textAlignment", CompatStatus::FULL);
    setStatus("contents", CompatStatus::STUB,
              "Returns a placeholder handle; backing bitmap lifecycle is not implemented.");
    setStatus("createContents", CompatStatus::STUB,
              "Content bitmap allocation is still TODO.");
    setStatus("destroyContents", CompatStatus::STUB,
              "Content bitmap release is still TODO.");
    setStatus("open", CompatStatus::FULL);
    setStatus("close", CompatStatus::FULL);
    setStatus("show", CompatStatus::FULL);
    setStatus("hide", CompatStatus::FULL);
    setStatus("update", CompatStatus::STUB);
    setStatus("getContentRect", CompatStatus::FULL);
    
    setStatus("drawActorFace", CompatStatus::PARTIAL);
    
    setStatus("drawActorHp", CompatStatus::STUB);
    setStatus("drawActorMp", CompatStatus::STUB);
    setStatus("drawActorTp", CompatStatus::STUB);
    
    setStatus("drawTextEx", CompatStatus::PARTIAL);
    
    setStatus("drawItemName", CompatStatus::FULL);
    
    setStatus("textWidth", CompatStatus::FULL);
    
    setStatus("textSize", CompatStatus::FULL);

    for (const auto& [methodName, status] : methodStatus_) {
        (void)status;
        methodCallCounts_[methodName] = 0;
    }
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

    auto textCmd = std::make_shared<urpg::TextCommand>();
    textCmd->text = text;
    textCmd->fontFace = fontFace_;
    textCmd->fontSize = fontSize_;
    textCmd->maxWidth = safeMaxWidth;
    textCmd->x = static_cast<float>(rect_.x + padding_ + lastTextDraw_->resolvedX);
    textCmd->y = static_cast<float>(rect_.y + padding_ + lastTextDraw_->resolvedY);
    textCmd->zOrder = 100;
    textCmd->r = textColor_.r;
    textCmd->g = textColor_.g;
    textCmd->b = textColor_.b;
    textCmd->a = textColor_.a;
    urpg::RenderLayer::getInstance().submit(textCmd);

    assert(!text.empty() || safeMaxWidth >= 0);
}

void Window_Base::drawIcon(int32_t iconIndex, int32_t x, int32_t y) {
    recordMethodCall("drawIcon");
    (void)x;
    (void)y;
    // TODO: Draw icon from icon set
    // Icon is 32x32 in MZ default
    assert(iconIndex >= 0);
}

void Window_Base::drawActorFace(int32_t actorId, int32_t x, int32_t y, 
                                 int32_t width, int32_t height) {
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

    // Placeholder renderer bridge: route resolved face cell id through drawIcon telemetry.
    drawIcon(faceIndex, dx, dy);
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
    const int32_t gaugeWidth = std::max(0, width);
    const int32_t gaugeY = y + std::max(0, lineHeight() - 12);
    drawGauge(x, gaugeY, std::max(1, gaugeWidth), 1.0, systemColor(16), systemColor(20));
    drawText("HP", x, y, std::min(gaugeWidth, 40));
}

void Window_Base::drawActorMp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawActorMp");
    assert(actorId >= 0);
    const int32_t gaugeWidth = std::max(0, width);
    const int32_t gaugeY = y + std::max(0, lineHeight() - 12);
    drawGauge(x, gaugeY, std::max(1, gaugeWidth), 1.0, systemColor(17), systemColor(5));
    drawText("MP", x, y, std::min(gaugeWidth, 40));
}

void Window_Base::drawActorTp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawActorTp");
    assert(actorId >= 0);
    const int32_t gaugeWidth = std::max(0, width);
    const int32_t gaugeY = y + std::max(0, lineHeight() - 12);
    drawGauge(x, gaugeY, std::max(1, gaugeWidth), 1.0, systemColor(18), systemColor(6));
    drawText("TP", x, y, std::min(gaugeWidth, 40));
}

void Window_Base::drawGauge(int32_t x, int32_t y, int32_t width,
                             double rate, const Color& color1, const Color& color2) {
    recordMethodCall("drawGauge");
    assert(width > 0);
    assert(rate >= 0.0 && rate <= 1.0);

    constexpr int32_t kGaugeHeight = 12;
    const int32_t fillWidth = static_cast<int32_t>(static_cast<double>(width) * std::clamp(rate, 0.0, 1.0));
    const float baseX = static_cast<float>(rect_.x + padding_ + x);
    const float baseY = static_cast<float>(rect_.y + padding_ + y);

    // Background rect
    auto bgCmd = std::make_shared<urpg::RectCommand>();
    bgCmd->x = baseX;
    bgCmd->y = baseY;
    bgCmd->w = static_cast<float>(width);
    bgCmd->h = static_cast<float>(kGaugeHeight);
    bgCmd->r = 0.2f;
    bgCmd->g = 0.2f;
    bgCmd->b = 0.2f;
    bgCmd->a = 1.0f;
    bgCmd->zOrder = 100;
    urpg::RenderLayer::getInstance().submit(bgCmd);

    // Fill rect (uses color1; gradient to color2 is TODO)
    auto fillCmd = std::make_shared<urpg::RectCommand>();
    fillCmd->x = baseX;
    fillCmd->y = baseY;
    fillCmd->w = static_cast<float>(fillWidth);
    fillCmd->h = static_cast<float>(kGaugeHeight);
    fillCmd->r = color1.r / 255.0f;
    fillCmd->g = color1.g / 255.0f;
    fillCmd->b = color1.b / 255.0f;
    fillCmd->a = color1.a / 255.0f;
    fillCmd->zOrder = 101;
    urpg::RenderLayer::getInstance().submit(fillCmd);

    (void)color2; // Gradient target reserved for future implementation
}

void Window_Base::drawCharacter(const std::string& characterName, 
                                 int32_t index, int32_t x, int32_t y) {
    recordMethodCall("drawCharacter");
    assert(!characterName.empty());
    assert(index >= 0);

    auto spriteCmd = std::make_shared<urpg::SpriteCommand>();
    spriteCmd->textureId = characterName;
    spriteCmd->x = static_cast<float>(rect_.x + padding_ + x);
    spriteCmd->y = static_cast<float>(rect_.y + padding_ + y);
    spriteCmd->zOrder = 100;
    // index: stored in srcX/srcY as a proxy until SpriteCommand gains an index field
    spriteCmd->srcX = index * 32;
    spriteCmd->srcY = 0;
    spriteCmd->width = 32;
    spriteCmd->height = 32;
    urpg::RenderLayer::getInstance().submit(spriteCmd);
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
    const int32_t textWidth =
        safeWidth > 0 ? std::max(0, safeWidth - (kIconWidth + kIconSpacing)) : 0;
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
    return Rect{
        rect_.x + padding_,
        rect_.y + padding_,
        innerWidth,
        innerHeight
    };
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
    message::RichTextLayoutEngine layout =
        buildLayoutEngineForWindow(*this, 0, message::MessageAlignment::Left);
    return layout.textWidth(text);
}

Rect Window_Base::textSize(const std::string& text) const {
    methodCallCounts_["textSize"]++;
    message::RichTextLayoutEngine layout =
        buildLayoutEngineForWindow(*this, 0, message::MessageAlignment::Left);
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

BitmapHandle Window_Base::contents() const {
    methodCallCounts_["contents"]++;
    return contents_;
}

void Window_Base::createContents() {
    recordMethodCall("createContents");
    // TODO: Create actual bitmap
    // contents_ = BitmapManager::create(rect_.width - padding_ * 2, rect_.height - padding_ * 2);
    contents_ = 1;  // Placeholder handle
}

void Window_Base::destroyContents() {
    recordMethodCall("destroyContents");
    contents_ = INVALID_BITMAP;
}

void Window_Base::recordMethodCall(const std::string& methodName) {
    initializeMethodStatus();  // Ensure initialized
    auto it = methodCallCounts_.find(methodName);
    if (it != methodCallCounts_.end()) {
        ++it->second;
    }
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

uint32_t Window_Base::getMethodCallCount(const std::string& methodName) {
    initializeMethodStatus();  // Ensure initialized
    auto it = methodCallCounts_.find(methodName);
    return it != methodCallCounts_.end() ? it->second : 0;
}

std::vector<std::string> Window_Base::getTrackedMethods() {
    initializeMethodStatus();  // Ensure initialized
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
    initializeMethodStatus();  // Ensure initialized

    auto getInt = [](const Value& v, int64_t defaultVal = 0) -> int64_t {
        if (auto* p = std::get_if<int64_t>(&v.v)) return *p;
        if (auto* p = std::get_if<double>(&v.v)) return static_cast<int64_t>(*p);
        return defaultVal;
    };
    auto getDouble = [](const Value& v, double defaultVal = 0.0) -> double {
        if (auto* p = std::get_if<double>(&v.v)) return *p;
        if (auto* p = std::get_if<int64_t>(&v.v)) return static_cast<double>(*p);
        return defaultVal;
    };
    auto getString = [](const Value& v, const std::string& defaultVal = "") -> std::string {
        if (auto* p = std::get_if<std::string>(&v.v)) return *p;
        return defaultVal;
    };

    auto dispatch = [](std::function<Value(Window_Base&)> fn) -> Value {
        if (!defaultInstance_) return Value::Nil();
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
            // Color parsing: expect hex integers or object with r/g/b/a
            Color color1 = Color{255, 255, 255, 255};
            Color color2 = Color{255, 255, 255, 255};
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
        return dispatch([&](Window_Base& w) {
            return Value::Int(w.lineHeight());
        });
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
                } else {
                    // TODO: parse color object
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
        return dispatch([&](Window_Base& w) {
            return Value::Int(w.fontSize());
        });
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
        return dispatch([&](Window_Base& w) {
            return Value::Int(static_cast<int64_t>(w.contents()));
        });
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
    : Window_Base(params)
    , maxCols_(std::max(1, params.maxCols))
    , itemHeight_(std::max(1, params.itemHeight))
    , numVisibleRows_(std::max(1, params.numVisibleRows))
{
}

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
    const int32_t spacing = 8;  // MZ default
    const int32_t totalSpacing = spacing * (safeMaxCols - 1);
    const int32_t availableWidth = std::max(0, getContentRect().width - totalSpacing);
    return availableWidth / safeMaxCols;
}

void Window_Selectable::cursorDown(bool wrap) {
    if (maxItems_ <= 0) return;
    
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
    if (maxItems_ <= 0) return;
    
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
    if (safeMaxCols < 2) return;
    
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
    if (safeMaxCols < 2) return;
    
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
    if (maxItems_ <= 0) return;
    
    const int32_t safeMaxCols = std::max(1, maxCols_);
    int32_t newIndex = index_ + numVisibleRows_ * safeMaxCols;
    if (newIndex < maxItems_) {
        setIndex(newIndex);
    } else {
        setIndex(maxItems_ - 1);
    }
}

void Window_Selectable::cursorPageup() {
    if (maxItems_ <= 0) return;
    
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
    if (maxItems_ <= 0) return 0;
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
    static const std::vector<std::string> methodsToExpose = {
        "setIndex", "setMaxItems", "setMaxCols", "getItemWidth",
        "cursorDown", "cursorUp", "cursorLeft", "cursorRight",
        "cursorPagedown", "cursorPageup", "getRow", "getCol",
        "setTopRow", "getTopRow", "processPagedown", "processPageup"
    };
    methods.reserve(methodsToExpose.size());
    for (const auto& methodName : methodsToExpose) {
        methods.push_back({methodName, [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB});
    }

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
    (void)cmd;
    Rect itemRect{
        0,  // Will be calculated based on row/col
        (index / getMaxCols()) * getItemHeight(),
        getItemWidth(),
        getItemHeight()
    };
    (void)itemRect;
    
    // TODO: Draw text with appropriate color based on enabled state
    // Color: normalColor() if enabled, dimColor() if disabled
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

void Window_Command::registerAPI(QuickJSContext& ctx) {
    Window_Selectable::registerAPI(ctx);
    
    static const std::vector<QuickJSContext::MethodDef> methods = {
        {"addCommand", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"clearCommands", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"getCommandCount", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"isCommandEnabled", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"isCurrentItemEnabled", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"getCurrentSymbol", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"drawItem", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"drawAllItems", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"selectSymbol", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"selectExt", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"findSymbol", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"findExt", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
        {"callOkHandler", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::STUB},
    };

    ctx.registerObject("Window_Command", methods);
}

// ============================================================================
// Window_Message Implementation
// ============================================================================

Window_Message::Window_Message(const CreateParams& params)
    : Window_Base(params)
    , messageX_(params.messageX)
    , messageY_(params.messageY)
    , messageWidth_(params.messageWidth) {
}

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
    }, CompatStatus::STUB});
    
    methods.push_back({"setY", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"setDirection", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"setPattern", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"setVisible", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"setBlendMode", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"setOpacity", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"setScale", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
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
    animationId_ = animationId;
    animationFramesRemaining_ = resolveAnimationDurationFrames(animationId);
    animationPlaying_ = animationFramesRemaining_ > 0;
    if (!animationPlaying_) {
        animationId_ = 0;
    }
}

void Sprite_Actor::startEffect(const std::string& effect) {
    currentEffect_ = effect;
    effectDurationFrames_ = resolveEffectDurationFrames(effect);
    effecting_ = effectDurationFrames_ > 0;

    if (effect == "instantCollapse" && effecting_) {
        opacity_ = 0;
    }
    if (!effecting_) {
        currentEffect_.clear();
    }
}

void Sprite_Actor::update() {
    if (animationPlaying_) {
        if (animationFramesRemaining_ > 0) {
            --animationFramesRemaining_;
        }
        if (animationFramesRemaining_ <= 0) {
            animationPlaying_ = false;
            animationFramesRemaining_ = 0;
            animationId_ = 0;
        }
    }

    if (!effecting_) {
        return;
    }

    if (currentEffect_ == "collapse") {
        opacity_ = std::max(0, opacity_ - 8);
    } else if (currentEffect_ == "bossCollapse") {
        opacity_ = std::max(0, opacity_ - 4);
    } else if (currentEffect_ == "instantCollapse") {
        opacity_ = 0;
    }

    if (effectDurationFrames_ > 0) {
        --effectDurationFrames_;
    }
    if (effectDurationFrames_ <= 0) {
        effecting_ = false;
        effectDurationFrames_ = 0;
        currentEffect_.clear();
    }
}

void Sprite_Actor::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"setMotion", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"startMotion", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"startAnimation", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});

    methods.push_back({"isAnimationPlaying", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"startEffect", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"setVisible", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"setBlendMode", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
    methods.push_back({"setOpacity", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::STUB});
    
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
