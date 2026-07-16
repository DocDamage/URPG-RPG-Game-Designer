#include "engine/core/presentation/runtime_presentation_bible.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("Runtime presentation bible defines bounded original language and hierarchy",
          "[presentation][bible][pcq600]") {
    using namespace urpg::presentation;
    const auto bible = makeRuntimePresentationBible();
    REQUIRE(validateRuntimePresentationBible(bible).empty());
    REQUIRE(bible.id.starts_with("urpg."));
    REQUIRE(bible.sound_motifs.size() == 6);
    REQUIRE(bible.feedback_hierarchy.size() == 4);
    REQUIRE(bible.feedback_hierarchy.front().id == "critical");
    REQUIRE(bible.feedback_hierarchy.back().id == "ambient");
    REQUIRE(bible.particles.maximum_concurrent_emitters <= 32);
    REQUIRE(bible.particles.maximum_lifetime_ms <= 1500);
    REQUIRE(bible.camera.maximum_shake_pixels <= 8.0F);
    REQUIRE(bible.camera.maximum_hit_stop_ms <= 80);
    REQUIRE(bible.camera.motion_interruptible);

    const auto snapshot = runtimePresentationBibleSnapshot(bible);
    REQUIRE(snapshot["schema"] == "urpg.runtime_presentation_bible.v1");
    REQUIRE(snapshot["default_tokens"]["colors"].size() == 9);
    REQUIRE(snapshot["accessibility_variants"].size() == 4);
}

TEST_CASE("Runtime component showcase covers primary interaction states and variants",
          "[presentation][bible][pcq600]") {
    using namespace urpg::presentation;
    const auto bible = makeRuntimePresentationBible();
    for (const auto& variant : bible.accessibility_variants) {
        const auto showcase = runtimePresentationComponentShowcase(bible, variant);
        REQUIRE(showcase["schema"] == "urpg.runtime_component_showcase.v1");
        REQUIRE(showcase["components"].size() == 7);
        REQUIRE(showcase["sound_optional"] == true);
        REQUIRE(showcase["non_color_cues"] == true);
        REQUIRE(showcase["tokens"]["id"].get<std::string>().starts_with("urpg."));
        if (variant.reduced_motion) {
            REQUIRE(showcase["camera_shake_pixels"] == 0.0F);
            REQUIRE(showcase["particle_density"] <= 0.25F);
            for (const auto& [name, duration] : showcase["tokens"]["motion_ms"].items()) {
                REQUIRE((name == "instant" || duration == 0));
            }
        }
    }
}

TEST_CASE("Runtime presentation bible validation rejects unbounded or inaccessible policy",
          "[presentation][bible][pcq600]") {
    using namespace urpg::presentation;
    auto bible = makeRuntimePresentationBible();
    bible.camera.maximum_shake_pixels = 20.0F;
    bible.particles.maximum_concurrent_emitters = 100;
    bible.feedback_hierarchy[1].priority = bible.feedback_hierarchy[0].priority;
    bible.accessibility_variants[0].audio_optional = false;
    bible.sound_motifs.erase("confirm");
    const auto diagnostics = validateRuntimePresentationBible(bible);
    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "presentation_bible_camera_unbounded") != diagnostics.end());
    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "presentation_bible_particles_unbounded") != diagnostics.end());
    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "presentation_bible_feedback_hierarchy_invalid") != diagnostics.end());
    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "presentation_bible_accessibility_variant_invalid") != diagnostics.end());
    REQUIRE(std::find(diagnostics.begin(), diagnostics.end(), "presentation_bible_sound_missing:confirm") != diagnostics.end());
}
