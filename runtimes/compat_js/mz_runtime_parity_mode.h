#pragma once

#include <nlohmann/json.hpp>

namespace urpg::compat_js {

enum class MzRuntimeParityMode {
    BridgeOnly,
    RuntimeParityExperimental,
};

[[nodiscard]] const char* ToString(MzRuntimeParityMode mode);

struct MzRuntimeParityConfig {
    MzRuntimeParityMode mode = MzRuntimeParityMode::BridgeOnly;

    [[nodiscard]] bool runtimeParityEnabled() const;
    [[nodiscard]] nlohmann::json toJson() const;
};

}  // namespace urpg::compat_js
