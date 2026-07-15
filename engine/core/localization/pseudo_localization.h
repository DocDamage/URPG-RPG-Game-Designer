#pragma once

#include <string>
#include <string_view>

namespace urpg::localization {

// Produces a deterministic, non-persistent visual-expansion preview for
// localization layout review. It never changes a project locale bundle.
std::string pseudoLocalize(std::string_view source);

} // namespace urpg::localization
