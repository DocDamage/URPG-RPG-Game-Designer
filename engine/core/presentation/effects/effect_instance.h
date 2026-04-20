#pragma once

#include <array>
#include <cstdint>

namespace urpg::presentation::effects {

struct OverlayEmphasis {
    float value = 0.0f;

    bool operator==(const OverlayEmphasis&) const = default;
};

struct EffectIntensity {
    float value = 1.0f;

    bool operator==(const EffectIntensity&) const = default;
};

enum class EffectPlacement {
    World,
    Owner,
    Screen,
    Overlay
};

struct ResolvedEffectInstance {
    EffectPlacement placement = EffectPlacement::World;
    std::array<float, 3> position = {0.0f, 0.0f, 0.0f};
    std::uint64_t ownerId = 0;
    float duration = 0.0f;
    float scale = 1.0f;
    EffectIntensity intensity {};
    std::array<float, 4> color = {1.0f, 1.0f, 1.0f, 1.0f};
    OverlayEmphasis overlayEmphasis {};
};

} // namespace urpg::presentation::effects
