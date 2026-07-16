#pragma once

#include "engine/core/accessibility/inclusive_experience.h"
#include "engine/core/presentation/battle_feedback.h"
#include "engine/core/presentation/exploration_feedback.h"
#include "engine/core/presentation/runtime_feedback_stack.h"
#include "engine/core/ui/urpg_design_tokens.h"

#include <optional>

namespace urpg::accessibility {

struct InclusiveRuntimePolicy {
    ui::UrpgDesignTokens design_tokens;
    presentation::RuntimeFeedbackSettings shared_feedback;
    presentation::ExplorationFeedbackSettings exploration_feedback;
    presentation::BattleFeedbackSettings battle_feedback;
    ColorFilter color_filter = ColorFilter::None;
    float flash_intensity = 1.0F;
    bool non_color_cues = true;
    bool subtitles = true;
    bool captions = true;
    bool mono_audio = false;
};

std::optional<InclusiveRuntimePolicy> inclusiveRuntimePolicy(const InclusiveSettings& settings);
bool applyInclusiveRuntimePolicy(const InclusiveSettings& settings,
                                 presentation::RuntimeFeedbackStack* shared,
                                 presentation::ExplorationFeedbackDirector* exploration,
                                 presentation::BattleFeedbackDirector* battle);

} // namespace urpg::accessibility
