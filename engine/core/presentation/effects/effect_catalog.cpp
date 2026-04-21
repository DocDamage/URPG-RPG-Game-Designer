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

const EffectPreset kHealGlow {
    "heal_glow",
    EffectPlacement::World,
    0.28f,
    1.05f,
    {0.68f, 1.0f, 0.72f, 1.0f},
    {0.0f},
};

const EffectPreset kMissSweep {
    "miss_sweep",
    EffectPlacement::World,
    0.20f,
    0.90f,
    {0.72f, 0.78f, 0.88f, 1.0f},
    {0.0f},
};

const EffectPreset kDefeatFade {
    "defeat_fade",
    EffectPlacement::Overlay,
    0.45f,
    1.10f,
    {0.45f, 0.30f, 0.55f, 1.0f},
    {0.40f},
};

const EffectPreset kPhaseBanner {
    "phase_banner",
    EffectPlacement::Overlay,
    0.35f,
    1.20f,
    {1.0f, 0.92f, 0.60f, 1.0f},
    {0.80f},
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

const EffectPreset& EffectCatalog::healGlow() {
    return kHealGlow;
}

const EffectPreset& EffectCatalog::missSweep() {
    return kMissSweep;
}

const EffectPreset& EffectCatalog::defeatFade() {
    return kDefeatFade;
}

const EffectPreset& EffectCatalog::phaseBanner() {
    return kPhaseBanner;
}

} // namespace urpg::presentation::effects
