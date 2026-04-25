#include "engine/core/ui/theme_validator.h"

#include <utility>

namespace urpg::ui {

ThemeValidator::ThemeValidator(ThemeAssetCatalog catalog)
    : catalog_(std::move(catalog)) {}

std::vector<ThemeDiagnostic> ThemeValidator::validate(const UiTheme& theme) const {
    std::vector<ThemeDiagnostic> diagnostics;
    if (!catalog_.fonts.contains(theme.font)) {
        diagnostics.push_back({"missing_font", "theme references a font not present in the asset catalog"});
    }
    if (!catalog_.sounds.contains(theme.menuSound)) {
        diagnostics.push_back({"missing_sound", "theme references a menu sound not present in the asset catalog"});
    }
    return diagnostics;
}

} // namespace urpg::ui
