#pragma once

#include <nlohmann/json_fwd.hpp>

#include <string>

namespace urpg {

std::string DumpCanonicalJson(const nlohmann::json& value);

} // namespace urpg
