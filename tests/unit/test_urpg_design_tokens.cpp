#include "engine/core/ui/urpg_design_tokens.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using urpg::ui::UrpgThemeMode;
using urpg::ui::makeUrpgDesignTokens;
using urpg::ui::urpgDesignTokenGallerySnapshot;
using urpg::ui::validateUrpgDesignTokens;

TEST_CASE("URPG design token themes cover every semantic category", "[ui][design_tokens]") {
    for (const auto mode : {UrpgThemeMode::Light, UrpgThemeMode::Dark, UrpgThemeMode::HighContrast}) {
        const auto tokens = makeUrpgDesignTokens(mode);
        REQUIRE(validateUrpgDesignTokens(tokens).empty());
        const auto gallery = urpgDesignTokenGallerySnapshot(tokens);
        for (const auto* category : {"colors", "typography", "spacing", "corner_radius", "borders", "elevation",
                                     "icons", "motion_ms", "sounds", "focus", "density"}) {
            REQUIRE(gallery.contains(category));
            REQUIRE_FALSE(gallery[category].empty());
        }
    }
}

TEST_CASE("URPG design tokens scale geometry and honor reduced motion", "[ui][design_tokens]") {
    const auto base = makeUrpgDesignTokens(UrpgThemeMode::Dark, 1.0F, false);
    const auto scaled = makeUrpgDesignTokens(UrpgThemeMode::Dark, 2.0F, true);
    REQUIRE(scaled.typography.at("body") == base.typography.at("body") * 2.0F);
    REQUIRE(scaled.spacing.at("md") == base.spacing.at("md") * 2.0F);
    REQUIRE(scaled.density.at("minimum_hit_target") == base.density.at("minimum_hit_target") * 2.0F);
    REQUIRE(scaled.motion_ms.at("feedback") == 0);
    REQUIRE(scaled.motion_ms.at("transition") == 0);
    REQUIRE(validateUrpgDesignTokens(scaled).empty());
    REQUIRE(validateUrpgDesignTokens(makeUrpgDesignTokens(UrpgThemeMode::Light, 0.5F)).empty());
    REQUIRE(validateUrpgDesignTokens(makeUrpgDesignTokens(UrpgThemeMode::HighContrast, 3.0F)).empty());
}
