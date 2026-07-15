#pragma once

#include <cstddef>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>

namespace urpg::security {

struct ScriptTransformResult {
    std::string transformId;
    std::string scriptId;
    std::string transformedSource;
    std::string sourceSha256;
    std::string transformedSha256;
    std::size_t sourceBytes = 0;
    std::size_t transformedBytes = 0;

    [[nodiscard]] nlohmann::json toJson() const;
};

[[nodiscard]] ScriptTransformResult TransformScriptForRelease(std::string_view source, std::string_view scriptId);

} // namespace urpg::security
