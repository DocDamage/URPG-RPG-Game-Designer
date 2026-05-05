#pragma once

#include "engine/core/ui/theme_registry.h"

#include <set>
#include <string>
#include <vector>

namespace urpg::ui {

struct ThemeDiagnostic {
    std::string code;
    std::string message;
};

struct ThemeAssetCatalog {
    std::set<std::string> fonts;
    std::set<std::string> sounds;
    std::set<std::string> images;
};

class ThemeValidator {
public:
    explicit ThemeValidator(ThemeAssetCatalog catalog);
    [[nodiscard]] std::vector<ThemeDiagnostic> validate(const UiTheme& theme) const;

private:
    ThemeAssetCatalog catalog_;
};

} // namespace urpg::ui
