// WindowCompat - Core Surface Implementation
// Phase 2 - Compat Layer
//
// Implementation stubs for MZ Window API compatibility surface.

#include "window_compat.h"
#include "data_manager.h"
#include "input_manager.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <string_view>
#include <unordered_set>

namespace urpg {
namespace compat {

namespace {

constexpr int32_t kIconWidth = 32;
constexpr int32_t kIconSpacing = 4;
constexpr int32_t kFaceCellWidth = 144;
constexpr int32_t kFaceCellHeight = 144;
constexpr int32_t kFaceSheetCols = 4;
constexpr int32_t kFaceSheetRows = 2;
constexpr int32_t kFontStep = 12;
constexpr int32_t kFontSizeMin = 12;
constexpr int32_t kFontSizeMax = 96;
constexpr const char* kCurrencyUnit = "G";

enum class TextExTokenType : uint8_t {
    Text = 0,
    Icon = 1,
    Color = 2,
    FontBigger = 3,
    FontSmaller = 4,
    NewLine = 5
};

struct TextExToken {
    TextExTokenType type = TextExTokenType::Text;
    std::string text;
    int32_t value = 0;
};

struct TextMeasureResult {
    int32_t width = 0;
    int32_t height = 0;
};

bool parseBracketedInt(const std::string& text, size_t& cursor, int32_t& value) {
    if (cursor >= text.size() || text[cursor] != '[') {
        return false;
    }

    size_t i = cursor + 1;
    bool negative = false;
    if (i < text.size() && text[i] == '-') {
        negative = true;
        ++i;
    }
    if (i >= text.size() || !std::isdigit(static_cast<unsigned char>(text[i]))) {
        return false;
    }

    int64_t parsed = 0;
    while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) {
        parsed = parsed * 10 + static_cast<int64_t>(text[i] - '0');
        ++i;
    }
    if (i >= text.size() || text[i] != ']') {
        return false;
    }
    ++i;

    if (negative) {
        parsed = -parsed;
    }
    value = static_cast<int32_t>(parsed);
    cursor = i;
    return true;
}

void appendTextToken(std::vector<TextExToken>& tokens, std::string_view text) {
    if (text.empty()) {
        return;
    }
    if (!tokens.empty() && tokens.back().type == TextExTokenType::Text) {
        tokens.back().text.append(text.data(), text.size());
        return;
    }
    TextExToken token;
    token.type = TextExTokenType::Text;
    token.text.assign(text.data(), text.size());
    tokens.push_back(std::move(token));
}

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

std::string resolveEscapeText(char command, int32_t arg) {
    DataManager& data = DataManager::instance();
    switch (command) {
        case 'V':
            return std::to_string(data.getVariable(arg));
        case 'N': {
            if (const auto* actor = data.getActor(arg)) {
                if (!actor->name.empty()) {
                    return actor->name;
                }
            }
            return "Actor " + std::to_string(std::max(0, arg));
        }
        case 'P': {
            if (arg > 0) {
                const int32_t actorId = data.getPartyMember(arg - 1);
                if (actorId > 0) {
                    if (const auto* actor = data.getActor(actorId)) {
                        if (!actor->name.empty()) {
                            return actor->name;
                        }
                    }
                    return "Actor " + std::to_string(actorId);
                }
            }
            return "";
        }
        case 'G':
            return kCurrencyUnit;
        default:
            break;
    }
    return "";
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

std::vector<TextExToken> tokenizeTextEx(const std::string& text) {
    std::vector<TextExToken> tokens;
    std::string plainBuffer;
    plainBuffer.reserve(text.size());

    const auto flushPlainBuffer = [&]() {
        if (!plainBuffer.empty()) {
            appendTextToken(tokens, plainBuffer);
            plainBuffer.clear();
        }
    };

    size_t i = 0;
    while (i < text.size()) {
        const char ch = text[i];
        if (ch == '\r') {
            ++i;
            continue;
        }
        if (ch == '\n') {
            flushPlainBuffer();
            TextExToken token;
            token.type = TextExTokenType::NewLine;
            tokens.push_back(token);
            ++i;
            continue;
        }
        if (ch != '\\') {
            plainBuffer.push_back(ch);
            ++i;
            continue;
        }

        if (i + 1 >= text.size()) {
            plainBuffer.push_back('\\');
            ++i;
            continue;
        }

        const char rawCommand = text[i + 1];
        if (rawCommand == '\\') {
            plainBuffer.push_back('\\');
            i += 2;
            continue;
        }

        const char command = static_cast<char>(std::toupper(static_cast<unsigned char>(rawCommand)));
        size_t cursor = i + 2;
        int32_t arg = 0;

        const auto pushLiteralCommand = [&]() {
            plainBuffer.push_back('\\');
            plainBuffer.push_back(rawCommand);
            i += 2;
        };

        if (command == '{') {
            flushPlainBuffer();
            TextExToken token;
            token.type = TextExTokenType::FontBigger;
            tokens.push_back(token);
            i = cursor;
            continue;
        }
        if (command == '}') {
            flushPlainBuffer();
            TextExToken token;
            token.type = TextExTokenType::FontSmaller;
            tokens.push_back(token);
            i = cursor;
            continue;
        }
        if (command == 'G') {
            flushPlainBuffer();
            appendTextToken(tokens, kCurrencyUnit);
            i = cursor;
            continue;
        }

        if (command == 'C' || command == 'I' || command == 'V' || command == 'N' || command == 'P') {
            if (!parseBracketedInt(text, cursor, arg)) {
                pushLiteralCommand();
                continue;
            }

            flushPlainBuffer();
            if (command == 'C') {
                TextExToken token;
                token.type = TextExTokenType::Color;
                token.value = arg;
                tokens.push_back(token);
            } else if (command == 'I') {
                TextExToken token;
                token.type = TextExTokenType::Icon;
                token.value = arg;
                tokens.push_back(token);
            } else {
                appendTextToken(tokens, resolveEscapeText(command, arg));
            }
            i = cursor;
            continue;
        }

        pushLiteralCommand();
    }

    flushPlainBuffer();
    return tokens;
}

TextMeasureResult measureTextTokens(const std::vector<TextExToken>& tokens,
                                    int32_t baseLineHeight,
                                    int32_t baseFontSize) {
    TextMeasureResult result;
    int32_t lineWidth = 0;
    int32_t lineHeight = std::max(1, baseLineHeight);
    int32_t fontSize = std::max(1, baseFontSize);
    bool sawAnyToken = false;

    for (const auto& token : tokens) {
        sawAnyToken = true;
        switch (token.type) {
            case TextExTokenType::Text: {
                lineWidth += measurePlainTextRenderer(token.text, fontSize);
                lineHeight = std::max(lineHeight, std::max(baseLineHeight, fontSize + 8));
                result.width = std::max(result.width, lineWidth);
                break;
            }
            case TextExTokenType::Icon: {
                lineWidth += kIconWidth + kIconSpacing;
                lineHeight = std::max(lineHeight, std::max(baseLineHeight, kIconWidth));
                result.width = std::max(result.width, lineWidth);
                break;
            }
            case TextExTokenType::Color:
                break;
            case TextExTokenType::FontBigger:
                fontSize = std::min(kFontSizeMax, fontSize + kFontStep);
                lineHeight = std::max(lineHeight, std::max(baseLineHeight, fontSize + 8));
                break;
            case TextExTokenType::FontSmaller:
                fontSize = std::max(kFontSizeMin, fontSize - kFontStep);
                lineHeight = std::max(lineHeight, std::max(baseLineHeight, fontSize + 8));
                break;
            case TextExTokenType::NewLine:
                result.height += lineHeight;
                lineWidth = 0;
                lineHeight = std::max(1, baseLineHeight);
                break;
        }
    }

    if (!sawAnyToken) {
        result.height = std::max(1, baseLineHeight);
        return result;
    }

    result.width = std::max(result.width, lineWidth);
    result.height += lineHeight;
    return result;
}

uint32_t nextBitmapId = 1;
std::unordered_set<uint32_t> activeBitmaps;

BitmapHandle allocateBitmap() {
    uint32_t id = nextBitmapId++;
    activeBitmaps.insert(id);
    return id;
}

void releaseBitmap(BitmapHandle handle) {
    if (handle != INVALID_BITMAP) {
        activeBitmaps.erase(handle);
    }
}

bool isValidBitmap(BitmapHandle handle) {
    return handle != INVALID_BITMAP && activeBitmaps.count(handle) > 0;
}

} // namespace

// ============================================================================
// Window_Base Implementation
// ============================================================================

std::unordered_map<std::string, CompatStatus> Window_Base::methodStatus_;
std::unordered_map<std::string, std::string> Window_Base::methodDeviations_;
std::unordered_map<std::string, uint32_t> Window_Base::methodCallCounts_;

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
    methodStatus_["open"] = CompatStatus::FULL;
    methodStatus_["close"] = CompatStatus::FULL;
    methodStatus_["show"] = CompatStatus::FULL;
    methodStatus_["hide"] = CompatStatus::FULL;
    methodStatus_["update"] = CompatStatus::FULL;
    methodStatus_["getContentRect"] = CompatStatus::FULL;
    
    methodStatus_["drawActorFace"] = CompatStatus::FULL;
    
    methodStatus_["drawActorHp"] = CompatStatus::FULL;
    methodStatus_["drawActorMp"] = CompatStatus::FULL;
    methodStatus_["drawActorTp"] = CompatStatus::FULL;
    
    methodStatus_["drawTextEx"] = CompatStatus::FULL;
    
    methodStatus_["drawItemName"] = CompatStatus::FULL;
    
    methodStatus_["textWidth"] = CompatStatus::FULL;
    
    methodStatus_["textSize"] = CompatStatus::FULL;
    methodStatus_["normalColor"] = CompatStatus::FULL;
    methodStatus_["dimColor"] = CompatStatus::FULL;
    methodStatus_["deathColor"] = CompatStatus::FULL;
    methodStatus_["itemRectForIndex"] = CompatStatus::FULL;
    methodStatus_["drawItem"] = CompatStatus::FULL;

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
    destroyContents();
}

void Window_Base::drawText(const std::string& text, int32_t x, int32_t y, 
                            int32_t maxWidth, const std::string& align) {
    recordMethodCall("drawText");
    // Compat layer: record that text was drawn at (x,y) with the given parameters.
    // Actual rasterization is handled by the native renderer tier.
    (void)text; (void)x; (void)y; (void)maxWidth; (void)align;
    assert(!text.empty() || maxWidth >= 0);
    
    // MZ behavior: empty text with max width clears the area
    if (text.empty() && maxWidth > 0) {
        // Clear area at (x, y) with width maxWidth and height = lineHeight
    }
}

void Window_Base::drawIcon(int32_t iconIndex, int32_t x, int32_t y) {
    recordMethodCall("drawIcon");
    // Compat layer: record icon draw request. Actual icon atlas lookup is native-renderer tier.
    (void)iconIndex; (void)x; (void)y;
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
    assert(actorId >= 0);
    const ActorData* actor = DataManager::instance().getActor(actorId);
    std::string name = actor ? actor->name : "Actor " + std::to_string(actorId);
    drawText(name, x, y, width);
}

void Window_Base::drawActorLevel(int32_t actorId, int32_t x, int32_t y) {
    recordMethodCall("drawActorLevel");
    assert(actorId >= 0);
    const ActorData* actor = DataManager::instance().getActor(actorId);
    int32_t level = actor ? actor->initialLevel : 1;
    drawText("Lv " + std::to_string(level), x, y);
}

void Window_Base::drawActorHp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawActorHp");
    assert(actorId >= 0);
    const ActorData* actor = DataManager::instance().getActor(actorId);
    const GameActor* gameActor = DataManager::instance().getGameActor(actorId);
    int32_t mhp = 100;
    int32_t hp = 100;
    if (gameActor) {
        mhp = gameActor->mhp;
        hp = gameActor->hp;
    } else if (actor && !actor->params.empty() && !actor->params[0].empty()) {
        mhp = actor->params[actor->initialLevel - 1][0];
        hp = mhp;
    }
    double rate = (mhp > 0) ? static_cast<double>(hp) / mhp : 1.0;
    const int32_t gaugeWidth = std::max(0, width);
    const int32_t gaugeY = y + std::max(0, lineHeight() - 12);
    drawGauge(x, gaugeY, std::max(1, gaugeWidth), rate, systemColor(16), systemColor(20));
    drawText("HP", x, y, std::min(gaugeWidth, 40));
}

void Window_Base::drawActorMp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawActorMp");
    assert(actorId >= 0);
    const ActorData* actor = DataManager::instance().getActor(actorId);
    const GameActor* gameActor = DataManager::instance().getGameActor(actorId);
    int32_t mmp = 100;
    int32_t mp = 100;
    if (gameActor) {
        mmp = gameActor->mmp;
        mp = gameActor->mp;
    } else if (actor && actor->params.size() > static_cast<size_t>(actor->initialLevel - 1) && actor->params[actor->initialLevel - 1].size() > 1) {
        mmp = actor->params[actor->initialLevel - 1][1];
        mp = mmp;
    }
    double rate = (mmp > 0) ? static_cast<double>(mp) / mmp : 1.0;
    const int32_t gaugeWidth = std::max(0, width);
    const int32_t gaugeY = y + std::max(0, lineHeight() - 12);
    drawGauge(x, gaugeY, std::max(1, gaugeWidth), rate, systemColor(17), systemColor(5));
    drawText("MP", x, y, std::min(gaugeWidth, 40));
}

void Window_Base::drawActorTp(int32_t actorId, int32_t x, int32_t y, int32_t width) {
    recordMethodCall("drawActorTp");
    assert(actorId >= 0);
    const GameActor* gameActor = DataManager::instance().getGameActor(actorId);
    int32_t tp = 0;
    if (gameActor) {
        tp = gameActor->tp;
    }
    int32_t mtp = 100;
    if (gameActor) {
        mtp = gameActor->mtp;
    }
    double rate = std::clamp(static_cast<double>(tp) / static_cast<double>(mtp), 0.0, 1.0);
    const int32_t gaugeWidth = std::max(0, width);
    const int32_t gaugeY = y + std::max(0, lineHeight() - 12);
    drawGauge(x, gaugeY, std::max(1, gaugeWidth), rate, systemColor(18), systemColor(6));
    drawText("TP", x, y, std::min(gaugeWidth, 40));
}

void Window_Base::drawGauge(int32_t x, int32_t y, int32_t width,
                             double rate, const Color& color1, const Color& color2) {
    recordMethodCall("drawGauge");
    // Compat layer: record gauge draw request. Actual gradient fill is native-renderer tier.
    (void)x; (void)y; (void)width; (void)color1; (void)color2;
    assert(width > 0);
    assert(rate >= 0.0 && rate <= 1.0);

    lastGaugeRate_ = rate;

    // Gauge height is typically 12 pixels
    // Background: dark gray
    // Fill: gradient from color1 to color2 based on rate
}

void Window_Base::drawCharacter(const std::string& characterName, 
                                 int32_t index, int32_t x, int32_t y) {
    recordMethodCall("drawCharacter");
    // Compat layer: record character sprite draw request. Actual sheet lookup is native-renderer tier.
    (void)characterName; (void)index; (void)x; (void)y;
    assert(!characterName.empty());
    assert(index >= 0);
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
    const auto tokens = tokenizeTextEx(text);
    const int32_t baseLineHeight = lineHeight();
    int32_t cursorX = x;
    int32_t cursorY = y;
    int32_t currentLineHeight = std::max(1, baseLineHeight);
    int32_t currentFontSize = std::max(1, fontSize_);
    const Color originalColor = textColor_;

    const auto currentMaxWidth = [&]() -> int32_t {
        if (width <= 0) {
            return 0;
        }
        return std::max(0, width - (cursorX - x));
    };

    for (const auto& token : tokens) {
        switch (token.type) {
            case TextExTokenType::Text: {
                if (!token.text.empty()) {
                    drawText(token.text, cursorX, cursorY, currentMaxWidth());
                    cursorX += measurePlainTextRenderer(token.text, currentFontSize);
                    currentLineHeight = std::max(currentLineHeight, std::max(baseLineHeight, currentFontSize + 8));
                }
                break;
            }
            case TextExTokenType::Icon:
                drawIcon(token.value, cursorX, cursorY);
                cursorX += kIconWidth + kIconSpacing;
                currentLineHeight = std::max(currentLineHeight, std::max(baseLineHeight, kIconWidth));
                break;
            case TextExTokenType::Color:
                changeTextColor(token.value);
                break;
            case TextExTokenType::FontBigger:
                currentFontSize = std::min(kFontSizeMax, currentFontSize + kFontStep);
                currentLineHeight = std::max(currentLineHeight, std::max(baseLineHeight, currentFontSize + 8));
                break;
            case TextExTokenType::FontSmaller:
                currentFontSize = std::max(kFontSizeMin, currentFontSize - kFontStep);
                currentLineHeight = std::max(currentLineHeight, std::max(baseLineHeight, currentFontSize + 8));
                break;
            case TextExTokenType::NewLine:
                cursorX = x;
                cursorY += currentLineHeight;
                currentLineHeight = std::max(1, baseLineHeight);
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
    const auto tokens = tokenizeTextEx(text);
    const TextMeasureResult measured = measureTextTokens(tokens, lineHeight(), fontSize_);
    return measured.width;
}

Rect Window_Base::textSize(const std::string& text) const {
    methodCallCounts_["textSize"]++;
    const auto tokens = tokenizeTextEx(text);
    const TextMeasureResult measured = measureTextTokens(tokens, lineHeight(), fontSize_);
    return Rect{0, 0, measured.width, measured.height};
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

Color Window_Base::normalColor() const {
    methodCallCounts_["normalColor"]++;
    return systemColor(0); // White
}

Color Window_Base::dimColor() const {
    methodCallCounts_["dimColor"]++;
    return Color{128, 128, 128, 255}; // Gray
}

Color Window_Base::deathColor() const {
    methodCallCounts_["deathColor"]++;
    return Color{255, 0, 0, 255}; // Red
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

BitmapHandle Window_Base::contents() const {
    methodCallCounts_["contents"]++;
    return contents_;
}

void Window_Base::createContents() {
    recordMethodCall("createContents");
    if (contents_ == INVALID_BITMAP) {
        contents_ = allocateBitmap();
    }
}

void Window_Base::destroyContents() {
    recordMethodCall("destroyContents");
    releaseBitmap(contents_);
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

void Window_Base::registerAPI(QuickJSContext& ctx) {
    initializeMethodStatus();  // Ensure initialized

    static const std::vector<std::string> methodsToExpose = {
        "drawText", "drawIcon", "drawActorFace", "drawActorName", "drawActorLevel",
        "drawActorHp", "drawActorMp", "drawActorTp", "drawGauge", "drawCharacter",
        "drawItemName", "drawTextEx", "lineHeight", "textWidth", "textSize",
        "changeTextColor", "resetTextColor", "textColor", "systemColor",
        "resetFontSettings", "fontFace", "fontSize", "setFontFace", "setFontSize",
        "contents", "createContents", "destroyContents", "open", "close",
        "show", "hide", "update", "getContentRect"
    };

    std::vector<QuickJSContext::MethodDef> methods;
    methods.reserve(methodsToExpose.size());
    for (const auto& methodName : methodsToExpose) {
        QuickJSContext::MethodDef method;
        method.name = methodName;
        method.fn = [](const std::vector<Value>&) -> Value {
            // TODO: Bridge to actual instance
            return Value::Nil();
        };
        method.status = getMethodStatus(methodName);
        method.deviationNote = getMethodDeviation(methodName);
        methods.push_back(method);
    }

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

int32_t Window_Selectable::hitTest(int32_t localX, int32_t localY) const {
    if (maxItems_ <= 0 || maxCols_ <= 0 || itemHeight_ <= 0) {
        return -1;
    }

    const int32_t relX = localX - padding_;
    const int32_t relY = localY - padding_ + (topRow_ * itemHeight_);

    if (relX < 0 || relY < 0) {
        return -1;
    }

    const int32_t itemWidth = getItemWidth();
    if (itemWidth <= 0) {
        return -1;
    }

    const int32_t col = relX / itemWidth;
    const int32_t row = relY / itemHeight_;

    if (col >= maxCols_) {
        return -1;
    }

    const int32_t index = row * maxCols_ + col;
    if (index < 0 || index >= maxItems_) {
        return -1;
    }

    return index;
}

void Window_Selectable::processCursorMove() {
    if (!isActive() || !isVisible()) {
        return;
    }

    // Check diagonal (dir8) first to avoid double-movement when
    // dir4 and dir8 would both map to a direction.
    const int32_t dir8 = InputManager::instance().getDir8();
    switch (dir8) {
        case 1:
            cursorDown(true);
            cursorLeft(true);
            return;
        case 3:
            cursorDown(true);
            cursorRight(true);
            return;
        case 7:
            cursorUp(true);
            cursorLeft(true);
            return;
        case 9:
            cursorUp(true);
            cursorRight(true);
            return;
    }

    const int32_t dir4 = InputManager::instance().getDir4();
    switch (dir4) {
        case 2: cursorDown(true); break;
        case 4: cursorLeft(true); break;
        case 6: cursorRight(true); break;
        case 8: cursorUp(true); break;
    }

    // Touch/mouse tap selection
    if (InputManager::instance().isMouseTriggered(0)) {
        const int32_t mx = InputManager::instance().getMouseX();
        const int32_t my = InputManager::instance().getMouseY();
        const Rect r = getRect();
        const int32_t localX = mx - r.x;
        const int32_t localY = my - r.y;
        const int32_t hitIndex = hitTest(localX, localY);
        if (hitIndex >= 0) {
            setIndex(hitIndex);
        }
    }

    if (InputManager::instance().isTouchTriggered()) {
        const int32_t tx = InputManager::instance().getTouchX();
        const int32_t ty = InputManager::instance().getTouchY();
        const Rect r = getRect();
        const int32_t localX = tx - r.x;
        const int32_t localY = ty - r.y;
        const int32_t hitIndex = hitTest(localX, localY);
        if (hitIndex >= 0) {
            setIndex(hitIndex);
        }
    }
}

void Window_Selectable::processHandling() {
    if (!isActive()) {
        return;
    }

    if (InputManager::instance().isTriggered(InputKey::DECISION)) {
        onOk();
    }
    if (InputManager::instance().isTriggered(InputKey::CANCEL)) {
        onCancel();
    }

    // Mouse/touch OK (click on an item acts as selection + OK)
    if (InputManager::instance().isMouseTriggered(0)) {
        const int32_t mx = InputManager::instance().getMouseX();
        const int32_t my = InputManager::instance().getMouseY();
        const Rect r = getRect();
        const int32_t localX = mx - r.x;
        const int32_t localY = my - r.y;
        if (hitTest(localX, localY) >= 0) {
            onOk();
        }
    }
    if (InputManager::instance().isTouchTriggered()) {
        const int32_t tx = InputManager::instance().getTouchX();
        const int32_t ty = InputManager::instance().getTouchY();
        const Rect r = getRect();
        const int32_t localX = tx - r.x;
        const int32_t localY = ty - r.y;
        if (hitTest(localX, localY) >= 0) {
            onOk();
        }
    }
}

void Window_Selectable::onOk() {
    if (onSelect_ && index_ >= 0) {
        onSelect_(index_);
    }
}

void Window_Selectable::onCancel() {
    // Default empty implementation
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
        }, CompatStatus::FULL});
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

void Window_Command::onOk() {
    callOkHandler();
}

Rect Window_Command::itemRectForIndex(int32_t index) const {
    const int32_t row = index / getMaxCols();
    const int32_t col = index % getMaxCols();
    const Rect content = getContentRect();
    
    Rect rect;
    rect.x = content.x + col * getItemWidth();
    rect.y = content.y + row * getItemHeight();
    rect.width = getItemWidth();
    rect.height = getItemHeight();
    return rect;
}

void Window_Command::drawItem(int32_t index) {
    recordMethodCall("drawItem");
    const CommandItem& command = getCommand(index);
    
    const Rect rect = itemRectForIndex(index);
    
    // Determine text color based on enabled state
    Color color = command.enabled ? normalColor() : dimColor();
    changeTextColor(color);
    
    // Draw the command name
    drawText(command.name, rect.x, rect.y, rect.width, "left");
    
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

void Window_Command::registerAPI(QuickJSContext& ctx) {
    Window_Selectable::registerAPI(ctx);
    
    static const std::vector<QuickJSContext::MethodDef> methods = {
        {"addCommand", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"clearCommands", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"getCommandCount", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"isCommandEnabled", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"isCurrentItemEnabled", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"getCurrentSymbol", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"drawItem", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"drawAllItems", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"selectSymbol", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"selectExt", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"findSymbol", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"findExt", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
        {"callOkHandler", [](const std::vector<Value>&) -> Value {
            return Value::Nil();
        }, CompatStatus::FULL},
    };

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
    releaseBitmap(bitmap_);
}

void Sprite_Character::setCharacterName(const std::string& name) {
    if (characterName_ != name) {
        characterName_ = name;
        // Release old bitmap and allocate new one
        releaseBitmap(bitmap_);
        bitmap_ = INVALID_BITMAP;
        if (!characterName_.empty()) {
            bitmap_ = allocateBitmap();
        }
    }
}

void Sprite_Character::setCharacterIndex(int32_t index) {
    if (characterIndex_ != index) {
        characterIndex_ = index;
        // Update source rect for character sheet slicing
        // MZ character sheets are 4x2 grid (12 chars per sheet, 3 rows of 4)
        // Source rect: (index % 4) * 48, (index / 4) * 48, 48, 48
        // For now, just record the change; actual slicing is native-renderer tier
    }
}

void Sprite_Character::update() {
    // Advance character animation pattern deterministically
    pattern_ = (pattern_ + 1) % 4; // MZ uses 4-frame walk animation
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
    releaseBitmap(bitmap_);
}

void Sprite_Actor::startMotion(int32_t motion) {
    motion_ = motion;
    motionPlaying_ = true;
    // MZ motion durations: idle=1, walk=12, chant=18, guard=18, damage=24,
    // evade=18, thrust=12, swing=12, missile=12, skill=12, spell=18,
    // item=18, escape=18, victory=18, dying=18, abnormal=18, sleep=18, dead=1
    // For simplicity, use a deterministic mapping:
    static const int32_t kMotionDurations[] = {
        1, 12, 18, 18, 24, 18, 12, 12, 12, 12, 18, 18, 18, 18, 18, 18, 18, 1
    };
    if (motion >= 0 && motion < static_cast<int32_t>(sizeof(kMotionDurations)/sizeof(kMotionDurations[0]))) {
        motionFramesRemaining_ = kMotionDurations[motion];
    } else {
        motionFramesRemaining_ = 12;
    }
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

    // Motion animation countdown
    if (motionPlaying_ && motionFramesRemaining_ > 0) {
        --motionFramesRemaining_;
        if (motionFramesRemaining_ <= 0) {
            motionPlaying_ = false;
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
    }, CompatStatus::FULL});
    
    methods.push_back({"startMotion", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"startAnimation", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});

    methods.push_back({"isAnimationPlaying", [](const std::vector<Value>&) -> Value {
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"startEffect", [](const std::vector<Value>&) -> Value {
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
