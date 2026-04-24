#include "window_render_helpers.h"

#include <algorithm>
#include <cstring>

namespace urpg::compat {

namespace {

constexpr int32_t kFaceSheetRows = 2;

int32_t resolveCharacterDirectionRow(int32_t direction) {
    switch (normalizeCharacterDirection(direction)) {
    case 2:
        return 0;
    case 4:
        return 1;
    case 6:
        return 2;
    case 8:
        return 3;
    default:
        return 0;
    }
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

} // namespace

int32_t resolveActorGaugeMax(
    const DataManager& data,
    const ActorData* actor,
    int32_t actorId,
    int32_t paramId,
    int32_t fallback) {
    if (!actor) {
        return fallback;
    }
    return std::max(1, data.getActorParam(actorId, paramId, actor->level));
}

int32_t resolveActorGaugeCurrent(const ActorData* actor, int32_t fallback, int32_t maxValue, const char* gauge) {
    if (!actor) {
        return fallback;
    }
    if (std::strcmp(gauge, "hp") == 0) {
        return std::clamp(actor->hp, 0, maxValue);
    }
    if (std::strcmp(gauge, "mp") == 0) {
        return std::clamp(actor->mp, 0, maxValue);
    }
    return std::clamp(actor->tp, 0, maxValue);
}

void drawActorResourceGauge(
    Window_Base& window,
    int32_t x,
    int32_t y,
    int32_t width,
    const char* label,
    int32_t currentValue,
    int32_t maxValue,
    const Color& color1,
    const Color& color2) {
    const int32_t gaugeWidth = std::max(0, width);
    const int32_t gaugeY = y + std::max(0, window.lineHeight() - 12);
    const double rate =
        maxValue > 0 ? static_cast<double>(currentValue) / static_cast<double>(maxValue) : 0.0;

    window.drawGauge(x, gaugeY, std::max(1, gaugeWidth), std::clamp(rate, 0.0, 1.0), color1, color2);
    window.drawText(label, x, y, std::min(gaugeWidth, 40));

    const int32_t valueWidth = std::min(std::max(gaugeWidth - 44, 0), 96);
    if (valueWidth > 0) {
        window.drawText(std::to_string(currentValue), x + gaugeWidth - valueWidth, y, valueWidth, "right");
    }
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

bool isLoopingActorMotion(int32_t motion) {
    switch (motion) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
        return true;
    default:
        return false;
    }
}

int32_t resolveActorMotionDurationFrames(int32_t motion) {
    if (motion < 0) {
        return 0;
    }
    if (isLoopingActorMotion(motion)) {
        return 12;
    }
    return 18;
}

int32_t normalizeCharacterIndex(int32_t characterIndex) {
    constexpr int32_t kCharactersPerSheet = 8;
    if (characterIndex < 0) {
        return 0;
    }
    return characterIndex % kCharactersPerSheet;
}

int32_t normalizeCharacterPattern(int32_t pattern) {
    constexpr int32_t kPatternCount = 3;
    if (pattern < 0) {
        return 0;
    }
    return pattern % kPatternCount;
}

int32_t normalizeCharacterDirection(int32_t direction) {
    switch (direction) {
    case 2:
    case 4:
    case 6:
    case 8:
        return direction;
    default:
        return 2;
    }
}

Rect resolveCharacterSourceRect(int32_t characterIndex, int32_t direction, int32_t pattern) {
    constexpr int32_t kCharSheetCols = 4;
    constexpr int32_t kCharCellWidth = 48;
    constexpr int32_t kCharCellHeight = 48;
    const int32_t normalizedIndex = normalizeCharacterIndex(characterIndex);
    const int32_t normalizedPattern = normalizeCharacterPattern(pattern);
    const int32_t row = resolveCharacterDirectionRow(direction);
    const int32_t baseX = (normalizedIndex % kCharSheetCols) * 3 * kCharCellWidth;
    const int32_t baseY = (normalizedIndex / kCharSheetCols) * 4 * kCharCellHeight;
    return Rect{
        baseX + normalizedPattern * kCharCellWidth,
        baseY + row * kCharCellHeight,
        kCharCellWidth,
        kCharCellHeight,
    };
}

int32_t resolveActorFaceIndex(const ActorData* actor, int32_t actorId) {
    if (actor != nullptr) {
        return normalizeFaceIndex(actor->faceIndex);
    }
    return normalizeFaceIndex(std::max(0, actorId - 1));
}

std::string resolveActorFaceName(const ActorData* actor, int32_t actorId) {
    if (actor != nullptr && !actor->faceName.empty()) {
        return actor->faceName;
    }
    return "ActorFace_" + std::to_string(std::max(0, actorId));
}

std::string resolveActorBattlerAssetId(int32_t actorId) {
    if (actorId <= 0) {
        return "";
    }

    if (const ActorData* actor = DataManager::instance().getActor(actorId)) {
        return actor->battlerName;
    }
    return "";
}

} // namespace urpg::compat
