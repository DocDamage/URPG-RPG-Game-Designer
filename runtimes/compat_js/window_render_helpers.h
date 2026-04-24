#pragma once

#include "data_manager.h"
#include "window_compat.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace urpg::compat {

constexpr int32_t kIconWidth = 32;
constexpr int32_t kIconSpacing = 4;
constexpr int32_t kFaceCellWidth = 144;
constexpr int32_t kFaceCellHeight = 144;
constexpr int32_t kFaceSheetCols = 4;

int32_t resolveActorGaugeMax(
    const DataManager& data,
    const ActorData* actor,
    int32_t actorId,
    int32_t paramId,
    int32_t fallback);
int32_t resolveActorGaugeCurrent(const ActorData* actor, int32_t fallback, int32_t maxValue, const char* gauge);
void drawActorResourceGauge(
    Window_Base& window,
    int32_t x,
    int32_t y,
    int32_t width,
    const char* label,
    int32_t currentValue,
    int32_t maxValue,
    const Color& color1,
    const Color& color2);

const ItemData* resolveAnyItemById(DataManager& data, int32_t itemId);
std::string resolveItemLabel(const ItemData* item, int32_t itemId);
int32_t resolveItemIconIndex(const ItemData* item);

int32_t resolveEffectDurationFrames(std::string_view effect);
int32_t resolveAnimationDurationFrames(int32_t animationId);
bool isLoopingActorMotion(int32_t motion);
int32_t resolveActorMotionDurationFrames(int32_t motion);

int32_t normalizeCharacterIndex(int32_t characterIndex);
int32_t normalizeCharacterPattern(int32_t pattern);
int32_t normalizeCharacterDirection(int32_t direction);
Rect resolveCharacterSourceRect(int32_t characterIndex, int32_t direction, int32_t pattern);

int32_t resolveActorFaceIndex(const ActorData* actor, int32_t actorId);
std::string resolveActorFaceName(const ActorData* actor, int32_t actorId);
std::string resolveActorBattlerAssetId(int32_t actorId);

} // namespace urpg::compat
