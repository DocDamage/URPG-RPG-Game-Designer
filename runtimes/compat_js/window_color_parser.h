#pragma once

#include "window_compat.h"
#include <optional>

namespace urpg {
namespace compat {

Color lerpColor(const Color& from, const Color& to, double t);
std::optional<Color> colorFromValue(const Value& value, const Window_Base* windowForSystemColor = nullptr);

} // namespace compat
} // namespace urpg
