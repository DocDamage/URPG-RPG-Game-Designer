#pragma once

#include <compare>
#include <cstdint>
#include <tuple>

#include "engine/core/presentation/effects/effect_instance.h"

namespace urpg::presentation::effects {

enum class EffectCueKind {
    Gameplay,
    Status,
    System,
    CastStart,
    HitConfirm,
    CriticalHit,
    BloodSplatter,
    GuardClash,
    MissSweep,
    HealPulse,
    DefeatFade,
    PhaseBanner
};

enum class EffectAnchorMode { World, Owner, Target, Screen, Overlay };

struct EffectCue {
    std::uint64_t frameTick = 0;
    std::uint32_t sequenceIndex = 0;
    std::uint64_t sourceId = 0;
    std::uint64_t ownerId = 0;
    EffectCueKind kind = EffectCueKind::Gameplay;
    EffectAnchorMode anchorMode = EffectAnchorMode::World;
    OverlayEmphasis overlayEmphasis{};
    EffectIntensity intensity{};

    auto operator<=>(const EffectCue& other) const noexcept {
        return std::tie(frameTick, sequenceIndex, sourceId, ownerId) <=>
               std::tie(other.frameTick, other.sequenceIndex, other.sourceId, other.ownerId);
    }

    bool operator==(const EffectCue&) const = default;
};

inline bool effectCueLess(const EffectCue& lhs, const EffectCue& rhs) noexcept {
    return lhs < rhs;
}

} // namespace urpg::presentation::effects
