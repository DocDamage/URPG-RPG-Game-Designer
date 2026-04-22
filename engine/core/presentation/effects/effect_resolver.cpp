#include "engine/core/presentation/effects/effect_resolver.h"

#include <algorithm>

#include "engine/core/presentation/battle_scene_state.h"

namespace urpg::presentation::effects {

namespace {

constexpr float kHeavyCueThreshold = 1.5f;
constexpr float kCriticalCueThreshold = 1.9f;
constexpr float kCriticalOverlayThreshold = 0.8f;

std::uint64_t resolveAnchorCueId(const EffectCue& cue) noexcept {
    switch (cue.anchorMode) {
    case EffectAnchorMode::Owner:
        return cue.sourceId != 0 ? cue.sourceId : cue.ownerId;
    case EffectAnchorMode::Target:
        return cue.ownerId != 0 ? cue.ownerId : cue.sourceId;
    case EffectAnchorMode::World:
        return cue.ownerId != 0 ? cue.ownerId : cue.sourceId;
    case EffectAnchorMode::Screen:
    case EffectAnchorMode::Overlay:
        return 0;
    }

    return 0;
}

void applyBattleAnchorPosition(ResolvedEffectInstance& instance,
                               const EffectCue& cue,
                               const urpg::presentation::BattleSceneState* battleState) {
    if (battleState == nullptr) {
        return;
    }

    const std::uint64_t anchorCueId = resolveAnchorCueId(cue);
    if (anchorCueId == 0) {
        return;
    }

    for (const auto& participant : battleState->participants) {
        if (participant.cueId != anchorCueId || !participant.hasAnchorPosition) {
            continue;
        }

        instance.position = {
            participant.anchorPosition.x,
            participant.anchorPosition.y,
            participant.anchorPosition.z
        };
        return;
    }
}

} // namespace

bool EffectResolver::isCriticalCue(const EffectCue& cue) noexcept {
    return cue.intensity.value >= kCriticalCueThreshold || cue.overlayEmphasis.value >= kCriticalOverlayThreshold;
}

bool EffectResolver::isHeavyCue(const EffectCue& cue) noexcept {
    return cue.intensity.value >= kHeavyCueThreshold;
}

ResolvedEffectInstance EffectResolver::makeInstance(const EffectCue& cue, const EffectPreset& preset,
                                                    const urpg::presentation::BattleSceneState* battleState,
                                                    std::optional<EffectPlacement> placementOverride,
                                                    float overlayEmphasisOverride) {
    ResolvedEffectInstance instance;
    instance.placement = placementOverride.value_or(preset.placement);
    instance.ownerId = cue.ownerId != 0 ? cue.ownerId : cue.sourceId;
    instance.duration = preset.duration;
    instance.scale = preset.scale;
    instance.intensity = cue.intensity;
    instance.color = preset.color;
    instance.overlayEmphasis = overlayEmphasisOverride >= 0.0f
        ? OverlayEmphasis{overlayEmphasisOverride}
        : preset.overlayEmphasis;
    applyBattleAnchorPosition(instance, cue, battleState);
    return instance;
}

std::vector<ResolvedEffectInstance> EffectResolver::resolve(const EffectCue& cue,
                                                            urpg::presentation::CapabilityTier tier,
                                                            const urpg::presentation::BattleSceneState* battleState) const {
    std::vector<ResolvedEffectInstance> resolved;

    switch (cue.kind) {
    case EffectCueKind::HealPulse:
        resolved.push_back(makeInstance(cue, EffectCatalog::healGlow(), battleState, EffectPlacement::World, -1.0f));
        return resolved;
    case EffectCueKind::MissSweep:
        resolved.push_back(makeInstance(cue, EffectCatalog::missSweep(), battleState, EffectPlacement::World, -1.0f));
        return resolved;
    case EffectCueKind::DefeatFade:
        resolved.push_back(makeInstance(cue, EffectCatalog::defeatFade(), battleState, std::nullopt, -1.0f));
        return resolved;
    case EffectCueKind::PhaseBanner:
        resolved.push_back(makeInstance(cue, EffectCatalog::phaseBanner(), battleState, std::nullopt, -1.0f));
        return resolved;
    case EffectCueKind::CastStart:
        resolved.push_back(makeInstance(cue, EffectCatalog::castSmall(), battleState, EffectPlacement::World, -1.0f));
        return resolved;
    case EffectCueKind::GuardClash:
        resolved.push_back(makeInstance(cue, EffectCatalog::impactHeavy(), battleState, EffectPlacement::World, -1.0f));
        return resolved;
    case EffectCueKind::CriticalHit:
    case EffectCueKind::HitConfirm:
    case EffectCueKind::Gameplay:
    case EffectCueKind::Status:
    case EffectCueKind::System:
        break;
    }

    const bool critical = (cue.kind == EffectCueKind::CriticalHit) || isCriticalCue(cue);
    const bool heavy = isHeavyCue(cue);
    const bool lowTier = tier == urpg::presentation::CapabilityTier::Tier0_Baseline;

    if (critical) {
        if (lowTier) {
            resolved.push_back(makeInstance(cue, EffectCatalog::fallbackOverlay(), battleState, std::nullopt,
                                            std::max(cue.overlayEmphasis.value, 0.95f)));
            return resolved;
        }

        resolved.push_back(makeInstance(cue, EffectCatalog::critBurst(), battleState, EffectPlacement::World, 0.0f));
        resolved.push_back(makeInstance(cue, EffectCatalog::fallbackOverlay(), battleState, std::nullopt,
                                        std::max(cue.overlayEmphasis.value, 0.95f)));
        return resolved;
    }

    if (heavy) {
        if (lowTier) {
            resolved.push_back(makeInstance(cue, EffectCatalog::impactHeavy(), battleState, EffectPlacement::World, 0.0f));
            return resolved;
        }

        resolved.push_back(makeInstance(cue, EffectCatalog::impactHeavy(), battleState, EffectPlacement::World, 0.0f));
        resolved.push_back(makeInstance(cue, EffectCatalog::fallbackOverlay(), battleState, std::nullopt,
                                        std::max(cue.overlayEmphasis.value, 0.60f)));
        return resolved;
    }

    if (cue.anchorMode == EffectAnchorMode::Overlay || cue.anchorMode == EffectAnchorMode::Screen) {
        resolved.push_back(makeInstance(cue, EffectCatalog::fallbackOverlay(), battleState, std::nullopt, cue.overlayEmphasis.value));
        return resolved;
    }

    resolved.push_back(makeInstance(cue, EffectCatalog::fallbackWorld(), battleState, EffectPlacement::World, 0.0f));
    return resolved;
}

} // namespace urpg::presentation::effects
