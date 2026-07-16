#include "engine/core/accessibility/inclusive_runtime_policy.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/runtime_shell_flow.h"
#include "engine/core/platform/headless_renderer.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Inclusive runtime policy derives consistent visual audio and motion behavior",
          "[accessibility][runtime_policy][pcq651]") {
    auto settings = urpg::accessibility::InclusiveSettings::safeDefaults();
    settings.text_scale = urpg::accessibility::TextScaleProfile::ExtraLarge;
    settings.high_contrast = true;
    settings.color_filter = urpg::accessibility::ColorFilter::Deuteranopia;
    settings.reduced_motion = true;
    settings.screen_shake = 0.0F;
    settings.flash_intensity = 0.25F;
    settings.captions = false;
    settings.mono_audio = true;

    const auto policy = urpg::accessibility::inclusiveRuntimePolicy(settings);
    REQUIRE(policy.has_value());
    CHECK(policy->design_tokens.mode == urpg::ui::UrpgThemeMode::HighContrast);
    CHECK(policy->design_tokens.scale == Catch::Approx(2.0F));
    CHECK(policy->design_tokens.reduced_motion);
    CHECK_FALSE(policy->shared_feedback.particles_enabled);
    CHECK_FALSE(policy->shared_feedback.screen_shake_enabled);
    CHECK_FALSE(policy->shared_feedback.hit_stop_enabled);
    CHECK(policy->battle_feedback.color_filter == urpg::presentation::BattleColorFilter::Deuteranopia);
    const auto filteredRed = urpg::applyRendererColorMatrix(policy->renderer, {1.0F, 0.0F, 0.0F});
    CHECK(filteredRed[0] == Catch::Approx(0.625F));
    CHECK(filteredRed[1] == Catch::Approx(0.700F));
    CHECK(filteredRed[2] == Catch::Approx(0.000F));
    CHECK(policy->renderer.flashIntensity == Catch::Approx(0.25F));
    CHECK_FALSE(policy->exploration_feedback.captions_enabled);
    CHECK(policy->flash_intensity == Catch::Approx(0.25F));
    CHECK(policy->non_color_cues);
    CHECK(policy->mono_audio);
}

TEST_CASE("Inclusive runtime policy applies atomically to shell exploration and battle feedback",
          "[accessibility][runtime_policy][scene][pcq651]") {
    auto settings = urpg::accessibility::InclusiveSettings::safeDefaults();
    settings.effects_volume = 0.0F;
    settings.reduced_motion = true;
    settings.screen_shake = 0.0F;
    settings.color_filter = urpg::accessibility::ColorFilter::Tritanopia;
    settings.flash_intensity = 0.4F;

    urpg::scene::RuntimeShellFlow shell;
    urpg::HeadlessRenderer renderer;
    REQUIRE(urpg::accessibility::applyInclusiveRuntimePolicy(
        settings, &shell.feedbackStack(), nullptr, nullptr, &renderer));
    const auto shell_settings = shell.feedbackStack().snapshot().settings;
    CHECK_FALSE(shell_settings.audio_enabled);
    CHECK(shell_settings.reduced_motion);
    CHECK_FALSE(shell_settings.screen_shake_enabled);
    CHECK(renderer.accessibilitySettings().flashIntensity == Catch::Approx(0.4F));
    const auto filteredBlue = urpg::applyRendererColorMatrix(
        renderer.accessibilitySettings(), {0.0F, 0.0F, 1.0F});
    CHECK(filteredBlue[0] == Catch::Approx(0.0F));
    CHECK(filteredBlue[1] == Catch::Approx(0.567F));
    CHECK(filteredBlue[2] == Catch::Approx(0.525F));

    urpg::scene::MapScene map("InclusivePolicyMap", 2, 2);
    REQUIRE(map.setDialogueInclusiveSettings(settings));
    const auto exploration_settings = map.explorationFeedback().snapshot().settings;
    CHECK_FALSE(exploration_settings.audio_enabled);
    CHECK(exploration_settings.reduced_motion);
    CHECK_FALSE(exploration_settings.screen_shake_enabled);

    urpg::scene::BattleScene battle({});
    REQUIRE(battle.setInclusiveSettings(settings));
    const auto battle_settings = battle.battleFeedback().snapshot().settings;
    CHECK_FALSE(battle_settings.audio_enabled);
    CHECK(battle_settings.reduced_motion);
    CHECK(battle_settings.color_filter == urpg::presentation::BattleColorFilter::Tritanopia);
    CHECK_FALSE(battle_settings.screen_shake_enabled);
    CHECK(battle.inclusiveFlashIntensity() == Catch::Approx(0.4F));
    CHECK(battle.inclusiveNonColorCues());
}

TEST_CASE("Battle overlay emphasis renders a fading flash bounded by inclusive intensity",
          "[accessibility][runtime_policy][scene][battle][render][pcq651]") {
    auto settings = urpg::accessibility::InclusiveSettings::safeDefaults();
    settings.flash_intensity = 0.4F;

    urpg::scene::BattleScene battle({});
    REQUIRE(battle.setInclusiveSettings(settings));
    urpg::presentation::effects::EffectCue cue;
    cue.overlayEmphasis.value = 0.9F;
    battle.enqueueEffectCue(cue);
    CHECK(battle.activeFlashOpacity() == Catch::Approx(0.36F));

    urpg::SpriteBatcher batcher;
    batcher.begin();
    battle.draw(batcher);
    batcher.end();
    bool foundBoundedFlash = false;
    for (const auto& batch : batcher.getBatches()) {
        for (const auto& vertex : batch.vertices) {
            if (vertex.position[2] == Catch::Approx(0.99F) && vertex.color[3] == Catch::Approx(0.36F)) {
                foundBoundedFlash = true;
            }
        }
    }
    CHECK(foundBoundedFlash);

    battle.onUpdate(0.2F);
    CHECK(battle.activeFlashOpacity() == Catch::Approx(0.0F));

    settings.flash_intensity = 0.0F;
    REQUIRE(battle.setInclusiveSettings(settings));
    battle.enqueueEffectCue(cue);
    CHECK(battle.activeFlashOpacity() == Catch::Approx(0.0F));
}

TEST_CASE("Invalid inclusive settings leave runtime feedback consumers unchanged",
          "[accessibility][runtime_policy][pcq651]") {
    auto invalid = urpg::accessibility::InclusiveSettings::safeDefaults();
    invalid.flash_intensity = 1.5F;

    urpg::scene::RuntimeShellFlow shell;
    const auto before = shell.feedbackStack().snapshot().settings;
    REQUIRE_FALSE(shell.setInclusiveSettings(invalid));
    const auto after = shell.feedbackStack().snapshot().settings;
    CHECK(after.audio_enabled == before.audio_enabled);
    CHECK(after.reduced_motion == before.reduced_motion);
    CHECK(after.intensity_scale == Catch::Approx(before.intensity_scale));
}
