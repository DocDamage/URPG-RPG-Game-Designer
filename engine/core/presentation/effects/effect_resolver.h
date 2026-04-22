#pragma once

#include <optional>
#include <vector>

#include "engine/core/presentation/effects/effect_catalog.h"
#include "engine/core/presentation/effects/effect_cue.h"
#include "engine/core/presentation/presentation_types.h"

namespace urpg::presentation {
struct BattleSceneState;
}

namespace urpg::presentation::effects {

class EffectResolver {
public:
    std::vector<ResolvedEffectInstance> resolve(const EffectCue& cue,
                                               urpg::presentation::CapabilityTier tier,
                                               const urpg::presentation::BattleSceneState* battleState = nullptr) const;

private:
    static bool isCriticalCue(const EffectCue& cue) noexcept;
    static bool isHeavyCue(const EffectCue& cue) noexcept;
    static ResolvedEffectInstance makeInstance(const EffectCue& cue, const EffectPreset& preset,
                                               const urpg::presentation::BattleSceneState* battleState = nullptr,
                                               std::optional<EffectPlacement> placementOverride = std::nullopt,
                                               float overlayEmphasisOverride = -1.0f);
};

} // namespace urpg::presentation::effects
