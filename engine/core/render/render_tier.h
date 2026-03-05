#pragma once

#include <cstdint>
#include <string_view>

namespace urpg {

enum class RenderTier : uint8_t {
    Basic = 0,
    Standard = 1,
    Advanced = 2
};

struct RenderFeatureDecl {
    std::string_view id;
    RenderTier min_tier = RenderTier::Basic;
};

constexpr bool SupportsTier(RenderTier available, RenderTier required) {
    return static_cast<uint8_t>(available) >= static_cast<uint8_t>(required);
}

constexpr bool IsFeatureSupported(RenderTier available, const RenderFeatureDecl& feature) {
    return SupportsTier(available, feature.min_tier);
}

} // namespace urpg
