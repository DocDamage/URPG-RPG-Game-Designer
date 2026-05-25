#pragma once

#include <nlohmann/json.hpp>

namespace urpg::localization {

[[nodiscard]] nlohmann::json extractDialoguePreviewLocalizationBundle(const nlohmann::json& document);
[[nodiscard]] nlohmann::json writebackDialoguePreviewLocalizationBundle(const nlohmann::json& document,
                                                                        const nlohmann::json& bundle);

} // namespace urpg::localization
