#include "engine/core/format/canonical_json.h"

#include <nlohmann/json.hpp>

namespace urpg {

std::string DumpCanonicalJson(const nlohmann::json& value) {
    return value.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
}

} // namespace urpg
