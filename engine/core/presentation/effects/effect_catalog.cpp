#include "engine/core/presentation/effects/effect_catalog.h"

namespace urpg::presentation::effects {

namespace {

const EffectPreset kFallbackWorld {
    "fallback_world",
    EffectPlacement::World,
    0.18f,
    1.0f,
    {1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f},
};

const EffectPreset kFallbackOverlay {
    "fallback_overlay",
    EffectPlacement::Overlay,
    0.08f,
    1.0f,
    {1.0f, 1.0f, 1.0f, 1.0f},
    {0.60f},
};

const EffectPreset kCastSmall {
    "cast_small",
    EffectPlacement::World,
    0.22f,
    0.95f,
    {0.72f, 0.84f, 1.0f, 1.0f},
    {0.0f},
};

const EffectPreset kImpactHeavy {
    "impact_heavy",
    EffectPlacement::World,
    0.26f,
    1.15f,
    {1.0f, 0.90f, 0.78f, 1.0f},
    {0.0f},
};

const EffectPreset kCritBurst {
    "crit_burst",
    EffectPlacement::World,
    0.30f,
    1.25f,
    {1.0f, 0.68f, 0.68f, 1.0f},
    {0.0f},
};

} // namespace

const EffectPreset& EffectCatalog::fallbackWorld() {
    return kFallbackWorld;
}

const EffectPreset& EffectCatalog::fallbackOverlay() {
    return kFallbackOverlay;
}

const EffectPreset& EffectCatalog::castSmall() {
    return kCastSmall;
}

const EffectPreset& EffectCatalog::impactHeavy() {
    return kImpactHeavy;
}

const EffectPreset& EffectCatalog::critBurst() {
    return kCritBurst;
}

} // namespace urpg::presentation::effects
