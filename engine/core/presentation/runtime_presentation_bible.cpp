#include "engine/core/presentation/runtime_presentation_bible.h"

#include <algorithm>
#include <set>

namespace urpg::presentation {

RuntimePresentationBible makeRuntimePresentationBible() {
    RuntimePresentationBible bible;
    bible.id = "urpg.runtime_presentation.playful_workshop";
    bible.revision = "1.0.0-candidate";
    bible.visual_motif = "inked field-guide shapes, warm workshop surfaces, and sky-blue action signals";
    bible.camera_motif = "player-led framing with short bounded emphasis and no decorative drift";
    bible.default_tokens = ui::makeUrpgDesignTokens(ui::UrpgThemeMode::Dark);
    bible.sound_motifs = {{"focus", "urpg.sound.focus_tick"}, {"confirm", "urpg.sound.confirm"},
                          {"cancel", "urpg.sound.cancel"}, {"error", "urpg.sound.warning"},
                          {"reward", "urpg.sound.reward_rise"}, {"quest", "urpg.sound.quest_chime"}};
    bible.feedback_hierarchy = {
        {"critical", 400, 1500, true, true}, {"primary", 300, 900, true, true},
        {"secondary", 200, 700, false, true}, {"ambient", 100, 1200, false, false}};
    bible.particles = {24, 1200, 0.15F, {"paper_spark", "ink_arc", "star_notch", "soft_dust"}};
    bible.camera = {6.0F, 65, 160, true};
    bible.accessibility_variants = {
        {"default", ui::UrpgThemeMode::Dark, 1.0F, false, true, true},
        {"large_text", ui::UrpgThemeMode::Dark, 1.35F, false, true, true},
        {"high_contrast", ui::UrpgThemeMode::HighContrast, 1.0F, false, true, true},
        {"reduced_motion", ui::UrpgThemeMode::Dark, 1.0F, true, true, true}};
    return bible;
}

std::vector<std::string> validateRuntimePresentationBible(const RuntimePresentationBible& bible) {
    std::vector<std::string> diagnostics;
    if (!bible.id.starts_with("urpg.") || bible.revision.empty()) diagnostics.push_back("presentation_bible_identity_invalid");
    if (!ui::validateUrpgDesignTokens(bible.default_tokens).empty()) diagnostics.push_back("presentation_bible_tokens_invalid");
    for (const auto* sound : {"focus", "confirm", "cancel", "error", "reward", "quest"}) {
        const auto found = bible.sound_motifs.find(sound);
        if (found == bible.sound_motifs.end() || !found->second.starts_with("urpg.sound."))
            diagnostics.push_back(std::string("presentation_bible_sound_missing:") + sound);
    }
    std::set<std::string> feedback_ids;
    uint32_t previous_priority = UINT32_MAX;
    for (const auto& tier : bible.feedback_hierarchy) {
        if (tier.id.empty() || !feedback_ids.insert(tier.id).second || tier.priority >= previous_priority ||
            tier.maximum_duration_ms == 0 || tier.maximum_duration_ms > 2000) {
            diagnostics.push_back("presentation_bible_feedback_hierarchy_invalid");
            break;
        }
        previous_priority = tier.priority;
    }
    if (bible.feedback_hierarchy.size() != 4) diagnostics.push_back("presentation_bible_feedback_tiers_incomplete");
    if (bible.particles.maximum_concurrent_emitters == 0 || bible.particles.maximum_concurrent_emitters > 32 ||
        bible.particles.maximum_lifetime_ms > 1500 || bible.particles.reduced_motion_density > 0.25F)
        diagnostics.push_back("presentation_bible_particles_unbounded");
    if (bible.camera.maximum_shake_pixels > 8.0F || bible.camera.maximum_hit_stop_ms > 80 ||
        bible.camera.transition_ms > 250 || !bible.camera.motion_interruptible)
        diagnostics.push_back("presentation_bible_camera_unbounded");
    std::set<std::string> variant_ids;
    for (const auto& variant : bible.accessibility_variants) {
        if (variant.id.empty() || !variant_ids.insert(variant.id).second || variant.ui_scale < 0.8F ||
            variant.ui_scale > 2.0F || !variant.audio_optional || !variant.non_color_cues) {
            diagnostics.push_back("presentation_bible_accessibility_variant_invalid");
            break;
        }
        const auto tokens = ui::makeUrpgDesignTokens(variant.theme, variant.ui_scale, variant.reduced_motion);
        if (!ui::validateUrpgDesignTokens(tokens).empty()) diagnostics.push_back("presentation_bible_variant_tokens_invalid:" + variant.id);
    }
    for (const auto* required : {"default", "large_text", "high_contrast", "reduced_motion"}) {
        if (!variant_ids.contains(required)) diagnostics.push_back(std::string("presentation_bible_variant_missing:") + required);
    }
    return diagnostics;
}

nlohmann::json runtimePresentationBibleSnapshot(const RuntimePresentationBible& bible) {
    nlohmann::json feedback = nlohmann::json::array();
    for (const auto& tier : bible.feedback_hierarchy) {
        feedback.push_back({{"id", tier.id}, {"priority", tier.priority},
                            {"maximum_duration_ms", tier.maximum_duration_ms},
                            {"interrupts_lower_priority", tier.interrupts_lower_priority},
                            {"requires_non_color_cue", tier.requires_non_color_cue}});
    }
    nlohmann::json variants = nlohmann::json::array();
    for (const auto& variant : bible.accessibility_variants) {
        variants.push_back({{"id", variant.id}, {"ui_scale", variant.ui_scale},
                            {"reduced_motion", variant.reduced_motion}, {"audio_optional", variant.audio_optional},
                            {"non_color_cues", variant.non_color_cues},
                            {"tokens", ui::urpgDesignTokenGallerySnapshot(
                                ui::makeUrpgDesignTokens(variant.theme, variant.ui_scale, variant.reduced_motion))}});
    }
    return {{"schema", "urpg.runtime_presentation_bible.v1"}, {"id", bible.id}, {"revision", bible.revision},
            {"visual_motif", bible.visual_motif}, {"camera_motif", bible.camera_motif},
            {"default_tokens", ui::urpgDesignTokenGallerySnapshot(bible.default_tokens)},
            {"sound_motifs", bible.sound_motifs}, {"feedback_hierarchy", std::move(feedback)},
            {"particles", {{"maximum_concurrent_emitters", bible.particles.maximum_concurrent_emitters},
                            {"maximum_lifetime_ms", bible.particles.maximum_lifetime_ms},
                            {"reduced_motion_density", bible.particles.reduced_motion_density},
                            {"motifs", bible.particles.motifs}}},
            {"camera", {{"maximum_shake_pixels", bible.camera.maximum_shake_pixels},
                         {"maximum_hit_stop_ms", bible.camera.maximum_hit_stop_ms},
                         {"transition_ms", bible.camera.transition_ms},
                         {"motion_interruptible", bible.camera.motion_interruptible}}},
            {"accessibility_variants", std::move(variants)}};
}

nlohmann::json runtimePresentationComponentShowcase(const RuntimePresentationBible& bible,
                                                    const RuntimeAccessibilityVariant& variant) {
    const auto tokens = ui::makeUrpgDesignTokens(variant.theme, variant.ui_scale, variant.reduced_motion);
    nlohmann::json components = nlohmann::json::array({
        {{"id", "primary_button"}, {"states", {"idle", "focused", "pressed", "disabled"}}, {"feedback_tier", "primary"}},
        {{"id", "dialogue_choice"}, {"states", {"idle", "focused", "selected", "unavailable"}}, {"feedback_tier", "primary"}},
        {{"id", "interaction_prompt"}, {"states", {"available", "acknowledged", "completed"}}, {"feedback_tier", "secondary"}},
        {{"id", "quest_notification"}, {"states", {"started", "updated", "completed"}}, {"feedback_tier", "secondary"}},
        {{"id", "error_card"}, {"states", {"error", "recovery_available", "resolved"}}, {"feedback_tier", "critical"}},
        {{"id", "progress_indicator"}, {"states", {"active", "paused", "complete", "failed"}}, {"feedback_tier", "ambient"}},
        {{"id", "combat_value"}, {"states", {"damage", "heal", "status", "critical"}}, {"feedback_tier", "primary"}}
    });
    return {{"schema", "urpg.runtime_component_showcase.v1"}, {"bible_id", bible.id},
            {"bible_revision", bible.revision}, {"variant", variant.id},
            {"tokens", ui::urpgDesignTokenGallerySnapshot(tokens)}, {"components", std::move(components)},
            {"camera_shake_pixels", variant.reduced_motion ? 0.0F : bible.camera.maximum_shake_pixels},
            {"particle_density", variant.reduced_motion ? bible.particles.reduced_motion_density : 1.0F},
            {"sound_optional", variant.audio_optional}, {"non_color_cues", variant.non_color_cues}};
}

} // namespace urpg::presentation
