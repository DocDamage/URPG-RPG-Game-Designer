#pragma once

#include <string>

#include "engine/core/presentation/effects/effect_instance.h"

namespace urpg::presentation::effects {

struct EffectPreset {
    std::string id;
    EffectPlacement placement = EffectPlacement::World;
    float duration = 0.0f;
    float scale = 1.0f;
    std::array<float, 4> color = {1.0f, 1.0f, 1.0f, 1.0f};
    OverlayEmphasis overlayEmphasis {};
};

class EffectCatalog {
public:
    static const EffectPreset& fallbackWorld();
    static const EffectPreset& fallbackOverlay();
    static const EffectPreset& castSmall();
    static const EffectPreset& impactHeavy();
    static const EffectPreset& critBurst();
};

} // namespace urpg::presentation::effects
