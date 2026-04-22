#include "effect_translator.h"

namespace urpg::presentation::effects {

void EffectTranslator::append(const std::vector<ResolvedEffectInstance>& instances,
                              urpg::presentation::PresentationFrameIntent& intent) const {
    for (const auto& instance : instances) {
        const auto effectId = static_cast<uint32_t>(instance.ownerId);
        const auto intensity = instance.intensity.value;

        if (instance.placement == EffectPlacement::World) {
            intent.AddWorldEffect(
                effectId,
                {instance.position[0], instance.position[1], instance.position[2]},
                instance.ownerId,
                instance.duration,
                instance.scale,
                intensity,
                instance.color,
                instance.overlayEmphasis.value);
            continue;
        }

        intent.AddOverlayEffect(
            effectId,
            {instance.position[0], instance.position[1], instance.position[2]},
            instance.ownerId,
            instance.duration,
            instance.scale,
            intensity,
            instance.color,
            1.0f,
            instance.overlayEmphasis.value);
    }
}

} // namespace urpg::presentation::effects
