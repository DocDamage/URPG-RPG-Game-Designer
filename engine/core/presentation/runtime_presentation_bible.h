#pragma once

#include "engine/core/ui/urpg_design_tokens.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace urpg::presentation {

struct RuntimeFeedbackTier {
    std::string id;
    uint32_t priority = 0;
    uint32_t maximum_duration_ms = 0;
    bool interrupts_lower_priority = false;
    bool requires_non_color_cue = true;
};

struct RuntimeParticleLanguage {
    uint32_t maximum_concurrent_emitters = 0;
    uint32_t maximum_lifetime_ms = 0;
    float reduced_motion_density = 0.0F;
    std::vector<std::string> motifs;
};

struct RuntimeCameraLanguage {
    float maximum_shake_pixels = 0.0F;
    uint32_t maximum_hit_stop_ms = 0;
    uint32_t transition_ms = 0;
    bool motion_interruptible = true;
};

struct RuntimeAccessibilityVariant {
    std::string id;
    ui::UrpgThemeMode theme = ui::UrpgThemeMode::Dark;
    float ui_scale = 1.0F;
    bool reduced_motion = false;
    bool audio_optional = true;
    bool non_color_cues = true;
};

struct RuntimePresentationBible {
    std::string id;
    std::string revision;
    std::string visual_motif;
    std::string camera_motif;
    ui::UrpgDesignTokens default_tokens;
    std::map<std::string, std::string> sound_motifs;
    std::vector<RuntimeFeedbackTier> feedback_hierarchy;
    RuntimeParticleLanguage particles;
    RuntimeCameraLanguage camera;
    std::vector<RuntimeAccessibilityVariant> accessibility_variants;
};

RuntimePresentationBible makeRuntimePresentationBible();
std::vector<std::string> validateRuntimePresentationBible(const RuntimePresentationBible& bible);
nlohmann::json runtimePresentationBibleSnapshot(const RuntimePresentationBible& bible);
nlohmann::json runtimePresentationComponentShowcase(const RuntimePresentationBible& bible,
                                                    const RuntimeAccessibilityVariant& variant);

} // namespace urpg::presentation
