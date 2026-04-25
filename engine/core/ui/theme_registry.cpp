#include "engine/core/ui/theme_registry.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace urpg::ui {

void ThemeRegistry::addTheme(UiTheme theme) {
    themes_.push_back(std::move(theme));
}

UiTheme ThemeRegistry::theme(const std::string& id) const {
    const auto it = std::ranges::find_if(themes_, [&](const UiTheme& theme) { return theme.id == id; });
    return it == themes_.end() ? UiTheme{} : *it;
}

ThemePreviewSnapshot ThemeRegistry::previewScreens(const std::string& theme_id, std::vector<std::string> screens) const {
    std::ranges::sort(screens);
    std::ostringstream hash;
    hash << theme_id << ":";
    for (std::size_t i = 0; i < screens.size(); ++i) {
        if (i > 0) {
            hash << "|";
        }
        hash << screens[i];
    }
    return {theme_id, screens, hash.str()};
}

} // namespace urpg::ui
