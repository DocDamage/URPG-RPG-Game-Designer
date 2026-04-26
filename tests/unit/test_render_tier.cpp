#include "engine/core/render/render_tier.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Render tiers are ordered by capability", "[render]") {
    REQUIRE(urpg::SupportsTier(urpg::RenderTier::Standard, urpg::RenderTier::Basic));
    REQUIRE(urpg::SupportsTier(urpg::RenderTier::Advanced, urpg::RenderTier::Standard));
    REQUIRE_FALSE(urpg::SupportsTier(urpg::RenderTier::Basic, urpg::RenderTier::Advanced));
}

TEST_CASE("Feature declaration checks available tier", "[render]") {
    constexpr urpg::RenderFeatureDecl feature{"deferred_lighting", urpg::RenderTier::Advanced};

    REQUIRE_FALSE(urpg::IsFeatureSupported(urpg::RenderTier::Standard, feature));
    REQUIRE(urpg::IsFeatureSupported(urpg::RenderTier::Advanced, feature));
}
