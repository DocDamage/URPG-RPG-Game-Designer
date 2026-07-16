#include "engine/core/accessibility/inclusive_runtime_policy.h"

#include <algorithm>

namespace urpg::accessibility {
namespace {

float textScale(const TextScaleProfile profile) {
    switch (profile) {
    case TextScaleProfile::Compact: return 0.85F;
    case TextScaleProfile::Standard: return 1.0F;
    case TextScaleProfile::Large: return 1.5F;
    case TextScaleProfile::ExtraLarge: return 2.0F;
    }
    return 1.0F;
}

presentation::BattleColorFilter battleFilter(const ColorFilter filter) {
    switch (filter) {
    case ColorFilter::None: return presentation::BattleColorFilter::None;
    case ColorFilter::Protanopia: return presentation::BattleColorFilter::Protanopia;
    case ColorFilter::Deuteranopia: return presentation::BattleColorFilter::Deuteranopia;
    case ColorFilter::Tritanopia: return presentation::BattleColorFilter::Tritanopia;
    case ColorFilter::Monochrome: return presentation::BattleColorFilter::Monochrome;
    }
    return presentation::BattleColorFilter::None;
}

} // namespace

std::optional<InclusiveRuntimePolicy> inclusiveRuntimePolicy(const InclusiveSettings& settings) {
    if (!settings.isValid()) return std::nullopt;

    InclusiveRuntimePolicy policy;
    policy.design_tokens = ui::makeUrpgDesignTokens(settings.high_contrast ? ui::UrpgThemeMode::HighContrast
                                                                           : ui::UrpgThemeMode::Dark,
                                                    textScale(settings.text_scale), settings.reduced_motion);
    const bool effects_audio = settings.master_volume > 0.0F && settings.effects_volume > 0.0F;
    const bool motion_enabled = !settings.reduced_motion;
    const bool shake_enabled = motion_enabled && settings.screen_shake > 0.0F;
    const float intensity = std::clamp(std::max(settings.screen_shake, settings.flash_intensity), 0.0F, 1.0F);
    policy.shared_feedback = {effects_audio, true, motion_enabled, shake_enabled, motion_enabled,
                              settings.reduced_motion, intensity};
    policy.exploration_feedback = {effects_audio, settings.reduced_motion, settings.captions, 1.0F,
                                   true, motion_enabled, shake_enabled, motion_enabled};
    policy.battle_feedback = {effects_audio, settings.reduced_motion, battleFilter(settings.color_filter),
                              presentation::BattlePlaybackMode::Normal, true, motion_enabled,
                              shake_enabled, motion_enabled};
    policy.color_filter = settings.color_filter;
    policy.flash_intensity = settings.flash_intensity;
    policy.non_color_cues = settings.non_color_cues;
    policy.subtitles = settings.subtitles;
    policy.captions = settings.captions;
    policy.mono_audio = settings.mono_audio;
    return policy;
}

bool applyInclusiveRuntimePolicy(const InclusiveSettings& settings,
                                 presentation::RuntimeFeedbackStack* shared,
                                 presentation::ExplorationFeedbackDirector* exploration,
                                 presentation::BattleFeedbackDirector* battle) {
    const auto policy = inclusiveRuntimePolicy(settings);
    if (!policy) return false;
    if (shared) shared->setSettings(policy->shared_feedback);
    if (exploration) exploration->setSettings(policy->exploration_feedback);
    if (battle) battle->setSettings(policy->battle_feedback);
    return true;
}

} // namespace urpg::accessibility
