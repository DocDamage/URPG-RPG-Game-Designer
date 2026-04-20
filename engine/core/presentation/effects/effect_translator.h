#pragma once

#include <vector>

#include "effect_instance.h"
#include "engine/core/presentation/presentation_runtime.h"

namespace urpg::presentation::effects {

class EffectTranslator {
public:
    void append(const std::vector<ResolvedEffectInstance>& instances,
                urpg::presentation::PresentationFrameIntent& intent) const;
};

} // namespace urpg::presentation::effects
