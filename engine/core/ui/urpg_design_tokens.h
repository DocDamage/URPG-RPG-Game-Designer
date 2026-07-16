#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace urpg::ui {

enum class UrpgThemeMode { Light, Dark, HighContrast };

struct UrpgColorToken {
    float red = 0.0F;
    float green = 0.0F;
    float blue = 0.0F;
    float alpha = 1.0F;
};

struct UrpgDesignTokens {
    std::string id;
    UrpgThemeMode mode = UrpgThemeMode::Dark;
    float scale = 1.0F;
    bool reduced_motion = false;
    std::map<std::string, UrpgColorToken> colors;
    std::map<std::string, float> typography;
    std::map<std::string, float> spacing;
    std::map<std::string, float> corner_radius;
    std::map<std::string, float> borders;
    std::map<std::string, float> elevation;
    std::map<std::string, std::string> icons;
    std::map<std::string, uint32_t> motion_ms;
    std::map<std::string, std::string> sounds;
    std::map<std::string, float> focus;
    std::map<std::string, float> density;
};

UrpgDesignTokens makeUrpgDesignTokens(UrpgThemeMode mode, float scale = 1.0F, bool reduced_motion = false);
std::vector<std::string> validateUrpgDesignTokens(const UrpgDesignTokens& tokens);
nlohmann::json urpgDesignTokenGallerySnapshot(const UrpgDesignTokens& tokens);
double urpgColorContrastRatio(const UrpgColorToken& foreground, const UrpgColorToken& background);

} // namespace urpg::ui
