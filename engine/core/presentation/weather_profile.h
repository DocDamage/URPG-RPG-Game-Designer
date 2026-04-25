#pragma once

#include <string>

namespace urpg::presentation {

struct WeatherProfile {
    std::string id;
    std::string type = "clear";
    float intensity = 0.0f;
    int32_t lighting_tint = 0xffffff;
};

} // namespace urpg::presentation
